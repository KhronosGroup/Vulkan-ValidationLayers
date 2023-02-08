
// This file is ***GENERATED***.  Do Not Edit.
// See layer_chassis_generator.py for modifications.

/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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
#include "best_practices/best_practices_validation.h"
#include "core_checks/core_validation.h"
#include "gpu_validation/gpu_validation.h"
#include "object_lifetime_validation.h"
#include "gpu_validation/debug_printf.h"
#include "stateless/stateless_validation.h"
#include "sync/sync_validation.h"
#include "thread_safety.h"

// This header file must be included after the above validation object class definitions
#include "chassis_dispatch_helper.h"

// Global list of sType,size identifiers
std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};


namespace vulkan_layer_chassis {

static const VkLayerProperties global_layer = {
    OBJECT_LAYER_NAME, VK_LAYER_API_VERSION, 1, "LunarG validation Layer",
};

static const VkExtensionProperties instance_extensions[] = {{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
                                                            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION},
                                                            {VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME, VK_EXT_VALIDATION_FEATURES_SPEC_VERSION}};
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

extern const vvl::unordered_map<std::string, function_data> name_to_funcptr_map;

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
    if (!context->fine_grained_locking) {
        context->LogPerformanceWarning(context->instance, kVUID_Core_CreateInstance_Locking_Warning,
                                       "Fine-grained locking is disabled, this will adversely affect performance of multithreaded applications.");
    }
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
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(nullptr, "vkCreateInstance");
    if (fpCreateInstance == nullptr) return VK_ERROR_INITIALIZATION_FAILED;
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
    uint32_t specified_version = (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
    uint32_t api_version = VK_MAKE_API_VERSION(0, VK_API_VERSION_MAJOR(specified_version), VK_API_VERSION_MINOR(specified_version), 0);

    auto report_data = new debug_report_data{};
    report_data->instance_pnext_chain = SafePnextCopy(pCreateInfo->pNext);
    ActivateInstanceDebugCallbacks(report_data);

    // Set up enable and disable features flags
    CHECK_ENABLED local_enables {};
    CHECK_DISABLED local_disables {};
    bool lock_setting;
    ConfigAndEnvSettings config_and_env_settings_data {OBJECT_LAYER_DESCRIPTION, pCreateInfo->pNext, local_enables, local_disables,
        report_data->filter_message_ids, &report_data->duplicate_message_limit, &lock_setting};
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

    auto core_checks_obj = new CoreChecks;
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
    for (const ValidationObject* intercept : local_object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : local_object_dispatch) {
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
    framework->fine_grained_locking = lock_setting;

    framework->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &framework->instance_dispatch_table, fpGetInstanceProcAddr);
    framework->report_data = report_data;
    framework->api_version = api_version;
    framework->instance_extensions.InitFromInstanceCreateInfo(specified_version, pCreateInfo);

    // We need to call this to properly check which device extensions have been promoted when validating query functions
    // that take as input a physical device, which can be called before a logical device has been created.
    framework->device_extensions.InitFromDeviceCreateInfo(&framework->instance_extensions, specified_version);

    OutputLayerStatusInfo(framework);

    thread_checker_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    object_tracker_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    parameter_validation_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    core_checks_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    best_practices_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    gpu_assisted_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    debug_printf_obj->FinalizeInstanceValidationObject(framework, *pInstance);
    sync_validation_obj->FinalizeInstanceValidationObject(framework, *pInstance);

    for (ValidationObject* intercept : framework->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        intercept->PreCallValidateDestroyInstance(instance, pAllocator);
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyInstance(instance, pAllocator);
    }

    layer_data->instance_dispatch_table.DestroyInstance(instance, pAllocator);

    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyInstance(instance, pAllocator);
    }

    DeactivateInstanceDebugCallbacks(layer_data->report_data);
    FreePnextChain(layer_data->report_data->instance_pnext_chain);

    LayerDebugUtilsDestroyInstance(layer_data->report_data);

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
    if (fpCreateDevice == nullptr) {
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
    for (const ValidationObject* intercept : instance_interceptor->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDevice(gpu, pCreateInfo, pAllocator, pDevice);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : instance_interceptor->object_dispatch) {
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

    auto thread_safety_obj = new ThreadSafety(reinterpret_cast<ThreadSafety *>(instance_interceptor->GetValidationObject(instance_interceptor->object_dispatch, LayerObjectTypeThreading)));
    thread_safety_obj->InitDeviceValidationObject(!disables[thread_safety], instance_interceptor, device_interceptor);

    auto stateless_validation_obj = new StatelessValidation;
    stateless_validation_obj->InitDeviceValidationObject(!disables[stateless_checks], instance_interceptor, device_interceptor);

    auto object_tracker_obj = new ObjectLifetimes;
    object_tracker_obj->InitDeviceValidationObject(!disables[object_tracking], instance_interceptor, device_interceptor);

    auto core_checks_obj = new CoreChecks;
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

    for (ValidationObject* intercept : instance_interceptor->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, result);
    }

    device_interceptor->InitObjectDispatchVectors();

    DeviceExtensionWhitelist(device_interceptor, pCreateInfo, *pDevice);
    DeviceExtensionWarnlist(device_interceptor, pCreateInfo, *pDevice);

    return result;
}

// NOTE: Do _not_ skip the dispatch call when destroying a device. Whether or not there was a validation error,
//       the loader will destroy the device, and know nothing about future references to this device making it
//       impossible for the caller to use this device handle further. IOW, this is our _only_ chance to (potentially)
//       dispatch the driver's DestroyDevice function.
VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(device);
    auto layer_data = GetLayerDataPtr(key, layer_data_map);
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        intercept->PreCallValidateDestroyDevice(device, pAllocator);
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDevice(device, pAllocator);
    }

    layer_data->device_dispatch_table.DestroyDevice(device, pAllocator);

    for (ValidationObject* intercept : layer_data->object_dispatch) {
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        cgpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(cgpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(cgpl_state[intercept->container_type]));
    }

    auto usepCreateInfos = (!cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
    if (cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

    VkResult result = DispatchCreateGraphicsPipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

    for (ValidationObject* intercept : layer_data->object_dispatch) {
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        ccpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(ccpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, &(ccpl_state[intercept->container_type]));
    }

    auto usepCreateInfos = (!ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
    if (ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

    VkResult result = DispatchCreateComputePipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        crtpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                      pAllocator, pPipelines, &(crtpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                            pPipelines, &(crtpl_state[intercept->container_type]));
    }

    VkResult result = DispatchCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        crtpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos,
                                                                       pAllocator, pPipelines, &(crtpl_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                            pPipelines, &(crtpl_state[intercept->container_type]));
    }

    VkResult result = DispatchCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, &cpl_state);
    }
    VkResult result = DispatchCreatePipelineLayout(device, &cpl_state.modified_create_info, pAllocator, pPipelineLayout);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, &csm_state);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, &csm_state);
    }
    VkResult result = DispatchCreateShaderModule(device, &csm_state.instrumented_create_info, pAllocator, pShaderModule);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        ads_state[intercept->container_type].Init(pAllocateInfo->descriptorSetCount);
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, &(ads_state[intercept->container_type]));
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    }
    VkResult result = DispatchAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, &cb_state);
    }
    VkResult result = DispatchCreateBuffer(device, &cb_state.modified_create_info, pAllocator, pBuffer);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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

    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);
    }

    VkResult result = DispatchGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties);

    if (original_pToolProperties != nullptr) {
        pToolProperties = original_pToolProperties;
    }
    (*pToolCount)++;

    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
static const std::set<std::string> kDeviceWarnExtensionNames {
};

static void DeviceExtensionWarnlist(ValidationObject *layer_data, const VkDeviceCreateInfo *pCreateInfo, VkDevice device) {
    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        // Check for recognized device extensions
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceWarnExtensionNames)) {
            layer_data->LogWarning(layer_data->device, kVUIDUndefined,
                    "Device Extension %s validation support is incomplete, incorrect results are possible.",
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    }
    VkResult result = DispatchEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceProperties(physicalDevice, pProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceProperties(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties(physicalDevice, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties*           pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceQueue]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceQueue]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    }
    DispatchGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceQueue]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueSubmit]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueSubmit(queue, submitCount, pSubmits, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSubmit]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);
    }
    VkResult result = DispatchQueueSubmit(queue, submitCount, pSubmits, fence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSubmit]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueSubmit(queue, submitCount, pSubmits, fence, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueWaitIdle(
    VkQueue                                     queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueWaitIdle]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueWaitIdle(queue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueWaitIdle]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueWaitIdle(queue);
    }
    VkResult result = DispatchQueueWaitIdle(queue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueWaitIdle]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueWaitIdle(queue, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeviceWaitIdle(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDeviceWaitIdle]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDeviceWaitIdle(device);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDeviceWaitIdle]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDeviceWaitIdle(device);
    }
    VkResult result = DispatchDeviceWaitIdle(device);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDeviceWaitIdle]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAllocateMemory]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAllocateMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    }
    VkResult result = DispatchAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAllocateMemory]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFreeMemory]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateFreeMemory(device, memory, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordFreeMemory(device, memory, pAllocator);
    }
    DispatchFreeMemory(device, memory, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeMemory]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateMapMemory]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateMapMemory(device, memory, offset, size, flags, ppData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordMapMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordMapMemory(device, memory, offset, size, flags, ppData);
    }
    VkResult result = DispatchMapMemory(device, memory, offset, size, flags, ppData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordMapMemory]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUnmapMemory]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateUnmapMemory(device, memory);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUnmapMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUnmapMemory(device, memory);
    }
    DispatchUnmapMemory(device, memory);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUnmapMemory]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFlushMappedMemoryRanges]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFlushMappedMemoryRanges]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }
    VkResult result = DispatchFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFlushMappedMemoryRanges]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateInvalidateMappedMemoryRanges]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordInvalidateMappedMemoryRanges]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    }
    VkResult result = DispatchInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordInvalidateMappedMemoryRanges]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceMemoryCommitment]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryCommitment]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    }
    DispatchGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryCommitment]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindBufferMemory]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindBufferMemory(device, buffer, memory, memoryOffset);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindBufferMemory(device, buffer, memory, memoryOffset);
    }
    VkResult result = DispatchBindBufferMemory(device, buffer, memory, memoryOffset);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindImageMemory]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindImageMemory(device, image, memory, memoryOffset);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindImageMemory(device, image, memory, memoryOffset);
    }
    VkResult result = DispatchBindImageMemory(device, image, memory, memoryOffset);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageMemoryRequirements(device, image, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageMemoryRequirements(device, image, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements(device, image, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSparseMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSparseMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSparseMemoryRequirements]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    }
    DispatchGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueBindSparse]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueBindSparse]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    }
    VkResult result = DispatchQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueBindSparse]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateFence]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateFence(device, pCreateInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateFence]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateFence(device, pCreateInfo, pAllocator, pFence);
    }
    VkResult result = DispatchCreateFence(device, pCreateInfo, pAllocator, pFence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateFence]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyFence]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyFence(device, fence, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyFence]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyFence(device, fence, pAllocator);
    }
    DispatchDestroyFence(device, fence, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyFence]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetFences]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateResetFences(device, fenceCount, pFences);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetFences]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetFences(device, fenceCount, pFences);
    }
    VkResult result = DispatchResetFences(device, fenceCount, pFences);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetFences]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceStatus]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetFenceStatus(device, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceStatus]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFenceStatus(device, fence);
    }
    VkResult result = DispatchGetFenceStatus(device, fence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceStatus]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWaitForFences]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateWaitForFences(device, fenceCount, pFences, waitAll, timeout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitForFences]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    }
    VkResult result = DispatchWaitForFences(device, fenceCount, pFences, waitAll, timeout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitForFences]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSemaphore]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    }
    VkResult result = DispatchCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSemaphore]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySemaphore]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroySemaphore(device, semaphore, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySemaphore(device, semaphore, pAllocator);
    }
    DispatchDestroySemaphore(device, semaphore, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySemaphore]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateEvent]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateEvent(device, pCreateInfo, pAllocator, pEvent);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    }
    VkResult result = DispatchCreateEvent(device, pCreateInfo, pAllocator, pEvent);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateEvent]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyEvent]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyEvent(device, event, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyEvent(device, event, pAllocator);
    }
    DispatchDestroyEvent(device, event, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyEvent(device, event, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetEventStatus(
    VkDevice                                    device,
    VkEvent                                     event) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetEventStatus]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetEventStatus(device, event);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetEventStatus]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetEventStatus(device, event);
    }
    VkResult result = DispatchGetEventStatus(device, event);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetEventStatus]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetEvent(device, event);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetEvent(device, event);
    }
    VkResult result = DispatchSetEvent(device, event);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetEvent]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateResetEvent(device, event);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetEvent(device, event);
    }
    VkResult result = DispatchResetEvent(device, event);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetEvent]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateQueryPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    }
    VkResult result = DispatchCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateQueryPool]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyQueryPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyQueryPool(device, queryPool, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyQueryPool(device, queryPool, pAllocator);
    }
    DispatchDestroyQueryPool(device, queryPool, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyQueryPool]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetQueryPoolResults]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueryPoolResults]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    }
    VkResult result = DispatchGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueryPoolResults]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyBuffer(device, buffer, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyBuffer(device, buffer, pAllocator);
    }
    DispatchDestroyBuffer(device, buffer, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateBufferView]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateBufferView(device, pCreateInfo, pAllocator, pView);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateBufferView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateBufferView(device, pCreateInfo, pAllocator, pView);
    }
    VkResult result = DispatchCreateBufferView(device, pCreateInfo, pAllocator, pView);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateBufferView]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyBufferView]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyBufferView(device, bufferView, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyBufferView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyBufferView(device, bufferView, pAllocator);
    }
    DispatchDestroyBufferView(device, bufferView, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyBufferView]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateImage(device, pCreateInfo, pAllocator, pImage);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateImage(device, pCreateInfo, pAllocator, pImage);
    }
    VkResult result = DispatchCreateImage(device, pCreateInfo, pAllocator, pImage);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyImage(device, image, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyImage(device, image, pAllocator);
    }
    DispatchDestroyImage(device, image, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSubresourceLayout]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageSubresourceLayout(device, image, pSubresource, pLayout);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSubresourceLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    }
    DispatchGetImageSubresourceLayout(device, image, pSubresource, pLayout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSubresourceLayout]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateImageView]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateImageView(device, pCreateInfo, pAllocator, pView);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateImageView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateImageView(device, pCreateInfo, pAllocator, pView);
    }
    VkResult result = DispatchCreateImageView(device, pCreateInfo, pAllocator, pView);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateImageView]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyImageView]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyImageView(device, imageView, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyImageView]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyImageView(device, imageView, pAllocator);
    }
    DispatchDestroyImageView(device, imageView, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyImageView]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyImageView(device, imageView, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyShaderModule(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyShaderModule]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyShaderModule(device, shaderModule, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyShaderModule]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyShaderModule(device, shaderModule, pAllocator);
    }
    DispatchDestroyShaderModule(device, shaderModule, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyShaderModule]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreatePipelineCache]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreatePipelineCache]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    }
    VkResult result = DispatchCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreatePipelineCache]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPipelineCache]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyPipelineCache(device, pipelineCache, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipelineCache]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPipelineCache(device, pipelineCache, pAllocator);
    }
    DispatchDestroyPipelineCache(device, pipelineCache, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipelineCache]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineCacheData]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineCacheData]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    }
    VkResult result = DispatchGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineCacheData]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateMergePipelineCaches]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordMergePipelineCaches]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    }
    VkResult result = DispatchMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordMergePipelineCaches]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPipeline]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyPipeline(device, pipeline, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipeline]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPipeline(device, pipeline, pAllocator);
    }
    DispatchDestroyPipeline(device, pipeline, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipeline]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPipelineLayout]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyPipelineLayout(device, pipelineLayout, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPipelineLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    }
    DispatchDestroyPipelineLayout(device, pipelineLayout, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPipelineLayout]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSampler]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateSampler(device, pCreateInfo, pAllocator, pSampler);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSampler]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    }
    VkResult result = DispatchCreateSampler(device, pCreateInfo, pAllocator, pSampler);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSampler]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySampler]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroySampler(device, sampler, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySampler]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySampler(device, sampler, pAllocator);
    }
    DispatchDestroySampler(device, sampler, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySampler]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorSetLayout]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorSetLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    }
    VkResult result = DispatchCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorSetLayout]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorSetLayout]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorSetLayout]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    }
    DispatchDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorSetLayout]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    }
    VkResult result = DispatchCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorPool]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyDescriptorPool(device, descriptorPool, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
    DispatchDestroyDescriptorPool(device, descriptorPool, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyDescriptorPool(device, descriptorPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(
    VkDevice                                    device,
    VkDescriptorPool                            descriptorPool,
    VkDescriptorPoolResetFlags                  flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetDescriptorPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateResetDescriptorPool(device, descriptorPool, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetDescriptorPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetDescriptorPool(device, descriptorPool, flags);
    }
    VkResult result = DispatchResetDescriptorPool(device, descriptorPool, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetDescriptorPool]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFreeDescriptorSets]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    }
    VkResult result = DispatchFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeDescriptorSets]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUpdateDescriptorSets]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }
    DispatchUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateDescriptorSets]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateFramebuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateFramebuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    }
    VkResult result = DispatchCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateFramebuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyFramebuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyFramebuffer(device, framebuffer, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyFramebuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyFramebuffer(device, framebuffer, pAllocator);
    }
    DispatchDestroyFramebuffer(device, framebuffer, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyFramebuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyRenderPass(device, renderPass, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyRenderPass(device, renderPass, pAllocator);
    }
    DispatchDestroyRenderPass(device, renderPass, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyRenderPass]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRenderAreaGranularity]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetRenderAreaGranularity(device, renderPass, pGranularity);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRenderAreaGranularity]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRenderAreaGranularity(device, renderPass, pGranularity);
    }
    DispatchGetRenderAreaGranularity(device, renderPass, pGranularity);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRenderAreaGranularity]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateCommandPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    }
    VkResult result = DispatchCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateCommandPool]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyCommandPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyCommandPool(device, commandPool, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    }
    DispatchDestroyCommandPool(device, commandPool, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyCommandPool(device, commandPool, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL ResetCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolResetFlags                     flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetCommandPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateResetCommandPool(device, commandPool, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetCommandPool(device, commandPool, flags);
    }
    VkResult result = DispatchResetCommandPool(device, commandPool, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetCommandPool]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAllocateCommandBuffers]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAllocateCommandBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    }
    VkResult result = DispatchAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAllocateCommandBuffers]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateFreeCommandBuffers]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordFreeCommandBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
    DispatchFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordFreeCommandBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(
    VkCommandBuffer                             commandBuffer,
    const VkCommandBufferBeginInfo*             pBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBeginCommandBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBeginCommandBuffer(commandBuffer, pBeginInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBeginCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo);
    }
    VkResult result = DispatchBeginCommandBuffer(commandBuffer, pBeginInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBeginCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBeginCommandBuffer(commandBuffer, pBeginInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EndCommandBuffer(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateEndCommandBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateEndCommandBuffer(commandBuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordEndCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEndCommandBuffer(commandBuffer);
    }
    VkResult result = DispatchEndCommandBuffer(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordEndCommandBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetCommandBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateResetCommandBuffer(commandBuffer, flags);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetCommandBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetCommandBuffer(commandBuffer, flags);
    }
    VkResult result = DispatchResetCommandBuffer(commandBuffer, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetCommandBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindPipeline]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindPipeline]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    }
    DispatchCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindPipeline]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewport]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewport]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    }
    DispatchCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewport]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetScissor]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetScissor]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
    DispatchCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetScissor]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineWidth(
    VkCommandBuffer                             commandBuffer,
    float                                       lineWidth) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLineWidth]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetLineWidth(commandBuffer, lineWidth);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineWidth]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLineWidth(commandBuffer, lineWidth);
    }
    DispatchCmdSetLineWidth(commandBuffer, lineWidth);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineWidth]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBias]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBias]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
    DispatchCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBias]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetBlendConstants(
    VkCommandBuffer                             commandBuffer,
    const float                                 blendConstants[4]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetBlendConstants]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetBlendConstants(commandBuffer, blendConstants);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetBlendConstants]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetBlendConstants(commandBuffer, blendConstants);
    }
    DispatchCmdSetBlendConstants(commandBuffer, blendConstants);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetBlendConstants]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBounds]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBounds]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    }
    DispatchCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBounds]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilCompareMask]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilCompareMask]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    }
    DispatchCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilCompareMask]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilWriteMask]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilWriteMask]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    }
    DispatchCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilWriteMask]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilReference]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetStencilReference(commandBuffer, faceMask, reference);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilReference]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilReference(commandBuffer, faceMask, reference);
    }
    DispatchCmdSetStencilReference(commandBuffer, faceMask, reference);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilReference]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindDescriptorSets]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindDescriptorSets]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }
    DispatchCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindDescriptorSets]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindIndexBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindIndexBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    }
    DispatchCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindIndexBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindVertexBuffers]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindVertexBuffers]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    }
    DispatchCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindVertexBuffers]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDraw]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDraw]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
    DispatchCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDraw]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexed]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexed]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
    DispatchCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexed]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirect]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirect]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirect]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirect]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatch]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatch]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatch]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatchIndirect]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDispatchIndirect(commandBuffer, buffer, offset);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchIndirect]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDispatchIndirect(commandBuffer, buffer, offset);
    }
    DispatchCmdDispatchIndirect(commandBuffer, buffer, offset);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchIndirect]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    }
    DispatchCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBlitImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBlitImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }
    DispatchCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBlitImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBufferToImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBufferToImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBufferToImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImageToBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImageToBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }
    DispatchCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImageToBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdUpdateBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdUpdateBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    }
    DispatchCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdUpdateBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdFillBuffer]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdFillBuffer]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    }
    DispatchCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdFillBuffer]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdClearColorImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearColorImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    }
    DispatchCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearColorImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdClearDepthStencilImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearDepthStencilImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }
    DispatchCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearDepthStencilImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdClearAttachments]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdClearAttachments]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    }
    DispatchCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdClearAttachments]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResolveImage]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResolveImage]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }
    DispatchCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResolveImage]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetEvent(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetEvent(commandBuffer, event, stageMask);
    }
    DispatchCmdSetEvent(commandBuffer, event, stageMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetEvent]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResetEvent]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdResetEvent(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetEvent]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResetEvent(commandBuffer, event, stageMask);
    }
    DispatchCmdResetEvent(commandBuffer, event, stageMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetEvent]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWaitEvents]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWaitEvents]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
    DispatchCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWaitEvents]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPipelineBarrier]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPipelineBarrier]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }
    DispatchCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPipelineBarrier]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginQuery]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginQuery(commandBuffer, queryPool, query, flags);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginQuery]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginQuery(commandBuffer, queryPool, query, flags);
    }
    DispatchCmdBeginQuery(commandBuffer, queryPool, query, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginQuery]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndQuery]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndQuery(commandBuffer, queryPool, query);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndQuery]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndQuery(commandBuffer, queryPool, query);
    }
    DispatchCmdEndQuery(commandBuffer, queryPool, query);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndQuery]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResetQueryPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    }
    DispatchCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetQueryPool]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteTimestamp]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteTimestamp]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    }
    DispatchCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteTimestamp]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyQueryPoolResults]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyQueryPoolResults]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    }
    DispatchCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyQueryPoolResults]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPushConstants]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPushConstants]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    }
    DispatchCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPushConstants]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
    DispatchCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass(
    VkCommandBuffer                             commandBuffer,
    VkSubpassContents                           contents) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdNextSubpass]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdNextSubpass(commandBuffer, contents);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdNextSubpass(commandBuffer, contents);
    }
    DispatchCmdNextSubpass(commandBuffer, contents);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdNextSubpass(commandBuffer, contents);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndRenderPass(commandBuffer);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndRenderPass(commandBuffer);
    }
    DispatchCmdEndRenderPass(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdExecuteCommands]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdExecuteCommands]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    }
    DispatchCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdExecuteCommands]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindBufferMemory2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindBufferMemory2(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindBufferMemory2(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindBufferMemory2(device, bindInfoCount, pBindInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory2]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindImageMemory2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindImageMemory2(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindImageMemory2(device, bindInfoCount, pBindInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory2]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
    DispatchGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPeerMemoryFeatures]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMask(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDeviceMask]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDeviceMask]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDeviceMask(commandBuffer, deviceMask);
    }
    DispatchCmdSetDeviceMask(commandBuffer, deviceMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDeviceMask]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatchBase]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchBase]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchBase]) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageMemoryRequirements2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements2]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferMemoryRequirements2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSparseMemoryRequirements2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSparseMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSparseMemoryRequirements2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceProperties2(physicalDevice, pProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties2(physicalDevice, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
    DispatchGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL TrimCommandPool(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateTrimCommandPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateTrimCommandPool(device, commandPool, flags);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordTrimCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordTrimCommandPool(device, commandPool, flags);
    }
    DispatchTrimCommandPool(device, commandPool, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordTrimCommandPool]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordTrimCommandPool(device, commandPool, flags);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(
    VkDevice                                    device,
    const VkDeviceQueueInfo2*                   pQueueInfo,
    VkQueue*                                    pQueue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceQueue2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceQueue2(device, pQueueInfo, pQueue);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceQueue2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceQueue2(device, pQueueInfo, pQueue);
    }
    DispatchGetDeviceQueue2(device, pQueueInfo, pQueue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceQueue2]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSamplerYcbcrConversion]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSamplerYcbcrConversion]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    }
    VkResult result = DispatchCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSamplerYcbcrConversion]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySamplerYcbcrConversion]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySamplerYcbcrConversion]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    }
    DispatchDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySamplerYcbcrConversion]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorUpdateTemplate]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorUpdateTemplate]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    }
    VkResult result = DispatchCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorUpdateTemplate]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorUpdateTemplate]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorUpdateTemplate]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    }
    DispatchDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorUpdateTemplate]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUpdateDescriptorSetWithTemplate]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateDescriptorSetWithTemplate]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
    DispatchUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateDescriptorSetWithTemplate]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceExternalBufferProperties(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfo*   pExternalBufferInfo,
    VkExternalBufferProperties*                 pExternalBufferProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
    DispatchGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
    DispatchGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
    DispatchGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetLayoutSupport]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutSupport]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    }
    DispatchGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutSupport]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectCount]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectCount]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectCount]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirectCount]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirectCount]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirectCount]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateRenderPass2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass2]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRenderPass2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
    DispatchCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass2]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdNextSubpass2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
    DispatchCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    }
    DispatchCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass2]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetQueryPool]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateResetQueryPool(device, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetQueryPool]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetQueryPool(device, queryPool, firstQuery, queryCount);
    }
    DispatchResetQueryPool(device, queryPool, firstQuery, queryCount);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetQueryPool]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreCounterValue]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSemaphoreCounterValue(device, semaphore, pValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreCounterValue]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreCounterValue(device, semaphore, pValue);
    }
    VkResult result = DispatchGetSemaphoreCounterValue(device, semaphore, pValue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreCounterValue]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWaitSemaphores]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateWaitSemaphores(device, pWaitInfo, timeout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitSemaphores]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWaitSemaphores(device, pWaitInfo, timeout);
    }
    VkResult result = DispatchWaitSemaphores(device, pWaitInfo, timeout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitSemaphores]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSignalSemaphore]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSignalSemaphore(device, pSignalInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSignalSemaphore]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSignalSemaphore(device, pSignalInfo);
    }
    VkResult result = DispatchSignalSemaphore(device, pSignalInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSignalSemaphore]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferDeviceAddress]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferDeviceAddress(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferDeviceAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferDeviceAddress(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetBufferDeviceAddress(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferDeviceAddress]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferOpaqueCaptureAddress]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferOpaqueCaptureAddress(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferOpaqueCaptureAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferOpaqueCaptureAddress(device, pInfo);
    }
    uint64_t result = DispatchGetBufferOpaqueCaptureAddress(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferOpaqueCaptureAddress]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    }
    uint64_t result = DispatchGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryOpaqueCaptureAddress]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolProperties(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pToolCount,
    VkPhysicalDeviceToolProperties*             pToolProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceToolProperties(physicalDevice, pToolCount, pToolProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlot(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreatePrivateDataSlot]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreatePrivateDataSlot]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    }
    VkResult result = DispatchCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreatePrivateDataSlot]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlot(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPrivateDataSlot]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPrivateDataSlot]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
    }
    DispatchDestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPrivateDataSlot]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyPrivateDataSlot(device, privateDataSlot, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL SetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetPrivateData]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetPrivateData]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
    }
    VkResult result = DispatchSetPrivateData(device, objectType, objectHandle, privateDataSlot, data);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetPrivateData]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetPrivateData(device, objectType, objectHandle, privateDataSlot, data, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPrivateData(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPrivateData]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPrivateData]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
    }
    DispatchGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPrivateData]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPrivateData(device, objectType, objectHandle, privateDataSlot, pData);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetEvent2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetEvent2(commandBuffer, event, pDependencyInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetEvent2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo);
    }
    DispatchCmdSetEvent2(commandBuffer, event, pDependencyInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetEvent2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetEvent2(commandBuffer, event, pDependencyInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent2(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResetEvent2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdResetEvent2(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetEvent2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResetEvent2(commandBuffer, event, stageMask);
    }
    DispatchCmdResetEvent2(commandBuffer, event, stageMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetEvent2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResetEvent2(commandBuffer, event, stageMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWaitEvents2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWaitEvents2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
    DispatchCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWaitEvents2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWaitEvents2(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPipelineBarrier2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPipelineBarrier2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
    }
    DispatchCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPipelineBarrier2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteTimestamp2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteTimestamp2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
    }
    DispatchCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteTimestamp2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteTimestamp2(commandBuffer, stage, queryPool, query);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueSubmit2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSubmit2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence);
    }
    VkResult result = DispatchQueueSubmit2(queue, submitCount, pSubmits, fence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSubmit2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueSubmit2(queue, submitCount, pSubmits, fence, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBuffer2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBuffer2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
    }
    DispatchCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBuffer2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImage2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyImage2(commandBuffer, pCopyImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo);
    }
    DispatchCmdCopyImage2(commandBuffer, pCopyImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBufferToImage2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBufferToImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
    }
    DispatchCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBufferToImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImageToBuffer2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImageToBuffer2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
    }
    DispatchCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImageToBuffer2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage2(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBlitImage2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBlitImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo);
    }
    DispatchCmdBlitImage2(commandBuffer, pBlitImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBlitImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage2(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResolveImage2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResolveImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo);
    }
    DispatchCmdResolveImage2(commandBuffer, pResolveImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResolveImage2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResolveImage2(commandBuffer, pResolveImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginRendering(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRendering]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginRendering(commandBuffer, pRenderingInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRendering]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo);
    }
    DispatchCmdBeginRendering(commandBuffer, pRenderingInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRendering]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginRendering(commandBuffer, pRenderingInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRendering(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRendering]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndRendering(commandBuffer);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRendering]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndRendering(commandBuffer);
    }
    DispatchCmdEndRendering(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRendering]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndRendering(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCullMode(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCullMode]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCullMode(commandBuffer, cullMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCullMode]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCullMode(commandBuffer, cullMode);
    }
    DispatchCmdSetCullMode(commandBuffer, cullMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCullMode]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCullMode(commandBuffer, cullMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetFrontFace(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFrontFace]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetFrontFace(commandBuffer, frontFace);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFrontFace]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetFrontFace(commandBuffer, frontFace);
    }
    DispatchCmdSetFrontFace(commandBuffer, frontFace);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFrontFace]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetFrontFace(commandBuffer, frontFace);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopology(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPrimitiveTopology]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPrimitiveTopology]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
    }
    DispatchCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPrimitiveTopology]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    viewportCount,
    const VkViewport*                           pViewports) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportWithCount]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportWithCount]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
    }
    DispatchCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportWithCount]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetViewportWithCount(commandBuffer, viewportCount, pViewports);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetScissorWithCount(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    scissorCount,
    const VkRect2D*                             pScissors) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetScissorWithCount]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetScissorWithCount]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
    }
    DispatchCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetScissorWithCount]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetScissorWithCount(commandBuffer, scissorCount, pScissors);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindVertexBuffers2(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets,
    const VkDeviceSize*                         pSizes,
    const VkDeviceSize*                         pStrides) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindVertexBuffers2]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindVertexBuffers2]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
    DispatchCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindVertexBuffers2]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthTestEnable]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthTestEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
    }
    DispatchCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthTestEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthTestEnable(commandBuffer, depthTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthWriteEnable]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthWriteEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
    }
    DispatchCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthWriteEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthWriteEnable(commandBuffer, depthWriteEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOp(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthCompareOp]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthCompareOp]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
    }
    DispatchCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthCompareOp]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthCompareOp(commandBuffer, depthCompareOp);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBoundsTestEnable]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBoundsTestEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
    }
    DispatchCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBoundsTestEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBoundsTestEnable(commandBuffer, depthBoundsTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilTestEnable]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilTestEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
    }
    DispatchCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilTestEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilTestEnable(commandBuffer, stencilTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilOp(
    VkCommandBuffer                             commandBuffer,
    VkStencilFaceFlags                          faceMask,
    VkStencilOp                                 failOp,
    VkStencilOp                                 passOp,
    VkStencilOp                                 depthFailOp,
    VkCompareOp                                 compareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilOp]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilOp]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
    DispatchCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilOp]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilOp(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRasterizerDiscardEnable]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRasterizerDiscardEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
    }
    DispatchCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRasterizerDiscardEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetRasterizerDiscardEnable(commandBuffer, rasterizerDiscardEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBiasEnable]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBiasEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
    }
    DispatchCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBiasEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBiasEnable(commandBuffer, depthBiasEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnable(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPrimitiveRestartEnable]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPrimitiveRestartEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
    }
    DispatchCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPrimitiveRestartEnable]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPrimitiveRestartEnable(commandBuffer, primitiveRestartEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceBufferMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceBufferMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
    }
    DispatchGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceBufferMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceBufferMemoryRequirements(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceImageMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceImageMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
    }
    DispatchGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceImageMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceImageMemoryRequirements(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceImageSparseMemoryRequirements(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceImageSparseMemoryRequirements]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceImageSparseMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceImageSparseMemoryRequirements]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceImageSparseMemoryRequirements(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
}


VKAPI_ATTR void VKAPI_CALL DestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroySurfaceKHR(instance, surface, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySurfaceKHR(instance, surface, pAllocator);
    }
    DispatchDestroySurfaceKHR(instance, surface, pAllocator);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSwapchainKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSwapchainKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    }
    VkResult result = DispatchCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSwapchainKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySwapchainKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroySwapchainKHR(device, swapchain, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySwapchainKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySwapchainKHR(device, swapchain, pAllocator);
    }
    DispatchDestroySwapchainKHR(device, swapchain, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySwapchainKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainImagesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainImagesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }
    VkResult result = DispatchGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainImagesKHR]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireNextImageKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireNextImageKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }
    VkResult result = DispatchAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireNextImageKHR]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueuePresentKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueuePresentKHR(queue, pPresentInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueuePresentKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueuePresentKHR(queue, pPresentInfo);
    }
    VkResult result = DispatchQueuePresentKHR(queue, pPresentInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueuePresentKHR]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupPresentCapabilitiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPresentCapabilitiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    }
    VkResult result = DispatchGetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPresentCapabilitiesKHR]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupSurfacePresentModesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupSurfacePresentModesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    }
    VkResult result = DispatchGetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupSurfacePresentModesKHR]) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    }
    VkResult result = DispatchGetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireNextImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireNextImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    }
    VkResult result = DispatchAcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireNextImage2KHR]) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    }
    VkResult result = DispatchGetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    }
    VkResult result = DispatchCreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    }
    VkResult result = DispatchGetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSharedSwapchainsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSharedSwapchainsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    }
    VkResult result = DispatchCreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSharedSwapchainsKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
        if (skip) return VK_FALSE;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    }
    VkBool32 result = DispatchGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
        if (skip) return VK_FALSE;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    }
    VkBool32 result = DispatchGetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
        if (skip) return VK_FALSE;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    }
    VkBool32 result = DispatchGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
        if (skip) return VK_FALSE;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    }
    VkBool32 result = DispatchGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
    }
    return result;
}
#endif // VK_USE_PLATFORM_WIN32_KHR



VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkVideoProfileInfoKHR*                pVideoProfile,
    VkVideoCapabilitiesKHR*                     pCapabilities) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, pVideoProfile, pCapabilities, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceVideoFormatPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceVideoFormatInfoKHR*   pVideoFormatInfo,
    uint32_t*                                   pVideoFormatPropertyCount,
    VkVideoFormatPropertiesKHR*                 pVideoFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceVideoFormatPropertiesKHR(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceVideoFormatPropertiesKHR(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceVideoFormatPropertiesKHR(physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionKHR(
    VkDevice                                    device,
    const VkVideoSessionCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionKHR*                          pVideoSession) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateVideoSessionKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateVideoSessionKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
    }
    VkResult result = DispatchCreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateVideoSessionKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateVideoSessionKHR(device, pCreateInfo, pAllocator, pVideoSession, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyVideoSessionKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyVideoSessionKHR(device, videoSession, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyVideoSessionKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyVideoSessionKHR(device, videoSession, pAllocator);
    }
    DispatchDestroyVideoSessionKHR(device, videoSession, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyVideoSessionKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyVideoSessionKHR(device, videoSession, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetVideoSessionMemoryRequirementsKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t*                                   pMemoryRequirementsCount,
    VkVideoSessionMemoryRequirementsKHR*        pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetVideoSessionMemoryRequirementsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetVideoSessionMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements);
    }
    VkResult result = DispatchGetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetVideoSessionMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetVideoSessionMemoryRequirementsKHR(device, videoSession, pMemoryRequirementsCount, pMemoryRequirements, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BindVideoSessionMemoryKHR(
    VkDevice                                    device,
    VkVideoSessionKHR                           videoSession,
    uint32_t                                    bindSessionMemoryInfoCount,
    const VkBindVideoSessionMemoryInfoKHR*      pBindSessionMemoryInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindVideoSessionMemoryKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindVideoSessionMemoryKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos);
    }
    VkResult result = DispatchBindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindVideoSessionMemoryKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBindVideoSessionMemoryKHR(device, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateVideoSessionParametersKHR(
    VkDevice                                    device,
    const VkVideoSessionParametersCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkVideoSessionParametersKHR*                pVideoSessionParameters) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateVideoSessionParametersKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateVideoSessionParametersKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters);
    }
    VkResult result = DispatchCreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateVideoSessionParametersKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateVideoSessionParametersKHR(device, pCreateInfo, pAllocator, pVideoSessionParameters, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL UpdateVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkVideoSessionParametersUpdateInfoKHR* pUpdateInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUpdateVideoSessionParametersKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateUpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateVideoSessionParametersKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
    }
    VkResult result = DispatchUpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateVideoSessionParametersKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordUpdateVideoSessionParametersKHR(device, videoSessionParameters, pUpdateInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyVideoSessionParametersKHR(
    VkDevice                                    device,
    VkVideoSessionParametersKHR                 videoSessionParameters,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyVideoSessionParametersKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyVideoSessionParametersKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
    }
    DispatchDestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyVideoSessionParametersKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyVideoSessionParametersKHR(device, videoSessionParameters, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoBeginCodingInfoKHR*            pBeginInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginVideoCodingKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginVideoCodingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
    }
    DispatchCmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginVideoCodingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginVideoCodingKHR(commandBuffer, pBeginInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEndCodingInfoKHR*              pEndCodingInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndVideoCodingKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndVideoCodingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
    }
    DispatchCmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndVideoCodingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndVideoCodingKHR(commandBuffer, pEndCodingInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdControlVideoCodingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoCodingControlInfoKHR*          pCodingControlInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdControlVideoCodingKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdControlVideoCodingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
    }
    DispatchCmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdControlVideoCodingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdControlVideoCodingKHR(commandBuffer, pCodingControlInfo);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdDecodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoDecodeInfoKHR*                 pDecodeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDecodeVideoKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDecodeVideoKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
    }
    DispatchCmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDecodeVideoKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDecodeVideoKHR(commandBuffer, pDecodeInfo);
    }
}



VKAPI_ATTR void VKAPI_CALL CmdBeginRenderingKHR(
    VkCommandBuffer                             commandBuffer,
    const VkRenderingInfo*                      pRenderingInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRenderingKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
    }
    DispatchCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginRenderingKHR(commandBuffer, pRenderingInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderingKHR(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderingKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndRenderingKHR(commandBuffer);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndRenderingKHR(commandBuffer);
    }
    DispatchCmdEndRenderingKHR(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderingKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndRenderingKHR(commandBuffer);
    }
}



VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2*                  pFeatures) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    }
    DispatchGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2*                pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    }
    DispatchGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceProperties2KHR(physicalDevice, pProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2*                        pFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    }
    DispatchGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceFormatProperties2KHR(physicalDevice, format, pFormatProperties);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2*     pImageFormatInfo,
    VkImageFormatProperties2*                   pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceImageFormatProperties2KHR(physicalDevice, pImageFormatInfo, pImageFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
    DispatchGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2*          pMemoryProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    }
    DispatchGetPhysicalDeviceMemoryProperties2KHR(physicalDevice, pMemoryProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    }
    DispatchGetPhysicalDeviceSparseImageFormatProperties2KHR(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
    DispatchGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceGroupPeerMemoryFeaturesKHR(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDeviceMaskKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDeviceMaskKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDeviceMaskKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    }
    DispatchCmdSetDeviceMaskKHR(commandBuffer, deviceMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDeviceMaskKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDispatchBaseKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDispatchBaseKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDispatchBaseKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDispatchBaseKHR(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }
}



VKAPI_ATTR void VKAPI_CALL TrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlags                      flags) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateTrimCommandPoolKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateTrimCommandPoolKHR(device, commandPool, flags);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordTrimCommandPoolKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    }
    DispatchTrimCommandPoolKHR(device, commandPool, flags);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordTrimCommandPoolKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordTrimCommandPoolKHR(device, commandPool, flags);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDeviceGroupsKHR(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceGroupsKHR(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    }
    DispatchGetPhysicalDeviceExternalBufferPropertiesKHR(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryWin32HandleKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    }
    VkResult result = DispatchGetMemoryWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryWin32HandlePropertiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryWin32HandlePropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    }
    VkResult result = DispatchGetMemoryWin32HandlePropertiesKHR(device, handleType, handle, pMemoryWin32HandleProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryWin32HandlePropertiesKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetMemoryFdKHR(device, pGetFdInfo, pFd);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryFdKHR]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    }
    VkResult result = DispatchGetMemoryFdPropertiesKHR(device, handleType, fd, pMemoryFdProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryFdPropertiesKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
    DispatchGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceExternalSemaphorePropertiesKHR(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportSemaphoreWin32HandleKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportSemaphoreWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    }
    VkResult result = DispatchImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportSemaphoreWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreWin32HandleKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    }
    VkResult result = DispatchGetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportSemaphoreFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportSemaphoreFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    }
    VkResult result = DispatchImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportSemaphoreFdKHR]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetSemaphoreFdKHR(device, pGetFdInfo, pFd);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreFdKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPushDescriptorSetKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPushDescriptorSetKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    }
    DispatchCmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPushDescriptorSetKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPushDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPushDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    }
    DispatchCmdPushDescriptorSetWithTemplateKHR(commandBuffer, descriptorUpdateTemplate, layout, set, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPushDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    }
    VkResult result = DispatchCreateDescriptorUpdateTemplateKHR(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    }
    DispatchDestroyDescriptorUpdateTemplateKHR(device, descriptorUpdateTemplate, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDescriptorUpdateTemplateKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUpdateDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUpdateDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    }
    DispatchUpdateDescriptorSetWithTemplateKHR(device, descriptorSet, descriptorUpdateTemplate, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUpdateDescriptorSetWithTemplateKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateRenderPass2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateRenderPass2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    }
    VkResult result = DispatchCreateRenderPass2KHR(device, pCreateInfo, pAllocator, pRenderPass);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateRenderPass2KHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginRenderPass2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginRenderPass2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
    DispatchCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginRenderPass2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginRenderPass2KHR(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdNextSubpass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassBeginInfo*                   pSubpassBeginInfo,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdNextSubpass2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdNextSubpass2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
    DispatchCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdNextSubpass2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdNextSubpass2KHR(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndRenderPass2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkSubpassEndInfo*                     pSubpassEndInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndRenderPass2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndRenderPass2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    }
    DispatchCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndRenderPass2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndRenderPass2KHR(commandBuffer, pSubpassEndInfo);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainStatusKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSwapchainStatusKHR(device, swapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainStatusKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSwapchainStatusKHR(device, swapchain);
    }
    VkResult result = DispatchGetSwapchainStatusKHR(device, swapchain);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainStatusKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
    DispatchGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceExternalFencePropertiesKHR(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
    }
}


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL ImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportFenceWin32HandleKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportFenceWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    }
    VkResult result = DispatchImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportFenceWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceWin32HandleKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    }
    VkResult result = DispatchGetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceWin32HandleKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportFenceFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateImportFenceFdKHR(device, pImportFenceFdInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportFenceFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportFenceFdKHR(device, pImportFenceFdInfo);
    }
    VkResult result = DispatchImportFenceFdKHR(device, pImportFenceFdInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportFenceFdKHR]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFenceFdKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetFenceFdKHR(device, pGetFdInfo, pFd);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFenceFdKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFenceFdKHR(device, pGetFdInfo, pFd);
    }
    VkResult result = DispatchGetFenceFdKHR(device, pGetFdInfo, pFd);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFenceFdKHR]) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    }
    VkResult result = DispatchEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    }
    DispatchGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireProfilingLockKHR(
    VkDevice                                    device,
    const VkAcquireProfilingLockInfoKHR*        pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireProfilingLockKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquireProfilingLockKHR(device, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireProfilingLockKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireProfilingLockKHR(device, pInfo);
    }
    VkResult result = DispatchAcquireProfilingLockKHR(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireProfilingLockKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAcquireProfilingLockKHR(device, pInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL ReleaseProfilingLockKHR(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleaseProfilingLockKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateReleaseProfilingLockKHR(device);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleaseProfilingLockKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordReleaseProfilingLockKHR(device);
    }
    DispatchReleaseProfilingLockKHR(device);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleaseProfilingLockKHR]) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceDisplayPlaneProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    }
    VkResult result = DispatchGetDisplayPlaneCapabilities2KHR(physicalDevice, pDisplayPlaneInfo, pCapabilities);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageMemoryRequirements2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageMemoryRequirements2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
    DispatchGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageMemoryRequirements2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferMemoryRequirements2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferMemoryRequirements2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    }
    DispatchGetBufferMemoryRequirements2KHR(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferMemoryRequirements2KHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSparseMemoryRequirements2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSparseMemoryRequirements2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetImageSparseMemoryRequirements2KHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSparseMemoryRequirements2KHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateSamplerYcbcrConversionKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateSamplerYcbcrConversionKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    }
    VkResult result = DispatchCreateSamplerYcbcrConversionKHR(device, pCreateInfo, pAllocator, pYcbcrConversion);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateSamplerYcbcrConversionKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroySamplerYcbcrConversionKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroySamplerYcbcrConversionKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    }
    DispatchDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroySamplerYcbcrConversionKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroySamplerYcbcrConversionKHR(device, ycbcrConversion, pAllocator);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL BindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfo*               pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindBufferMemory2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindBufferMemory2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindBufferMemory2KHR(device, bindInfoCount, pBindInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindBufferMemory2KHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindImageMemory2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindImageMemory2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindImageMemory2KHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetLayoutSupportKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutSupportKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    }
    DispatchGetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutSupportKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectCountKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectCountKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectCountKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirectCountKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirectCountKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirectCountKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndexedIndirectCountKHR(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}












VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreCounterValueKHR(
    VkDevice                                    device,
    VkSemaphore                                 semaphore,
    uint64_t*                                   pValue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreCounterValueKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSemaphoreCounterValueKHR(device, semaphore, pValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreCounterValueKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    }
    VkResult result = DispatchGetSemaphoreCounterValueKHR(device, semaphore, pValue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreCounterValueKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWaitSemaphoresKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateWaitSemaphoresKHR(device, pWaitInfo, timeout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitSemaphoresKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout);
    }
    VkResult result = DispatchWaitSemaphoresKHR(device, pWaitInfo, timeout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitSemaphoresKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordWaitSemaphoresKHR(device, pWaitInfo, timeout, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SignalSemaphoreKHR(
    VkDevice                                    device,
    const VkSemaphoreSignalInfo*                pSignalInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSignalSemaphoreKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSignalSemaphoreKHR(device, pSignalInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSignalSemaphoreKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSignalSemaphoreKHR(device, pSignalInfo);
    }
    VkResult result = DispatchSignalSemaphoreKHR(device, pSignalInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSignalSemaphoreKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    }
    VkResult result = DispatchGetPhysicalDeviceFragmentShadingRatesKHR(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    }
    DispatchCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFragmentShadingRateKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    }
}





VKAPI_ATTR VkResult VKAPI_CALL WaitForPresentKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    presentId,
    uint64_t                                    timeout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWaitForPresentKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateWaitForPresentKHR(device, swapchain, presentId, timeout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWaitForPresentKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWaitForPresentKHR(device, swapchain, presentId, timeout);
    }
    VkResult result = DispatchWaitForPresentKHR(device, swapchain, presentId, timeout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWaitForPresentKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordWaitForPresentKHR(device, swapchain, presentId, timeout, result);
    }
    return result;
}



VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetBufferDeviceAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferDeviceAddressKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferDeviceAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferDeviceAddressKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferDeviceAddressKHR(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetBufferDeviceAddressKHR(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferDeviceAddressKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferDeviceAddressKHR(device, pInfo, result);
    }
    return result;
}

VKAPI_ATTR uint64_t VKAPI_CALL GetBufferOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkBufferDeviceAddressInfo*            pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferOpaqueCaptureAddressKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferOpaqueCaptureAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferOpaqueCaptureAddressKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    }
    uint64_t result = DispatchGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferOpaqueCaptureAddressKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferOpaqueCaptureAddressKHR(device, pInfo);
    }
    return result;
}

VKAPI_ATTR uint64_t VKAPI_CALL GetDeviceMemoryOpaqueCaptureAddressKHR(
    VkDevice                                    device,
    const VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    }
    uint64_t result = DispatchGetDeviceMemoryOpaqueCaptureAddressKHR(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMemoryOpaqueCaptureAddressKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateDeferredOperationKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateDeferredOperationKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    }
    VkResult result = DispatchCreateDeferredOperationKHR(device, pAllocator, pDeferredOperation);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateDeferredOperationKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyDeferredOperationKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyDeferredOperationKHR(device, operation, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyDeferredOperationKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    }
    DispatchDestroyDeferredOperationKHR(device, operation, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyDeferredOperationKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyDeferredOperationKHR(device, operation, pAllocator);
    }
}

VKAPI_ATTR uint32_t VKAPI_CALL GetDeferredOperationMaxConcurrencyKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeferredOperationMaxConcurrencyKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeferredOperationMaxConcurrencyKHR(device, operation);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeferredOperationMaxConcurrencyKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    }
    uint32_t result = DispatchGetDeferredOperationMaxConcurrencyKHR(device, operation);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeferredOperationMaxConcurrencyKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeferredOperationMaxConcurrencyKHR(device, operation);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDeferredOperationResultKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeferredOperationResultKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeferredOperationResultKHR(device, operation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeferredOperationResultKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeferredOperationResultKHR(device, operation);
    }
    VkResult result = DispatchGetDeferredOperationResultKHR(device, operation);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeferredOperationResultKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeferredOperationResultKHR(device, operation, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DeferredOperationJoinKHR(
    VkDevice                                    device,
    VkDeferredOperationKHR                      operation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDeferredOperationJoinKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDeferredOperationJoinKHR(device, operation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDeferredOperationJoinKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDeferredOperationJoinKHR(device, operation);
    }
    VkResult result = DispatchDeferredOperationJoinKHR(device, operation);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDeferredOperationJoinKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineExecutablePropertiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineExecutablePropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    }
    VkResult result = DispatchGetPipelineExecutablePropertiesKHR(device, pPipelineInfo, pExecutableCount, pProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineExecutablePropertiesKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineExecutableStatisticsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineExecutableStatisticsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    }
    VkResult result = DispatchGetPipelineExecutableStatisticsKHR(device, pExecutableInfo, pStatisticCount, pStatistics);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineExecutableStatisticsKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelineExecutableInternalRepresentationsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelineExecutableInternalRepresentationsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    }
    VkResult result = DispatchGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelineExecutableInternalRepresentationsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPipelineExecutableInternalRepresentationsKHR(device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations, result);
    }
    return result;
}





#ifdef VK_ENABLE_BETA_EXTENSIONS

VKAPI_ATTR void VKAPI_CALL CmdEncodeVideoKHR(
    VkCommandBuffer                             commandBuffer,
    const VkVideoEncodeInfoKHR*                 pEncodeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEncodeVideoKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEncodeVideoKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
    }
    DispatchCmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEncodeVideoKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEncodeVideoKHR(commandBuffer, pEncodeInfo);
    }
}
#endif // VK_ENABLE_BETA_EXTENSIONS


VKAPI_ATTR void VKAPI_CALL CmdSetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    const VkDependencyInfo*                     pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetEvent2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    }
    DispatchCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetEvent2KHR(commandBuffer, event, pDependencyInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResetEvent2KHR(
    VkCommandBuffer                             commandBuffer,
    VkEvent                                     event,
    VkPipelineStageFlags2                       stageMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResetEvent2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdResetEvent2KHR(commandBuffer, event, stageMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
    }
    DispatchCmdResetEvent2KHR(commandBuffer, event, stageMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResetEvent2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResetEvent2KHR(commandBuffer, event, stageMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWaitEvents2KHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    eventCount,
    const VkEvent*                              pEvents,
    const VkDependencyInfo*                     pDependencyInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWaitEvents2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWaitEvents2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
    DispatchCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWaitEvents2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWaitEvents2KHR(commandBuffer, eventCount, pEvents, pDependencyInfos);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPipelineBarrier2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkDependencyInfo*                     pDependencyInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPipelineBarrier2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPipelineBarrier2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    }
    DispatchCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPipelineBarrier2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdPipelineBarrier2KHR(commandBuffer, pDependencyInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWriteTimestamp2KHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkQueryPool                                 queryPool,
    uint32_t                                    query) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteTimestamp2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteTimestamp2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    }
    DispatchCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteTimestamp2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteTimestamp2KHR(commandBuffer, stage, queryPool, query);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSubmit2KHR(
    VkQueue                                     queue,
    uint32_t                                    submitCount,
    const VkSubmitInfo2*                        pSubmits,
    VkFence                                     fence) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueSubmit2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSubmit2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    }
    VkResult result = DispatchQueueSubmit2KHR(queue, submitCount, pSubmits, fence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSubmit2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueSubmit2KHR(queue, submitCount, pSubmits, fence, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdWriteBufferMarker2AMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlags2                       stage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteBufferMarker2AMD]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteBufferMarker2AMD]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    }
    DispatchCmdWriteBufferMarker2AMD(commandBuffer, stage, dstBuffer, dstOffset, marker);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteBufferMarker2AMD]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetQueueCheckpointData2NV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueueCheckpointData2NV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    }
    DispatchGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueueCheckpointData2NV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetQueueCheckpointData2NV(queue, pCheckpointDataCount, pCheckpointData);
    }
}






VKAPI_ATTR void VKAPI_CALL CmdCopyBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferInfo2*                    pCopyBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBuffer2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    }
    DispatchCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBuffer2KHR(commandBuffer, pCopyBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageInfo2*                     pCopyImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    }
    DispatchCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImage2KHR(commandBuffer, pCopyImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyBufferToImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyBufferToImageInfo2*             pCopyBufferToImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    }
    DispatchCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyBufferToImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyBufferToImage2KHR(commandBuffer, pCopyBufferToImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyImageToBuffer2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyImageToBufferInfo2*             pCopyImageToBufferInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    }
    DispatchCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyImageToBuffer2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyImageToBuffer2KHR(commandBuffer, pCopyImageToBufferInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBlitImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkBlitImageInfo2*                     pBlitImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBlitImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBlitImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    }
    DispatchCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBlitImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBlitImage2KHR(commandBuffer, pBlitImageInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdResolveImage2KHR(
    VkCommandBuffer                             commandBuffer,
    const VkResolveImageInfo2*                  pResolveImageInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdResolveImage2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdResolveImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    }
    DispatchCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdResolveImage2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdResolveImage2KHR(commandBuffer, pResolveImageInfo);
    }
}



VKAPI_ATTR void VKAPI_CALL CmdTraceRaysIndirect2KHR(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectDeviceAddress) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdTraceRaysIndirect2KHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdTraceRaysIndirect2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
    }
    DispatchCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdTraceRaysIndirect2KHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdTraceRaysIndirect2KHR(commandBuffer, indirectDeviceAddress);
    }
}



VKAPI_ATTR void VKAPI_CALL GetDeviceBufferMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceBufferMemoryRequirements*     pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceBufferMemoryRequirementsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceBufferMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    }
    DispatchGetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceBufferMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceBufferMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceImageMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceImageMemoryRequirementsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceImageMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    }
    DispatchGetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceImageMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceImageMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceImageSparseMemoryRequirementsKHR(
    VkDevice                                    device,
    const VkDeviceImageMemoryRequirements*      pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2*           pSparseMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceImageSparseMemoryRequirementsKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceImageSparseMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
    DispatchGetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceImageSparseMemoryRequirementsKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceImageSparseMemoryRequirementsKHR(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    }
    VkResult result = DispatchCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    LayerCreateReportCallback(layer_data->report_data, false, pCreateInfo, pAllocator, pCallback);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    }
    DispatchDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
    LayerDestroyCallback(layer_data->report_data, callback, pAllocator);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    }
    DispatchDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
    }
}








VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDebugMarkerSetObjectTagEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDebugMarkerSetObjectTagEXT(device, pTagInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDebugMarkerSetObjectTagEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo);
    }
    VkResult result = DispatchDebugMarkerSetObjectTagEXT(device, pTagInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDebugMarkerSetObjectTagEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDebugMarkerSetObjectTagEXT(device, pTagInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDebugMarkerSetObjectNameEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDebugMarkerSetObjectNameEXT(device, pNameInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDebugMarkerSetObjectNameEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo);
    }
    layer_data->report_data->DebugReportSetMarkerObjectName(pNameInfo);
    VkResult result = DispatchDebugMarkerSetObjectNameEXT(device, pNameInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDebugMarkerSetObjectNameEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDebugMarkerSetObjectNameEXT(device, pNameInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDebugMarkerBeginEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDebugMarkerBeginEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    }
    DispatchCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDebugMarkerBeginEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDebugMarkerEndEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDebugMarkerEndEXT(commandBuffer);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDebugMarkerEndEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    }
    DispatchCmdDebugMarkerEndEXT(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDebugMarkerEndEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDebugMarkerEndEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDebugMarkerInsertEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDebugMarkerInsertEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    }
    DispatchCmdDebugMarkerInsertEXT(commandBuffer, pMarkerInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDebugMarkerInsertEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindTransformFeedbackBuffersEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindTransformFeedbackBuffersEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    }
    DispatchCmdBindTransformFeedbackBuffersEXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindTransformFeedbackBuffersEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginTransformFeedbackEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginTransformFeedbackEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }
    DispatchCmdBeginTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginTransformFeedbackEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndTransformFeedbackEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndTransformFeedbackEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }
    DispatchCmdEndTransformFeedbackEXT(commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndTransformFeedbackEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginQueryIndexedEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginQueryIndexedEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    }
    DispatchCmdBeginQueryIndexedEXT(commandBuffer, queryPool, query, flags, index);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginQueryIndexedEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndQueryIndexedEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndQueryIndexedEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    }
    DispatchCmdEndQueryIndexedEXT(commandBuffer, queryPool, query, index);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndQueryIndexedEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectByteCountEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectByteCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    }
    DispatchCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectByteCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndirectByteCountEXT(commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL CreateCuModuleNVX(
    VkDevice                                    device,
    const VkCuModuleCreateInfoNVX*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuModuleNVX*                              pModule) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateCuModuleNVX]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateCuModuleNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
    }
    VkResult result = DispatchCreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateCuModuleNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateCuModuleNVX(device, pCreateInfo, pAllocator, pModule, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateCuFunctionNVX(
    VkDevice                                    device,
    const VkCuFunctionCreateInfoNVX*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkCuFunctionNVX*                            pFunction) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateCuFunctionNVX]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateCuFunctionNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction);
    }
    VkResult result = DispatchCreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateCuFunctionNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateCuFunctionNVX(device, pCreateInfo, pAllocator, pFunction, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyCuModuleNVX(
    VkDevice                                    device,
    VkCuModuleNVX                               module,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyCuModuleNVX]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyCuModuleNVX(device, module, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyCuModuleNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyCuModuleNVX(device, module, pAllocator);
    }
    DispatchDestroyCuModuleNVX(device, module, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyCuModuleNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyCuModuleNVX(device, module, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyCuFunctionNVX(
    VkDevice                                    device,
    VkCuFunctionNVX                             function,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyCuFunctionNVX]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyCuFunctionNVX(device, function, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyCuFunctionNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyCuFunctionNVX(device, function, pAllocator);
    }
    DispatchDestroyCuFunctionNVX(device, function, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyCuFunctionNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyCuFunctionNVX(device, function, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCuLaunchKernelNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCuLaunchInfoNVX*                    pLaunchInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCuLaunchKernelNVX]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCuLaunchKernelNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
    }
    DispatchCmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCuLaunchKernelNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCuLaunchKernelNVX(commandBuffer, pLaunchInfo);
    }
}


VKAPI_ATTR uint32_t VKAPI_CALL GetImageViewHandleNVX(
    VkDevice                                    device,
    const VkImageViewHandleInfoNVX*             pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageViewHandleNVX]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageViewHandleNVX(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageViewHandleNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageViewHandleNVX(device, pInfo);
    }
    uint32_t result = DispatchGetImageViewHandleNVX(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageViewHandleNVX]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageViewAddressNVX]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageViewAddressNVX(device, imageView, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageViewAddressNVX]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageViewAddressNVX(device, imageView, pProperties);
    }
    VkResult result = DispatchGetImageViewAddressNVX(device, imageView, pProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageViewAddressNVX]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndirectCountAMD]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndirectCountAMD]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndirectCountAMD]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawIndexedIndirectCountAMD]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawIndexedIndirectCountAMD]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawIndexedIndirectCountAMD]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawIndexedIndirectCountAMD(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}




#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif // VK_ENABLE_BETA_EXTENSIONS

#ifdef VK_ENABLE_BETA_EXTENSIONS
#endif // VK_ENABLE_BETA_EXTENSIONS



VKAPI_ATTR VkResult VKAPI_CALL GetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetShaderInfoAMD]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetShaderInfoAMD]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    }
    VkResult result = DispatchGetShaderInfoAMD(device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetShaderInfoAMD]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceExternalImageFormatPropertiesNV(physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryWin32HandleNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryWin32HandleNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    }
    VkResult result = DispatchGetMemoryWin32HandleNV(device, memory, handleType, pHandle);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryWin32HandleNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateViSurfaceNN(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginConditionalRenderingEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginConditionalRenderingEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    }
    DispatchCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginConditionalRenderingEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginConditionalRenderingEXT(commandBuffer, pConditionalRenderingBegin);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndConditionalRenderingEXT(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndConditionalRenderingEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndConditionalRenderingEXT(commandBuffer);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndConditionalRenderingEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndConditionalRenderingEXT(commandBuffer);
    }
    DispatchCmdEndConditionalRenderingEXT(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndConditionalRenderingEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportWScalingNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportWScalingNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    }
    DispatchCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportWScalingNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetViewportWScalingNV(commandBuffer, firstViewport, viewportCount, pViewportWScalings);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL ReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateReleaseDisplayEXT(physicalDevice, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordReleaseDisplayEXT(physicalDevice, display);
    }
    VkResult result = DispatchReleaseDisplayEXT(physicalDevice, display);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquireXlibDisplayEXT(physicalDevice, dpy, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    }
    VkResult result = DispatchAcquireXlibDisplayEXT(physicalDevice, dpy, display);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    }
    VkResult result = DispatchGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfaceCapabilities2EXT(physicalDevice, surface, pSurfaceCapabilities);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDisplayPowerControlEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDisplayPowerControlEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    }
    VkResult result = DispatchDisplayPowerControlEXT(device, display, pDisplayPowerInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDisplayPowerControlEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateRegisterDeviceEventEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordRegisterDeviceEventEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    }
    VkResult result = DispatchRegisterDeviceEventEXT(device, pDeviceEventInfo, pAllocator, pFence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordRegisterDeviceEventEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateRegisterDisplayEventEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordRegisterDisplayEventEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    }
    VkResult result = DispatchRegisterDisplayEventEXT(device, display, pDisplayEventInfo, pAllocator, pFence);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordRegisterDisplayEventEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSwapchainCounterEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSwapchainCounterEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    }
    VkResult result = DispatchGetSwapchainCounterEXT(device, swapchain, counter, pCounterValue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSwapchainCounterEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRefreshCycleDurationGOOGLE]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRefreshCycleDurationGOOGLE]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    }
    VkResult result = DispatchGetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRefreshCycleDurationGOOGLE]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPastPresentationTimingGOOGLE]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPastPresentationTimingGOOGLE]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    }
    VkResult result = DispatchGetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPastPresentationTimingGOOGLE]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDiscardRectangleEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDiscardRectangleEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    }
    DispatchCmdSetDiscardRectangleEXT(commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDiscardRectangleEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetHdrMetadataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetHdrMetadataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    }
    DispatchSetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetHdrMetadataEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetDebugUtilsObjectNameEXT(device, pNameInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetDebugUtilsObjectNameEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetDebugUtilsObjectNameEXT(device, pNameInfo);
    }
    layer_data->report_data->DebugReportSetUtilsObjectName(pNameInfo);
    VkResult result = DispatchSetDebugUtilsObjectNameEXT(device, pNameInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetDebugUtilsObjectNameEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetDebugUtilsObjectTagEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetDebugUtilsObjectTagEXT(device, pTagInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetDebugUtilsObjectTagEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetDebugUtilsObjectTagEXT(device, pTagInfo);
    }
    VkResult result = DispatchSetDebugUtilsObjectTagEXT(device, pTagInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetDebugUtilsObjectTagEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
    BeginQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);
    DispatchQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(
    VkQueue                                     queue) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueEndDebugUtilsLabelEXT(queue);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueEndDebugUtilsLabelEXT(queue);
    }
    DispatchQueueEndDebugUtilsLabelEXT(queue);
    EndQueueDebugUtilsLabel(layer_data->report_data, queue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueEndDebugUtilsLabelEXT(queue);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(
    VkQueue                                     queue,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
    InsertQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);
    DispatchQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordQueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
    DispatchCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBeginDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdEndDebugUtilsLabelEXT(commandBuffer);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
    DispatchCmdEndDebugUtilsLabelEXT(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdEndDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdEndDebugUtilsLabelEXT(commandBuffer);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugUtilsLabelEXT*                 pLabelInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdInsertDebugUtilsLabelEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
    DispatchCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdInsertDebugUtilsLabelEXT]) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    }
    VkResult result = DispatchCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    LayerCreateMessengerCallback(layer_data->report_data, false, pCreateInfo, pAllocator, pMessenger);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    }
    DispatchDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    LayerDestroyCallback(layer_data->report_data, messenger, pAllocator);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    }
    DispatchSubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAndroidHardwareBufferPropertiesANDROID]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAndroidHardwareBufferPropertiesANDROID]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    }
    VkResult result = DispatchGetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAndroidHardwareBufferPropertiesANDROID]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryAndroidHardwareBufferANDROID]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryAndroidHardwareBufferANDROID]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    }
    VkResult result = DispatchGetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryAndroidHardwareBufferANDROID]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetSampleLocationsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetSampleLocationsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    }
    DispatchCmdSetSampleLocationsEXT(commandBuffer, pSampleLocationsInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetSampleLocationsEXT]) {
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    }
    DispatchGetPhysicalDeviceMultisamplePropertiesEXT(physicalDevice, samples, pMultisampleProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    }
    VkResult result = DispatchGetImageDrmFormatModifierPropertiesEXT(device, image, pProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageDrmFormatModifierPropertiesEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindShadingRateImageNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindShadingRateImageNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    }
    DispatchCmdBindShadingRateImageNV(commandBuffer, imageView, imageLayout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindShadingRateImageNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportShadingRatePaletteNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportShadingRatePaletteNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    }
    DispatchCmdSetViewportShadingRatePaletteNV(commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportShadingRatePaletteNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoarseSampleOrderNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoarseSampleOrderNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    }
    DispatchCmdSetCoarseSampleOrderNV(commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoarseSampleOrderNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateAccelerationStructureNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    }
    VkResult result = DispatchCreateAccelerationStructureNV(device, pCreateInfo, pAllocator, pAccelerationStructure);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyAccelerationStructureNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    }
    DispatchDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyAccelerationStructureNV(device, accelerationStructure, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL GetAccelerationStructureMemoryRequirementsNV(
    VkDevice                                    device,
    const VkAccelerationStructureMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureMemoryRequirementsNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureMemoryRequirementsNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
    DispatchGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureMemoryRequirementsNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetAccelerationStructureMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindAccelerationStructureMemoryNV(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindAccelerationStructureMemoryNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindAccelerationStructureMemoryNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    }
    VkResult result = DispatchBindAccelerationStructureMemoryNV(device, bindInfoCount, pBindInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindAccelerationStructureMemoryNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBuildAccelerationStructureNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBuildAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    }
    DispatchCmdBuildAccelerationStructureNV(commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBuildAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyAccelerationStructureNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    }
    DispatchCmdCopyAccelerationStructureNV(commandBuffer, dst, src, mode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyAccelerationStructureNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdTraceRaysNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdTraceRaysNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    }
    DispatchCmdTraceRaysNV(commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdTraceRaysNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingShaderGroupHandlesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingShaderGroupHandlesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    }
    VkResult result = DispatchGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingShaderGroupHandlesKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingShaderGroupHandlesNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingShaderGroupHandlesNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    }
    VkResult result = DispatchGetRayTracingShaderGroupHandlesNV(device, pipeline, firstGroup, groupCount, dataSize, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingShaderGroupHandlesNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureHandleNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureHandleNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    }
    VkResult result = DispatchGetAccelerationStructureHandleNV(device, accelerationStructure, dataSize, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureHandleNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteAccelerationStructuresPropertiesNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteAccelerationStructuresPropertiesNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
    DispatchCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteAccelerationStructuresPropertiesNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteAccelerationStructuresPropertiesNV(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CompileDeferredNV(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    uint32_t                                    shader) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCompileDeferredNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCompileDeferredNV(device, pipeline, shader);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCompileDeferredNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCompileDeferredNV(device, pipeline, shader);
    }
    VkResult result = DispatchCompileDeferredNV(device, pipeline, shader);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCompileDeferredNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    }
    VkResult result = DispatchGetMemoryHostPointerPropertiesEXT(device, handleType, pHostPointer, pMemoryHostPointerProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryHostPointerPropertiesEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteBufferMarkerAMD]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteBufferMarkerAMD]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    }
    DispatchCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteBufferMarkerAMD]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteBufferMarkerAMD(commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceCalibrateableTimeDomainsEXT(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pTimeDomainCount,
    VkTimeDomainEXT*                            pTimeDomains) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    }
    VkResult result = DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(physicalDevice, pTimeDomainCount, pTimeDomains);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetCalibratedTimestampsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetCalibratedTimestampsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    }
    VkResult result = DispatchGetCalibratedTimestampsEXT(device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetCalibratedTimestampsEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    }
    DispatchCmdDrawMeshTasksNV(commandBuffer, taskCount, firstTask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksIndirectNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksIndirectNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawMeshTasksIndirectNV(commandBuffer, buffer, offset, drawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksIndirectNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksIndirectCountNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksIndirectCountNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawMeshTasksIndirectCountNV(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksIndirectCountNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetExclusiveScissorNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetExclusiveScissorNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    }
    DispatchCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetExclusiveScissorNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetExclusiveScissorNV(commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdSetCheckpointNV(
    VkCommandBuffer                             commandBuffer,
    const void*                                 pCheckpointMarker) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCheckpointNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCheckpointNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    }
    DispatchCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCheckpointNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCheckpointNV(commandBuffer, pCheckpointMarker);
    }
}

VKAPI_ATTR void VKAPI_CALL GetQueueCheckpointDataNV(
    VkQueue                                     queue,
    uint32_t*                                   pCheckpointDataCount,
    VkCheckpointDataNV*                         pCheckpointData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetQueueCheckpointDataNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetQueueCheckpointDataNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    }
    DispatchGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetQueueCheckpointDataNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetQueueCheckpointDataNV(queue, pCheckpointDataCount, pCheckpointData);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL InitializePerformanceApiINTEL(
    VkDevice                                    device,
    const VkInitializePerformanceApiInfoINTEL*  pInitializeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateInitializePerformanceApiINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateInitializePerformanceApiINTEL(device, pInitializeInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordInitializePerformanceApiINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo);
    }
    VkResult result = DispatchInitializePerformanceApiINTEL(device, pInitializeInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordInitializePerformanceApiINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordInitializePerformanceApiINTEL(device, pInitializeInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL UninitializePerformanceApiINTEL(
    VkDevice                                    device) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateUninitializePerformanceApiINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateUninitializePerformanceApiINTEL(device);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordUninitializePerformanceApiINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordUninitializePerformanceApiINTEL(device);
    }
    DispatchUninitializePerformanceApiINTEL(device);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordUninitializePerformanceApiINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordUninitializePerformanceApiINTEL(device);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceMarkerInfoINTEL*         pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPerformanceMarkerINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPerformanceMarkerINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    }
    VkResult result = DispatchCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPerformanceMarkerINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPerformanceMarkerINTEL(commandBuffer, pMarkerInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceStreamMarkerINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceStreamMarkerInfoINTEL*   pMarkerInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPerformanceStreamMarkerINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPerformanceStreamMarkerINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    }
    VkResult result = DispatchCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPerformanceStreamMarkerINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPerformanceStreamMarkerINTEL(commandBuffer, pMarkerInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CmdSetPerformanceOverrideINTEL(
    VkCommandBuffer                             commandBuffer,
    const VkPerformanceOverrideInfoINTEL*       pOverrideInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPerformanceOverrideINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPerformanceOverrideINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    }
    VkResult result = DispatchCmdSetPerformanceOverrideINTEL(commandBuffer, pOverrideInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPerformanceOverrideINTEL]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquirePerformanceConfigurationINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquirePerformanceConfigurationINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    }
    VkResult result = DispatchAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquirePerformanceConfigurationINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAcquirePerformanceConfigurationINTEL(device, pAcquireInfo, pConfiguration, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ReleasePerformanceConfigurationINTEL(
    VkDevice                                    device,
    VkPerformanceConfigurationINTEL             configuration) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleasePerformanceConfigurationINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateReleasePerformanceConfigurationINTEL(device, configuration);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleasePerformanceConfigurationINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordReleasePerformanceConfigurationINTEL(device, configuration);
    }
    VkResult result = DispatchReleasePerformanceConfigurationINTEL(device, configuration);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleasePerformanceConfigurationINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordReleasePerformanceConfigurationINTEL(device, configuration, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL QueueSetPerformanceConfigurationINTEL(
    VkQueue                                     queue,
    VkPerformanceConfigurationINTEL             configuration) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateQueueSetPerformanceConfigurationINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateQueueSetPerformanceConfigurationINTEL(queue, configuration);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordQueueSetPerformanceConfigurationINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordQueueSetPerformanceConfigurationINTEL(queue, configuration);
    }
    VkResult result = DispatchQueueSetPerformanceConfigurationINTEL(queue, configuration);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordQueueSetPerformanceConfigurationINTEL]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPerformanceParameterINTEL]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPerformanceParameterINTEL(device, parameter, pValue);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPerformanceParameterINTEL]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPerformanceParameterINTEL(device, parameter, pValue);
    }
    VkResult result = DispatchGetPerformanceParameterINTEL(device, parameter, pValue);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPerformanceParameterINTEL]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetLocalDimmingAMD]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetLocalDimmingAMD]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    }
    DispatchSetLocalDimmingAMD(device, swapChain, localDimmingEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetLocalDimmingAMD]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateImagePipeSurfaceFUCHSIA(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferDeviceAddressEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferDeviceAddressEXT(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferDeviceAddressEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferDeviceAddressEXT(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetBufferDeviceAddressEXT(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferDeviceAddressEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceCooperativeMatrixPropertiesNV(physicalDevice, pPropertyCount, pProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    }
    VkResult result = DispatchGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(physicalDevice, pCombinationCount, pCombinations);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    }
    VkResult result = DispatchGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceSurfacePresentModes2EXT(physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AcquireFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateAcquireFullScreenExclusiveModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquireFullScreenExclusiveModeEXT(device, swapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordAcquireFullScreenExclusiveModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain);
    }
    VkResult result = DispatchAcquireFullScreenExclusiveModeEXT(device, swapchain);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordAcquireFullScreenExclusiveModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAcquireFullScreenExclusiveModeEXT(device, swapchain, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ReleaseFullScreenExclusiveModeEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleaseFullScreenExclusiveModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateReleaseFullScreenExclusiveModeEXT(device, swapchain);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleaseFullScreenExclusiveModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordReleaseFullScreenExclusiveModeEXT(device, swapchain);
    }
    VkResult result = DispatchReleaseFullScreenExclusiveModeEXT(device, swapchain);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleaseFullScreenExclusiveModeEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceGroupSurfacePresentModes2EXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceGroupSurfacePresentModes2EXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    }
    VkResult result = DispatchGetDeviceGroupSurfacePresentModes2EXT(device, pSurfaceInfo, pModes);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceGroupSurfacePresentModes2EXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateHeadlessSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLineStippleEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineStippleEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    }
    DispatchCmdSetLineStippleEXT(commandBuffer, lineStippleFactor, lineStipplePattern);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineStippleEXT]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateResetQueryPoolEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordResetQueryPoolEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    }
    DispatchResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordResetQueryPoolEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordResetQueryPoolEXT(device, queryPool, firstQuery, queryCount);
    }
}



VKAPI_ATTR void VKAPI_CALL CmdSetCullModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkCullModeFlags                             cullMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCullModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCullModeEXT(commandBuffer, cullMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCullModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    }
    DispatchCmdSetCullModeEXT(commandBuffer, cullMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCullModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCullModeEXT(commandBuffer, cullMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetFrontFaceEXT(
    VkCommandBuffer                             commandBuffer,
    VkFrontFace                                 frontFace) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFrontFaceEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetFrontFaceEXT(commandBuffer, frontFace);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFrontFaceEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    }
    DispatchCmdSetFrontFaceEXT(commandBuffer, frontFace);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFrontFaceEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetFrontFaceEXT(commandBuffer, frontFace);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveTopologyEXT(
    VkCommandBuffer                             commandBuffer,
    VkPrimitiveTopology                         primitiveTopology) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPrimitiveTopologyEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    }
    DispatchCmdSetPrimitiveTopologyEXT(commandBuffer, primitiveTopology);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPrimitiveTopologyEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportWithCountEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportWithCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    }
    DispatchCmdSetViewportWithCountEXT(commandBuffer, viewportCount, pViewports);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportWithCountEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetScissorWithCountEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetScissorWithCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    }
    DispatchCmdSetScissorWithCountEXT(commandBuffer, scissorCount, pScissors);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetScissorWithCountEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
    DispatchCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindVertexBuffers2EXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindVertexBuffers2EXT(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    }
    DispatchCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthTestEnableEXT(commandBuffer, depthTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthWriteEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    }
    DispatchCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthWriteEnableEXT(commandBuffer, depthWriteEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthCompareOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkCompareOp                                 depthCompareOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    }
    DispatchCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthCompareOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthCompareOpEXT(commandBuffer, depthCompareOp);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBoundsTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBoundsTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    }
    DispatchCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBoundsTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBoundsTestEnableEXT(commandBuffer, depthBoundsTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetStencilTestEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stencilTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilTestEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    }
    DispatchCmdSetStencilTestEnableEXT(commandBuffer, stencilTestEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilTestEnableEXT]) {
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetStencilOpEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetStencilOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
    DispatchCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetStencilOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetStencilOpEXT(commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
    }
}




VKAPI_ATTR VkResult VKAPI_CALL ReleaseSwapchainImagesEXT(
    VkDevice                                    device,
    const VkReleaseSwapchainImagesInfoEXT*      pReleaseInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateReleaseSwapchainImagesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateReleaseSwapchainImagesEXT(device, pReleaseInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordReleaseSwapchainImagesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordReleaseSwapchainImagesEXT(device, pReleaseInfo);
    }
    VkResult result = DispatchReleaseSwapchainImagesEXT(device, pReleaseInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordReleaseSwapchainImagesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordReleaseSwapchainImagesEXT(device, pReleaseInfo, result);
    }
    return result;
}



VKAPI_ATTR void VKAPI_CALL GetGeneratedCommandsMemoryRequirementsNV(
    VkDevice                                    device,
    const VkGeneratedCommandsMemoryRequirementsInfoNV* pInfo,
    VkMemoryRequirements2*                      pMemoryRequirements) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetGeneratedCommandsMemoryRequirementsNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetGeneratedCommandsMemoryRequirementsNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
    DispatchGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetGeneratedCommandsMemoryRequirementsNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetGeneratedCommandsMemoryRequirementsNV(device, pInfo, pMemoryRequirements);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdPreprocessGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdPreprocessGeneratedCommandsNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdPreprocessGeneratedCommandsNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    }
    DispatchCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdPreprocessGeneratedCommandsNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdPreprocessGeneratedCommandsNV(commandBuffer, pGeneratedCommandsInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdExecuteGeneratedCommandsNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    isPreprocessed,
    const VkGeneratedCommandsInfoNV*            pGeneratedCommandsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdExecuteGeneratedCommandsNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdExecuteGeneratedCommandsNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    }
    DispatchCmdExecuteGeneratedCommandsNV(commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdExecuteGeneratedCommandsNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindPipelineShaderGroupNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindPipelineShaderGroupNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    }
    DispatchCmdBindPipelineShaderGroupNV(commandBuffer, pipelineBindPoint, pipeline, groupIndex);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindPipelineShaderGroupNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateIndirectCommandsLayoutNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateIndirectCommandsLayoutNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    }
    VkResult result = DispatchCreateIndirectCommandsLayoutNV(device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateIndirectCommandsLayoutNV]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyIndirectCommandsLayoutNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyIndirectCommandsLayoutNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    }
    DispatchDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyIndirectCommandsLayoutNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyIndirectCommandsLayoutNV(device, indirectCommandsLayout, pAllocator);
    }
}






VKAPI_ATTR VkResult VKAPI_CALL AcquireDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    VkDisplayKHR                                display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquireDrmDisplayEXT(physicalDevice, drmFd, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireDrmDisplayEXT(physicalDevice, drmFd, display);
    }
    VkResult result = DispatchAcquireDrmDisplayEXT(physicalDevice, drmFd, display);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordAcquireDrmDisplayEXT(physicalDevice, drmFd, display, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDrmDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    int32_t                                     drmFd,
    uint32_t                                    connectorId,
    VkDisplayKHR*                               display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
    }
    VkResult result = DispatchGetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDrmDisplayEXT(physicalDevice, drmFd, connectorId, display, result);
    }
    return result;
}






VKAPI_ATTR VkResult VKAPI_CALL CreatePrivateDataSlotEXT(
    VkDevice                                    device,
    const VkPrivateDataSlotCreateInfo*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPrivateDataSlot*                          pPrivateDataSlot) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreatePrivateDataSlotEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreatePrivateDataSlotEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    }
    VkResult result = DispatchCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreatePrivateDataSlotEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreatePrivateDataSlotEXT(device, pCreateInfo, pAllocator, pPrivateDataSlot, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyPrivateDataSlotEXT(
    VkDevice                                    device,
    VkPrivateDataSlot                           privateDataSlot,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyPrivateDataSlotEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyPrivateDataSlotEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    }
    DispatchDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyPrivateDataSlotEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyPrivateDataSlotEXT(device, privateDataSlot, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL SetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t                                    data) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetPrivateDataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetPrivateDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    }
    VkResult result = DispatchSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetPrivateDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, data, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL GetPrivateDataEXT(
    VkDevice                                    device,
    VkObjectType                                objectType,
    uint64_t                                    objectHandle,
    VkPrivateDataSlot                           privateDataSlot,
    uint64_t*                                   pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPrivateDataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPrivateDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    }
    DispatchGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPrivateDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPrivateDataEXT(device, objectType, objectHandle, privateDataSlot, pData);
    }
}




#ifdef VK_USE_PLATFORM_METAL_EXT

VKAPI_ATTR void VKAPI_CALL ExportMetalObjectsEXT(
    VkDevice                                    device,
    VkExportMetalObjectsInfoEXT*                pMetalObjectsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateExportMetalObjectsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateExportMetalObjectsEXT(device, pMetalObjectsInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordExportMetalObjectsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordExportMetalObjectsEXT(device, pMetalObjectsInfo);
    }
    DispatchExportMetalObjectsEXT(device, pMetalObjectsInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordExportMetalObjectsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordExportMetalObjectsEXT(device, pMetalObjectsInfo);
    }
}
#endif // VK_USE_PLATFORM_METAL_EXT


VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSizeEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    VkDeviceSize*                               pLayoutSizeInBytes) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetLayoutSizeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutSizeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
    }
    DispatchGetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutSizeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDescriptorSetLayoutSizeEXT(device, layout, pLayoutSizeInBytes);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutBindingOffsetEXT(
    VkDevice                                    device,
    VkDescriptorSetLayout                       layout,
    uint32_t                                    binding,
    VkDeviceSize*                               pOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetLayoutBindingOffsetEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutBindingOffsetEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
    }
    DispatchGetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutBindingOffsetEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDescriptorSetLayoutBindingOffsetEXT(device, layout, binding, pOffset);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorEXT(
    VkDevice                                    device,
    const VkDescriptorGetInfoEXT*               pDescriptorInfo,
    size_t                                      dataSize,
    void*                                       pDescriptor) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDescriptorEXT(device, pDescriptorInfo, dataSize, pDescriptor);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorEXT(device, pDescriptorInfo, dataSize, pDescriptor);
    }
    DispatchGetDescriptorEXT(device, pDescriptorInfo, dataSize, pDescriptor);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDescriptorEXT(device, pDescriptorInfo, dataSize, pDescriptor);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBuffersEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    bufferCount,
    const VkDescriptorBufferBindingInfoEXT*     pBindingInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindDescriptorBuffersEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindDescriptorBuffersEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
    }
    DispatchCmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindDescriptorBuffersEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindDescriptorBuffersEXT(commandBuffer, bufferCount, pBindingInfos);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDescriptorBufferOffsetsEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    setCount,
    const uint32_t*                             pBufferIndices,
    const VkDeviceSize*                         pOffsets) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDescriptorBufferOffsetsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDescriptorBufferOffsetsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
    }
    DispatchCmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDescriptorBufferOffsetsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDescriptorBufferOffsetsEXT(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pBufferIndices, pOffsets);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBindDescriptorBufferEmbeddedSamplersEXT(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
    }
    DispatchCmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindDescriptorBufferEmbeddedSamplersEXT(commandBuffer, pipelineBindPoint, layout, set);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetBufferOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkBufferCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    }
    VkResult result = DispatchGetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetImageOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageCaptureDescriptorDataInfoEXT*  pInfo,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    }
    VkResult result = DispatchGetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetImageViewOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkImageViewCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    }
    VkResult result = DispatchGetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageViewOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSamplerOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkSamplerCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    }
    VkResult result = DispatchGetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSamplerOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(
    VkDevice                                    device,
    const VkAccelerationStructureCaptureDescriptorDataInfoEXT* pInfo,
    void*                                       pData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    }
    VkResult result = DispatchGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(device, pInfo, pData, result);
    }
    return result;
}




VKAPI_ATTR void VKAPI_CALL CmdSetFragmentShadingRateEnumNV(
    VkCommandBuffer                             commandBuffer,
    VkFragmentShadingRateNV                     shadingRate,
    const VkFragmentShadingRateCombinerOpKHR    combinerOps[2]) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetFragmentShadingRateEnumNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetFragmentShadingRateEnumNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
    }
    DispatchCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetFragmentShadingRateEnumNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetFragmentShadingRateEnumNV(commandBuffer, shadingRate, combinerOps);
    }
}







VKAPI_ATTR void VKAPI_CALL GetImageSubresourceLayout2EXT(
    VkDevice                                    device,
    VkImage                                     image,
    const VkImageSubresource2EXT*               pSubresource,
    VkSubresourceLayout2EXT*                    pLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetImageSubresourceLayout2EXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetImageSubresourceLayout2EXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
    }
    DispatchGetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetImageSubresourceLayout2EXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetImageSubresourceLayout2EXT(device, image, pSubresource, pLayout);
    }
}




VKAPI_ATTR VkResult VKAPI_CALL GetDeviceFaultInfoEXT(
    VkDevice                                    device,
    VkDeviceFaultCountsEXT*                     pFaultCounts,
    VkDeviceFaultInfoEXT*                       pFaultInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceFaultInfoEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceFaultInfoEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);
    }
    VkResult result = DispatchGetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceFaultInfoEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceFaultInfoEXT(device, pFaultCounts, pFaultInfo, result);
    }
    return result;
}



#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL AcquireWinrtDisplayNV(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateAcquireWinrtDisplayNV(physicalDevice, display);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordAcquireWinrtDisplayNV(physicalDevice, display);
    }
    VkResult result = DispatchAcquireWinrtDisplayNV(physicalDevice, display);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    }
    VkResult result = DispatchGetWinrtDisplayNV(physicalDevice, deviceRelativeId, pDisplay);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
        if (skip) return VK_FALSE;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    }
    VkBool32 result = DispatchGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceDirectFBPresentationSupportEXT(physicalDevice, queueFamilyIndex, dfb);
    }
    return result;
}
#endif // VK_USE_PLATFORM_DIRECTFB_EXT



VKAPI_ATTR void VKAPI_CALL CmdSetVertexInputEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexBindingDescriptionCount,
    const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
    uint32_t                                    vertexAttributeDescriptionCount,
    const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetVertexInputEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetVertexInputEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    }
    DispatchCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetVertexInputEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetVertexInputEXT(commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    }
}





#ifdef VK_USE_PLATFORM_FUCHSIA

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkMemoryGetZirconHandleInfoFUCHSIA*   pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryZirconHandleFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryZirconHandleFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    }
    VkResult result = DispatchGetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryZirconHandleFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMemoryZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBits          handleType,
    zx_handle_t                                 zirconHandle,
    VkMemoryZirconHandlePropertiesFUCHSIA*      pMemoryZirconHandleProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryZirconHandlePropertiesFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle, pMemoryZirconHandleProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle, pMemoryZirconHandleProperties);
    }
    VkResult result = DispatchGetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle, pMemoryZirconHandleProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMemoryZirconHandlePropertiesFUCHSIA(device, handleType, zirconHandle, pMemoryZirconHandleProperties, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

VKAPI_ATTR VkResult VKAPI_CALL ImportSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pImportSemaphoreZirconHandleInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateImportSemaphoreZirconHandleFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordImportSemaphoreZirconHandleFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo);
    }
    VkResult result = DispatchImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordImportSemaphoreZirconHandleFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordImportSemaphoreZirconHandleFUCHSIA(device, pImportSemaphoreZirconHandleInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSemaphoreZirconHandleFUCHSIA(
    VkDevice                                    device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pGetZirconHandleInfo,
    zx_handle_t*                                pZirconHandle) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetSemaphoreZirconHandleFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetSemaphoreZirconHandleFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    }
    VkResult result = DispatchGetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetSemaphoreZirconHandleFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetSemaphoreZirconHandleFUCHSIA(device, pGetZirconHandleInfo, pZirconHandle, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_FUCHSIA

#ifdef VK_USE_PLATFORM_FUCHSIA

VKAPI_ATTR VkResult VKAPI_CALL CreateBufferCollectionFUCHSIA(
    VkDevice                                    device,
    const VkBufferCollectionCreateInfoFUCHSIA*  pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBufferCollectionFUCHSIA*                  pCollection) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateBufferCollectionFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateBufferCollectionFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
    }
    VkResult result = DispatchCreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateBufferCollectionFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateBufferCollectionFUCHSIA(device, pCreateInfo, pAllocator, pCollection, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkImageConstraintsInfoFUCHSIA*        pImageConstraintsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetBufferCollectionImageConstraintsFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetBufferCollectionImageConstraintsFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);
    }
    VkResult result = DispatchSetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetBufferCollectionImageConstraintsFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetBufferCollectionImageConstraintsFUCHSIA(device, collection, pImageConstraintsInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkBufferConstraintsInfoFUCHSIA*       pBufferConstraintsInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);
    }
    VkResult result = DispatchSetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetBufferCollectionBufferConstraintsFUCHSIA(device, collection, pBufferConstraintsInfo, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyBufferCollectionFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyBufferCollectionFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyBufferCollectionFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
    }
    DispatchDestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyBufferCollectionFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyBufferCollectionFUCHSIA(device, collection, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetBufferCollectionPropertiesFUCHSIA(
    VkDevice                                    device,
    VkBufferCollectionFUCHSIA                   collection,
    VkBufferCollectionPropertiesFUCHSIA*        pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetBufferCollectionPropertiesFUCHSIA]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetBufferCollectionPropertiesFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
    }
    VkResult result = DispatchGetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetBufferCollectionPropertiesFUCHSIA]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetBufferCollectionPropertiesFUCHSIA(device, collection, pProperties, result);
    }
    return result;
}
#endif // VK_USE_PLATFORM_FUCHSIA


VKAPI_ATTR VkResult VKAPI_CALL GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(
    VkDevice                                    device,
    VkRenderPass                                renderpass,
    VkExtent2D*                                 pMaxWorkgroupSize) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);
    }
    VkResult result = DispatchGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(device, renderpass, pMaxWorkgroupSize, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdSubpassShadingHUAWEI(
    VkCommandBuffer                             commandBuffer) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSubpassShadingHUAWEI]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSubpassShadingHUAWEI(commandBuffer);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSubpassShadingHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSubpassShadingHUAWEI(commandBuffer);
    }
    DispatchCmdSubpassShadingHUAWEI(commandBuffer);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSubpassShadingHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSubpassShadingHUAWEI(commandBuffer);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdBindInvocationMaskHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkImageView                                 imageView,
    VkImageLayout                               imageLayout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBindInvocationMaskHUAWEI]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBindInvocationMaskHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
    }
    DispatchCmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBindInvocationMaskHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBindInvocationMaskHUAWEI(commandBuffer, imageView, imageLayout);
    }
}


VKAPI_ATTR VkResult VKAPI_CALL GetMemoryRemoteAddressNV(
    VkDevice                                    device,
    const VkMemoryGetRemoteAddressInfoNV*       pMemoryGetRemoteAddressInfo,
    VkRemoteAddressNV*                          pAddress) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMemoryRemoteAddressNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMemoryRemoteAddressNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress);
    }
    VkResult result = DispatchGetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMemoryRemoteAddressNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMemoryRemoteAddressNV(device, pMemoryGetRemoteAddressInfo, pAddress, result);
    }
    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL GetPipelinePropertiesEXT(
    VkDevice                                    device,
    const VkPipelineInfoEXT*                    pPipelineInfo,
    VkBaseOutStructure*                         pPipelineProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetPipelinePropertiesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetPipelinePropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);
    }
    VkResult result = DispatchGetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetPipelinePropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPipelinePropertiesEXT(device, pPipelineInfo, pPipelineProperties, result);
    }
    return result;
}



VKAPI_ATTR void VKAPI_CALL CmdSetPatchControlPointsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    patchControlPoints) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPatchControlPointsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPatchControlPointsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
    }
    DispatchCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPatchControlPointsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPatchControlPointsEXT(commandBuffer, patchControlPoints);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizerDiscardEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    rasterizerDiscardEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRasterizerDiscardEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRasterizerDiscardEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
    }
    DispatchCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRasterizerDiscardEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetRasterizerDiscardEnableEXT(commandBuffer, rasterizerDiscardEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthBiasEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthBiasEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthBiasEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthBiasEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
    }
    DispatchCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthBiasEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthBiasEnableEXT(commandBuffer, depthBiasEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEXT(
    VkCommandBuffer                             commandBuffer,
    VkLogicOp                                   logicOp) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLogicOpEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetLogicOpEXT(commandBuffer, logicOp);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLogicOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLogicOpEXT(commandBuffer, logicOp);
    }
    DispatchCmdSetLogicOpEXT(commandBuffer, logicOp);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLogicOpEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetLogicOpEXT(commandBuffer, logicOp);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPrimitiveRestartEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    primitiveRestartEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPrimitiveRestartEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPrimitiveRestartEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
    }
    DispatchCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPrimitiveRestartEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPrimitiveRestartEnableEXT(commandBuffer, primitiveRestartEnable);
    }
}

#ifdef VK_USE_PLATFORM_SCREEN_QNX

VKAPI_ATTR VkResult VKAPI_CALL CreateScreenSurfaceQNX(
    VkInstance                                  instance,
    const VkScreenSurfaceCreateInfoQNX*         pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
    }
    VkResult result = DispatchCreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface, result);
    }
    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GetPhysicalDeviceScreenPresentationSupportQNX(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct _screen_window*                      window) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);
        if (skip) return VK_FALSE;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);
    }
    VkBool32 result = DispatchGetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceScreenPresentationSupportQNX(physicalDevice, queueFamilyIndex, window);
    }
    return result;
}
#endif // VK_USE_PLATFORM_SCREEN_QNX


VKAPI_ATTR void                                    VKAPI_CALL CmdSetColorWriteEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorWriteEnables) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetColorWriteEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetColorWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
    }
    DispatchCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetColorWriteEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetColorWriteEnableEXT(commandBuffer, attachmentCount, pColorWriteEnables);
    }
}





VKAPI_ATTR void VKAPI_CALL CmdDrawMultiEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawInfoEXT*                   pVertexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMultiEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMultiEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
    }
    DispatchCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMultiEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawMultiEXT(commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMultiIndexedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    drawCount,
    const VkMultiDrawIndexedInfoEXT*            pIndexInfo,
    uint32_t                                    instanceCount,
    uint32_t                                    firstInstance,
    uint32_t                                    stride,
    const int32_t*                              pVertexOffset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMultiIndexedEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMultiIndexedEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
    }
    DispatchCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMultiIndexedEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawMultiIndexedEXT(commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL CreateMicromapEXT(
    VkDevice                                    device,
    const VkMicromapCreateInfoEXT*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkMicromapEXT*                              pMicromap) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateMicromapEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap);
    }
    VkResult result = DispatchCreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateMicromapEXT(device, pCreateInfo, pAllocator, pMicromap, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyMicromapEXT(
    VkDevice                                    device,
    VkMicromapEXT                               micromap,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyMicromapEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyMicromapEXT(device, micromap, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyMicromapEXT(device, micromap, pAllocator);
    }
    DispatchDestroyMicromapEXT(device, micromap, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyMicromapEXT(device, micromap, pAllocator);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBuildMicromapsEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBuildMicromapsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBuildMicromapsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
    }
    DispatchCmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBuildMicromapsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdBuildMicromapsEXT(commandBuffer, infoCount, pInfos);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BuildMicromapsEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    uint32_t                                    infoCount,
    const VkMicromapBuildInfoEXT*               pInfos) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBuildMicromapsEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBuildMicromapsEXT(device, deferredOperation, infoCount, pInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBuildMicromapsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBuildMicromapsEXT(device, deferredOperation, infoCount, pInfos);
    }
    VkResult result = DispatchBuildMicromapsEXT(device, deferredOperation, infoCount, pInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBuildMicromapsEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBuildMicromapsEXT(device, deferredOperation, infoCount, pInfos, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapInfoEXT*                pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyMicromapEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCopyMicromapEXT(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCopyMicromapEXT(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyMicromapEXT(device, deferredOperation, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCopyMicromapEXT(device, deferredOperation, pInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMicromapToMemoryEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyMicromapToMemoryEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCopyMicromapToMemoryEXT(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyMicromapToMemoryEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCopyMicromapToMemoryEXT(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyMicromapToMemoryEXT(device, deferredOperation, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyMicromapToMemoryEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCopyMicromapToMemoryEXT(device, deferredOperation, pInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CopyMemoryToMicromapEXT(
    VkDevice                                    device,
    VkDeferredOperationKHR                      deferredOperation,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyMemoryToMicromapEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCopyMemoryToMicromapEXT(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyMemoryToMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCopyMemoryToMicromapEXT(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyMemoryToMicromapEXT(device, deferredOperation, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyMemoryToMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCopyMemoryToMicromapEXT(device, deferredOperation, pInfo, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL WriteMicromapsPropertiesEXT(
    VkDevice                                    device,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    size_t                                      dataSize,
    void*                                       pData,
    size_t                                      stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWriteMicromapsPropertiesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateWriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWriteMicromapsPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
    }
    VkResult result = DispatchWriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWriteMicromapsPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordWriteMicromapsPropertiesEXT(device, micromapCount, pMicromaps, queryType, dataSize, pData, stride, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapInfoEXT*                pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyMicromapEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyMicromapEXT(commandBuffer, pInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyMicromapEXT(commandBuffer, pInfo);
    }
    DispatchCmdCopyMicromapEXT(commandBuffer, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyMicromapEXT(commandBuffer, pInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMicromapToMemoryEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMicromapToMemoryInfoEXT*        pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyMicromapToMemoryEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyMicromapToMemoryEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
    }
    DispatchCmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyMicromapToMemoryEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyMicromapToMemoryEXT(commandBuffer, pInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToMicromapEXT(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToMicromapInfoEXT*        pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyMemoryToMicromapEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyMemoryToMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
    }
    DispatchCmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyMemoryToMicromapEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyMemoryToMicromapEXT(commandBuffer, pInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdWriteMicromapsPropertiesEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    micromapCount,
    const VkMicromapEXT*                        pMicromaps,
    VkQueryType                                 queryType,
    VkQueryPool                                 queryPool,
    uint32_t                                    firstQuery) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteMicromapsPropertiesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteMicromapsPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
    }
    DispatchCmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteMicromapsPropertiesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteMicromapsPropertiesEXT(commandBuffer, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceMicromapCompatibilityEXT(
    VkDevice                                    device,
    const VkMicromapVersionInfoEXT*             pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceMicromapCompatibilityEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceMicromapCompatibilityEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
    }
    DispatchGetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceMicromapCompatibilityEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDeviceMicromapCompatibilityEXT(device, pVersionInfo, pCompatibility);
    }
}

VKAPI_ATTR void VKAPI_CALL GetMicromapBuildSizesEXT(
    VkDevice                                    device,
    VkAccelerationStructureBuildTypeKHR         buildType,
    const VkMicromapBuildInfoEXT*               pBuildInfo,
    VkMicromapBuildSizesInfoEXT*                pSizeInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetMicromapBuildSizesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetMicromapBuildSizesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
    }
    DispatchGetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetMicromapBuildSizesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetMicromapBuildSizesEXT(device, buildType, pBuildInfo, pSizeInfo);
    }
}



VKAPI_ATTR void VKAPI_CALL CmdDrawClusterHUAWEI(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawClusterHUAWEI]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawClusterHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawClusterHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawClusterHUAWEI(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawClusterIndirectHUAWEI(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawClusterIndirectHUAWEI]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawClusterIndirectHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
    }
    DispatchCmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawClusterIndirectHUAWEI]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawClusterIndirectHUAWEI(commandBuffer, buffer, offset);
    }
}



VKAPI_ATTR void VKAPI_CALL SetDeviceMemoryPriorityEXT(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    float                                       priority) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateSetDeviceMemoryPriorityEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateSetDeviceMemoryPriorityEXT(device, memory, priority);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordSetDeviceMemoryPriorityEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordSetDeviceMemoryPriorityEXT(device, memory, priority);
    }
    DispatchSetDeviceMemoryPriorityEXT(device, memory, priority);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordSetDeviceMemoryPriorityEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordSetDeviceMemoryPriorityEXT(device, memory, priority);
    }
}


VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutHostMappingInfoVALVE(
    VkDevice                                    device,
    const VkDescriptorSetBindingReferenceVALVE* pBindingReference,
    VkDescriptorSetLayoutHostMappingInfoVALVE*  pHostMapping) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference, pHostMapping);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference, pHostMapping);
    }
    DispatchGetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference, pHostMapping);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDescriptorSetLayoutHostMappingInfoVALVE(device, pBindingReference, pHostMapping);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetHostMappingVALVE(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    void**                                      ppData) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDescriptorSetHostMappingVALVE]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDescriptorSetHostMappingVALVE]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
    }
    DispatchGetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDescriptorSetHostMappingVALVE]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDescriptorSetHostMappingVALVE(device, descriptorSet, ppData);
    }
}





VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyMemoryIndirectNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyMemoryIndirectNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
    }
    DispatchCmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyMemoryIndirectNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyMemoryIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToImageIndirectNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             copyBufferAddress,
    uint32_t                                    copyCount,
    uint32_t                                    stride,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    const VkImageSubresourceLayers*             pImageSubresources) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyMemoryToImageIndirectNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage, dstImageLayout, pImageSubresources);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyMemoryToImageIndirectNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage, dstImageLayout, pImageSubresources);
    }
    DispatchCmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage, dstImageLayout, pImageSubresources);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyMemoryToImageIndirectNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyMemoryToImageIndirectNV(commandBuffer, copyBufferAddress, copyCount, stride, dstImage, dstImageLayout, pImageSubresources);
    }
}


VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    decompressRegionCount,
    const VkDecompressMemoryRegionNV*           pDecompressMemoryRegions) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDecompressMemoryNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDecompressMemoryNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
    }
    DispatchCmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDecompressMemoryNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDecompressMemoryNV(commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDecompressMemoryIndirectCountNV(
    VkCommandBuffer                             commandBuffer,
    VkDeviceAddress                             indirectCommandsAddress,
    VkDeviceAddress                             indirectCommandsCountAddress,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDecompressMemoryIndirectCountNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDecompressMemoryIndirectCountNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress, stride);
    }
    DispatchCmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDecompressMemoryIndirectCountNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDecompressMemoryIndirectCountNV(commandBuffer, indirectCommandsAddress, indirectCommandsCountAddress, stride);
    }
}






VKAPI_ATTR void VKAPI_CALL CmdSetTessellationDomainOriginEXT(
    VkCommandBuffer                             commandBuffer,
    VkTessellationDomainOrigin                  domainOrigin) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetTessellationDomainOriginEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetTessellationDomainOriginEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
    }
    DispatchCmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetTessellationDomainOriginEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetTessellationDomainOriginEXT(commandBuffer, domainOrigin);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthClampEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClampEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthClampEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthClampEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
    }
    DispatchCmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthClampEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthClampEnableEXT(commandBuffer, depthClampEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetPolygonModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkPolygonMode                               polygonMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetPolygonModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetPolygonModeEXT(commandBuffer, polygonMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetPolygonModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetPolygonModeEXT(commandBuffer, polygonMode);
    }
    DispatchCmdSetPolygonModeEXT(commandBuffer, polygonMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetPolygonModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetPolygonModeEXT(commandBuffer, polygonMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationSamplesEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       rasterizationSamples) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRasterizationSamplesEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRasterizationSamplesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
    }
    DispatchCmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRasterizationSamplesEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetRasterizationSamplesEXT(commandBuffer, rasterizationSamples);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetSampleMaskEXT(
    VkCommandBuffer                             commandBuffer,
    VkSampleCountFlagBits                       samples,
    const VkSampleMask*                         pSampleMask) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetSampleMaskEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetSampleMaskEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
    }
    DispatchCmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetSampleMaskEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetSampleMaskEXT(commandBuffer, samples, pSampleMask);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToCoverageEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToCoverageEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetAlphaToCoverageEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetAlphaToCoverageEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
    }
    DispatchCmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetAlphaToCoverageEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetAlphaToCoverageEnableEXT(commandBuffer, alphaToCoverageEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetAlphaToOneEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    alphaToOneEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetAlphaToOneEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetAlphaToOneEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
    }
    DispatchCmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetAlphaToOneEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetAlphaToOneEnableEXT(commandBuffer, alphaToOneEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLogicOpEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    logicOpEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLogicOpEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLogicOpEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
    }
    DispatchCmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLogicOpEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetLogicOpEnableEXT(commandBuffer, logicOpEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEnableEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkBool32*                             pColorBlendEnables) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetColorBlendEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetColorBlendEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
    }
    DispatchCmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetColorBlendEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetColorBlendEnableEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendEquationEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendEquationEXT*              pColorBlendEquations) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetColorBlendEquationEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetColorBlendEquationEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
    }
    DispatchCmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetColorBlendEquationEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetColorBlendEquationEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorWriteMaskEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorComponentFlags*                pColorWriteMasks) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetColorWriteMaskEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetColorWriteMaskEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
    }
    DispatchCmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetColorWriteMaskEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetColorWriteMaskEXT(commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetRasterizationStreamEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    rasterizationStream) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRasterizationStreamEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRasterizationStreamEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
    }
    DispatchCmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRasterizationStreamEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetRasterizationStreamEXT(commandBuffer, rasterizationStream);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetConservativeRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkConservativeRasterizationModeEXT          conservativeRasterizationMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetConservativeRasterizationModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetConservativeRasterizationModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
    }
    DispatchCmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetConservativeRasterizationModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetConservativeRasterizationModeEXT(commandBuffer, conservativeRasterizationMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetExtraPrimitiveOverestimationSizeEXT(
    VkCommandBuffer                             commandBuffer,
    float                                       extraPrimitiveOverestimationSize) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
    }
    DispatchCmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetExtraPrimitiveOverestimationSizeEXT(commandBuffer, extraPrimitiveOverestimationSize);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    depthClipEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthClipEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthClipEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
    }
    DispatchCmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthClipEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthClipEnableEXT(commandBuffer, depthClipEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetSampleLocationsEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    sampleLocationsEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetSampleLocationsEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetSampleLocationsEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
    }
    DispatchCmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetSampleLocationsEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetSampleLocationsEnableEXT(commandBuffer, sampleLocationsEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetColorBlendAdvancedEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstAttachment,
    uint32_t                                    attachmentCount,
    const VkColorBlendAdvancedEXT*              pColorBlendAdvanced) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetColorBlendAdvancedEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetColorBlendAdvancedEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
    }
    DispatchCmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetColorBlendAdvancedEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetColorBlendAdvancedEXT(commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetProvokingVertexModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkProvokingVertexModeEXT                    provokingVertexMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetProvokingVertexModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetProvokingVertexModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
    }
    DispatchCmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetProvokingVertexModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetProvokingVertexModeEXT(commandBuffer, provokingVertexMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineRasterizationModeEXT(
    VkCommandBuffer                             commandBuffer,
    VkLineRasterizationModeEXT                  lineRasterizationMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLineRasterizationModeEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineRasterizationModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
    }
    DispatchCmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineRasterizationModeEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetLineRasterizationModeEXT(commandBuffer, lineRasterizationMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetLineStippleEnableEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    stippledLineEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetLineStippleEnableEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetLineStippleEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
    }
    DispatchCmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetLineStippleEnableEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetLineStippleEnableEXT(commandBuffer, stippledLineEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetDepthClipNegativeOneToOneEXT(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    negativeOneToOne) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetDepthClipNegativeOneToOneEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetDepthClipNegativeOneToOneEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
    }
    DispatchCmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetDepthClipNegativeOneToOneEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetDepthClipNegativeOneToOneEXT(commandBuffer, negativeOneToOne);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportWScalingEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    viewportWScalingEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportWScalingEnableNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportWScalingEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
    }
    DispatchCmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportWScalingEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetViewportWScalingEnableNV(commandBuffer, viewportWScalingEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetViewportSwizzleNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportSwizzleNV*                  pViewportSwizzles) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetViewportSwizzleNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetViewportSwizzleNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
    }
    DispatchCmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetViewportSwizzleNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetViewportSwizzleNV(commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageToColorEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoverageToColorEnableNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoverageToColorEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
    }
    DispatchCmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoverageToColorEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCoverageToColorEnableNV(commandBuffer, coverageToColorEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageToColorLocationNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageToColorLocation) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoverageToColorLocationNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoverageToColorLocationNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
    }
    DispatchCmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoverageToColorLocationNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCoverageToColorLocationNV(commandBuffer, coverageToColorLocation);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageModulationModeNV                  coverageModulationMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoverageModulationModeNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoverageModulationModeNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
    }
    DispatchCmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoverageModulationModeNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCoverageModulationModeNV(commandBuffer, coverageModulationMode);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    coverageModulationTableEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoverageModulationTableEnableNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoverageModulationTableEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
    }
    DispatchCmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoverageModulationTableEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCoverageModulationTableEnableNV(commandBuffer, coverageModulationTableEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageModulationTableNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    coverageModulationTableCount,
    const float*                                pCoverageModulationTable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoverageModulationTableNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoverageModulationTableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
    }
    DispatchCmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoverageModulationTableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCoverageModulationTableNV(commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetShadingRateImageEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    shadingRateImageEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetShadingRateImageEnableNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetShadingRateImageEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
    }
    DispatchCmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetShadingRateImageEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetShadingRateImageEnableNV(commandBuffer, shadingRateImageEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetRepresentativeFragmentTestEnableNV(
    VkCommandBuffer                             commandBuffer,
    VkBool32                                    representativeFragmentTestEnable) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRepresentativeFragmentTestEnableNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRepresentativeFragmentTestEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
    }
    DispatchCmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRepresentativeFragmentTestEnableNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetRepresentativeFragmentTestEnableNV(commandBuffer, representativeFragmentTestEnable);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdSetCoverageReductionModeNV(
    VkCommandBuffer                             commandBuffer,
    VkCoverageReductionModeNV                   coverageReductionMode) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetCoverageReductionModeNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetCoverageReductionModeNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
    }
    DispatchCmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetCoverageReductionModeNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetCoverageReductionModeNV(commandBuffer, coverageReductionMode);
    }
}




VKAPI_ATTR void VKAPI_CALL GetShaderModuleIdentifierEXT(
    VkDevice                                    device,
    VkShaderModule                              shaderModule,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetShaderModuleIdentifierEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetShaderModuleIdentifierEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
    }
    DispatchGetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetShaderModuleIdentifierEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetShaderModuleIdentifierEXT(device, shaderModule, pIdentifier);
    }
}

VKAPI_ATTR void VKAPI_CALL GetShaderModuleCreateInfoIdentifierEXT(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    VkShaderModuleIdentifierEXT*                pIdentifier) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetShaderModuleCreateInfoIdentifierEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetShaderModuleCreateInfoIdentifierEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
    }
    DispatchGetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetShaderModuleCreateInfoIdentifierEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetShaderModuleCreateInfoIdentifierEXT(device, pCreateInfo, pIdentifier);
    }
}



VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceOpticalFlowImageFormatsNV(
    VkPhysicalDevice                            physicalDevice,
    const VkOpticalFlowImageFormatInfoNV*       pOpticalFlowImageFormatInfo,
    uint32_t*                                   pFormatCount,
    VkOpticalFlowImageFormatPropertiesNV*       pImageFormatProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);
    }
    VkResult result = DispatchGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);
    for (ValidationObject* intercept : layer_data->object_dispatch) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetPhysicalDeviceOpticalFlowImageFormatsNV(physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateOpticalFlowSessionNV(
    VkDevice                                    device,
    const VkOpticalFlowSessionCreateInfoNV*     pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkOpticalFlowSessionNV*                     pSession) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateOpticalFlowSessionNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateOpticalFlowSessionNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
    }
    VkResult result = DispatchCreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateOpticalFlowSessionNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCreateOpticalFlowSessionNV(device, pCreateInfo, pAllocator, pSession, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyOpticalFlowSessionNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    const VkAllocationCallbacks*                pAllocator) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyOpticalFlowSessionNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyOpticalFlowSessionNV(device, session, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyOpticalFlowSessionNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyOpticalFlowSessionNV(device, session, pAllocator);
    }
    DispatchDestroyOpticalFlowSessionNV(device, session, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyOpticalFlowSessionNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordDestroyOpticalFlowSessionNV(device, session, pAllocator);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL BindOpticalFlowSessionImageNV(
    VkDevice                                    device,
    VkOpticalFlowSessionNV                      session,
    VkOpticalFlowSessionBindingPointNV          bindingPoint,
    VkImageView                                 view,
    VkImageLayout                               layout) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBindOpticalFlowSessionImageNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBindOpticalFlowSessionImageNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);
    }
    VkResult result = DispatchBindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBindOpticalFlowSessionImageNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordBindOpticalFlowSessionImageNV(device, session, bindingPoint, view, layout, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdOpticalFlowExecuteNV(
    VkCommandBuffer                             commandBuffer,
    VkOpticalFlowSessionNV                      session,
    const VkOpticalFlowExecuteInfoNV*           pExecuteInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdOpticalFlowExecuteNV]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdOpticalFlowExecuteNV]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
    }
    DispatchCmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdOpticalFlowExecuteNV]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdOpticalFlowExecuteNV(commandBuffer, session, pExecuteInfo);
    }
}




VKAPI_ATTR VkResult VKAPI_CALL GetFramebufferTilePropertiesQCOM(
    VkDevice                                    device,
    VkFramebuffer                               framebuffer,
    uint32_t*                                   pPropertiesCount,
    VkTilePropertiesQCOM*                       pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetFramebufferTilePropertiesQCOM]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetFramebufferTilePropertiesQCOM]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);
    }
    VkResult result = DispatchGetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetFramebufferTilePropertiesQCOM]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetFramebufferTilePropertiesQCOM(device, framebuffer, pPropertiesCount, pProperties, result);
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDynamicRenderingTilePropertiesQCOM(
    VkDevice                                    device,
    const VkRenderingInfo*                      pRenderingInfo,
    VkTilePropertiesQCOM*                       pProperties) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDynamicRenderingTilePropertiesQCOM]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDynamicRenderingTilePropertiesQCOM]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties);
    }
    VkResult result = DispatchGetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDynamicRenderingTilePropertiesQCOM]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetDynamicRenderingTilePropertiesQCOM(device, pRenderingInfo, pProperties, result);
    }
    return result;
}








VKAPI_ATTR VkResult VKAPI_CALL CreateAccelerationStructureKHR(
    VkDevice                                    device,
    const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkAccelerationStructureKHR*                 pAccelerationStructure) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCreateAccelerationStructureKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCreateAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    }
    VkResult result = DispatchCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCreateAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateDestroyAccelerationStructureKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordDestroyAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    }
    DispatchDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordDestroyAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBuildAccelerationStructuresKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBuildAccelerationStructuresKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    }
    DispatchCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBuildAccelerationStructuresKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdBuildAccelerationStructuresIndirectKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdBuildAccelerationStructuresIndirectKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    }
    DispatchCmdBuildAccelerationStructuresIndirectKHR(commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdBuildAccelerationStructuresIndirectKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateBuildAccelerationStructuresKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordBuildAccelerationStructuresKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
    }
    VkResult result = DispatchBuildAccelerationStructuresKHR(device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordBuildAccelerationStructuresKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyAccelerationStructureKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyAccelerationStructureKHR(device, deferredOperation, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyAccelerationStructureToMemoryKHR(device, deferredOperation, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
    }
    VkResult result = DispatchCopyMemoryToAccelerationStructureKHR(device, deferredOperation, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    }
    VkResult result = DispatchWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordWriteAccelerationStructuresPropertiesKHR(device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride, result);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureInfoKHR*   pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyAccelerationStructureKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    }
    DispatchCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyAccelerationStructureKHR(commandBuffer, pInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyAccelerationStructureToMemoryKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyAccelerationStructureToMemoryInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    }
    DispatchCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyAccelerationStructureToMemoryKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyAccelerationStructureToMemoryKHR(commandBuffer, pInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdCopyMemoryToAccelerationStructureKHR(
    VkCommandBuffer                             commandBuffer,
    const VkCopyMemoryToAccelerationStructureInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    }
    DispatchCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdCopyMemoryToAccelerationStructureKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdCopyMemoryToAccelerationStructureKHR(commandBuffer, pInfo);
    }
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL GetAccelerationStructureDeviceAddressKHR(
    VkDevice                                    device,
    const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureDeviceAddressKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetAccelerationStructureDeviceAddressKHR(device, pInfo);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureDeviceAddressKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    }
    VkDeviceAddress result = DispatchGetAccelerationStructureDeviceAddressKHR(device, pInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureDeviceAddressKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
    DispatchCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdWriteAccelerationStructuresPropertiesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdWriteAccelerationStructuresPropertiesKHR(commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }
}

VKAPI_ATTR void VKAPI_CALL GetDeviceAccelerationStructureCompatibilityKHR(
    VkDevice                                    device,
    const VkAccelerationStructureVersionInfoKHR* pVersionInfo,
    VkAccelerationStructureCompatibilityKHR*    pCompatibility) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetDeviceAccelerationStructureCompatibilityKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetDeviceAccelerationStructureCompatibilityKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
    }
    DispatchGetDeviceAccelerationStructureCompatibilityKHR(device, pVersionInfo, pCompatibility);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetDeviceAccelerationStructureCompatibilityKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetAccelerationStructureBuildSizesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetAccelerationStructureBuildSizesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    }
    DispatchGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetAccelerationStructureBuildSizesKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdTraceRaysKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdTraceRaysKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    }
    DispatchCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdTraceRaysKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
        if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    }
    VkResult result = DispatchGetRayTracingCaptureReplayShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingCaptureReplayShaderGroupHandlesKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdTraceRaysIndirectKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdTraceRaysIndirectKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
    }
    DispatchCmdTraceRaysIndirectKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdTraceRaysIndirectKHR]) {
        auto lock = intercept->WriteLock();
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
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateGetRayTracingShaderGroupStackSizeKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
        if (skip) return 0;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordGetRayTracingShaderGroupStackSizeKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    }
    VkDeviceSize result = DispatchGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordGetRayTracingShaderGroupStackSizeKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordGetRayTracingShaderGroupStackSizeKHR(device, pipeline, group, groupShader);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL CmdSetRayTracingPipelineStackSizeKHR(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    pipelineStackSize) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdSetRayTracingPipelineStackSizeKHR]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdSetRayTracingPipelineStackSizeKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
    }
    DispatchCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdSetRayTracingPipelineStackSizeKHR]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdSetRayTracingPipelineStackSizeKHR(commandBuffer, pipelineStackSize);
    }
}



VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
    DispatchCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawMeshTasksEXT(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    uint32_t                                    drawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksIndirectEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksIndirectEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
    }
    DispatchCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksIndirectEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawMeshTasksIndirectEXT(commandBuffer, buffer, offset, drawCount, stride);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdDrawMeshTasksIndirectCountEXT(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride) {
    auto layer_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    bool skip = false;
    for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidateCmdDrawMeshTasksIndirectCountEXT]) {
        auto lock = intercept->ReadLock();
        skip |= intercept->PreCallValidateCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
        if (skip) return;
    }
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecordCmdDrawMeshTasksIndirectCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PreCallRecordCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    DispatchCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecordCmdDrawMeshTasksIndirectCountEXT]) {
        auto lock = intercept->WriteLock();
        intercept->PostCallRecordCmdDrawMeshTasksIndirectCountEXT(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
}

// Map of intercepted ApiName to its associated function data
#ifdef _MSC_VER
#pragma warning( suppress: 6262 ) // VS analysis: this uses more than 16 kiB, which is fine here at global scope
#endif
const vvl::unordered_map<std::string, function_data> name_to_funcptr_map = {
    {"vk_layerGetPhysicalDeviceProcAddr", {kFuncTypeInst, (void*)GetPhysicalDeviceProcAddr}},
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
    {"vkGetPhysicalDeviceToolProperties", {kFuncTypePdev, (void*)GetPhysicalDeviceToolProperties}},
    {"vkCreatePrivateDataSlot", {kFuncTypeDev, (void*)CreatePrivateDataSlot}},
    {"vkDestroyPrivateDataSlot", {kFuncTypeDev, (void*)DestroyPrivateDataSlot}},
    {"vkSetPrivateData", {kFuncTypeDev, (void*)SetPrivateData}},
    {"vkGetPrivateData", {kFuncTypeDev, (void*)GetPrivateData}},
    {"vkCmdSetEvent2", {kFuncTypeDev, (void*)CmdSetEvent2}},
    {"vkCmdResetEvent2", {kFuncTypeDev, (void*)CmdResetEvent2}},
    {"vkCmdWaitEvents2", {kFuncTypeDev, (void*)CmdWaitEvents2}},
    {"vkCmdPipelineBarrier2", {kFuncTypeDev, (void*)CmdPipelineBarrier2}},
    {"vkCmdWriteTimestamp2", {kFuncTypeDev, (void*)CmdWriteTimestamp2}},
    {"vkQueueSubmit2", {kFuncTypeDev, (void*)QueueSubmit2}},
    {"vkCmdCopyBuffer2", {kFuncTypeDev, (void*)CmdCopyBuffer2}},
    {"vkCmdCopyImage2", {kFuncTypeDev, (void*)CmdCopyImage2}},
    {"vkCmdCopyBufferToImage2", {kFuncTypeDev, (void*)CmdCopyBufferToImage2}},
    {"vkCmdCopyImageToBuffer2", {kFuncTypeDev, (void*)CmdCopyImageToBuffer2}},
    {"vkCmdBlitImage2", {kFuncTypeDev, (void*)CmdBlitImage2}},
    {"vkCmdResolveImage2", {kFuncTypeDev, (void*)CmdResolveImage2}},
    {"vkCmdBeginRendering", {kFuncTypeDev, (void*)CmdBeginRendering}},
    {"vkCmdEndRendering", {kFuncTypeDev, (void*)CmdEndRendering}},
    {"vkCmdSetCullMode", {kFuncTypeDev, (void*)CmdSetCullMode}},
    {"vkCmdSetFrontFace", {kFuncTypeDev, (void*)CmdSetFrontFace}},
    {"vkCmdSetPrimitiveTopology", {kFuncTypeDev, (void*)CmdSetPrimitiveTopology}},
    {"vkCmdSetViewportWithCount", {kFuncTypeDev, (void*)CmdSetViewportWithCount}},
    {"vkCmdSetScissorWithCount", {kFuncTypeDev, (void*)CmdSetScissorWithCount}},
    {"vkCmdBindVertexBuffers2", {kFuncTypeDev, (void*)CmdBindVertexBuffers2}},
    {"vkCmdSetDepthTestEnable", {kFuncTypeDev, (void*)CmdSetDepthTestEnable}},
    {"vkCmdSetDepthWriteEnable", {kFuncTypeDev, (void*)CmdSetDepthWriteEnable}},
    {"vkCmdSetDepthCompareOp", {kFuncTypeDev, (void*)CmdSetDepthCompareOp}},
    {"vkCmdSetDepthBoundsTestEnable", {kFuncTypeDev, (void*)CmdSetDepthBoundsTestEnable}},
    {"vkCmdSetStencilTestEnable", {kFuncTypeDev, (void*)CmdSetStencilTestEnable}},
    {"vkCmdSetStencilOp", {kFuncTypeDev, (void*)CmdSetStencilOp}},
    {"vkCmdSetRasterizerDiscardEnable", {kFuncTypeDev, (void*)CmdSetRasterizerDiscardEnable}},
    {"vkCmdSetDepthBiasEnable", {kFuncTypeDev, (void*)CmdSetDepthBiasEnable}},
    {"vkCmdSetPrimitiveRestartEnable", {kFuncTypeDev, (void*)CmdSetPrimitiveRestartEnable}},
    {"vkGetDeviceBufferMemoryRequirements", {kFuncTypeDev, (void*)GetDeviceBufferMemoryRequirements}},
    {"vkGetDeviceImageMemoryRequirements", {kFuncTypeDev, (void*)GetDeviceImageMemoryRequirements}},
    {"vkGetDeviceImageSparseMemoryRequirements", {kFuncTypeDev, (void*)GetDeviceImageSparseMemoryRequirements}},
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
    {"vkGetPhysicalDeviceVideoCapabilitiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceVideoCapabilitiesKHR}},
    {"vkGetPhysicalDeviceVideoFormatPropertiesKHR", {kFuncTypePdev, (void*)GetPhysicalDeviceVideoFormatPropertiesKHR}},
    {"vkCreateVideoSessionKHR", {kFuncTypeDev, (void*)CreateVideoSessionKHR}},
    {"vkDestroyVideoSessionKHR", {kFuncTypeDev, (void*)DestroyVideoSessionKHR}},
    {"vkGetVideoSessionMemoryRequirementsKHR", {kFuncTypeDev, (void*)GetVideoSessionMemoryRequirementsKHR}},
    {"vkBindVideoSessionMemoryKHR", {kFuncTypeDev, (void*)BindVideoSessionMemoryKHR}},
    {"vkCreateVideoSessionParametersKHR", {kFuncTypeDev, (void*)CreateVideoSessionParametersKHR}},
    {"vkUpdateVideoSessionParametersKHR", {kFuncTypeDev, (void*)UpdateVideoSessionParametersKHR}},
    {"vkDestroyVideoSessionParametersKHR", {kFuncTypeDev, (void*)DestroyVideoSessionParametersKHR}},
    {"vkCmdBeginVideoCodingKHR", {kFuncTypeDev, (void*)CmdBeginVideoCodingKHR}},
    {"vkCmdEndVideoCodingKHR", {kFuncTypeDev, (void*)CmdEndVideoCodingKHR}},
    {"vkCmdControlVideoCodingKHR", {kFuncTypeDev, (void*)CmdControlVideoCodingKHR}},
    {"vkCmdDecodeVideoKHR", {kFuncTypeDev, (void*)CmdDecodeVideoKHR}},
    {"vkCmdBeginRenderingKHR", {kFuncTypeDev, (void*)CmdBeginRenderingKHR}},
    {"vkCmdEndRenderingKHR", {kFuncTypeDev, (void*)CmdEndRenderingKHR}},
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
    {"vkWaitForPresentKHR", {kFuncTypeDev, (void*)WaitForPresentKHR}},
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
#ifdef VK_ENABLE_BETA_EXTENSIONS
    {"vkCmdEncodeVideoKHR", {kFuncTypeDev, (void*)CmdEncodeVideoKHR}},
#endif
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
    {"vkCmdTraceRaysIndirect2KHR", {kFuncTypeDev, (void*)CmdTraceRaysIndirect2KHR}},
    {"vkGetDeviceBufferMemoryRequirementsKHR", {kFuncTypeDev, (void*)GetDeviceBufferMemoryRequirementsKHR}},
    {"vkGetDeviceImageMemoryRequirementsKHR", {kFuncTypeDev, (void*)GetDeviceImageMemoryRequirementsKHR}},
    {"vkGetDeviceImageSparseMemoryRequirementsKHR", {kFuncTypeDev, (void*)GetDeviceImageSparseMemoryRequirementsKHR}},
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
    {"vkCreateCuModuleNVX", {kFuncTypeDev, (void*)CreateCuModuleNVX}},
    {"vkCreateCuFunctionNVX", {kFuncTypeDev, (void*)CreateCuFunctionNVX}},
    {"vkDestroyCuModuleNVX", {kFuncTypeDev, (void*)DestroyCuModuleNVX}},
    {"vkDestroyCuFunctionNVX", {kFuncTypeDev, (void*)DestroyCuFunctionNVX}},
    {"vkCmdCuLaunchKernelNVX", {kFuncTypeDev, (void*)CmdCuLaunchKernelNVX}},
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
    {"vkReleaseSwapchainImagesEXT", {kFuncTypeDev, (void*)ReleaseSwapchainImagesEXT}},
    {"vkGetGeneratedCommandsMemoryRequirementsNV", {kFuncTypeDev, (void*)GetGeneratedCommandsMemoryRequirementsNV}},
    {"vkCmdPreprocessGeneratedCommandsNV", {kFuncTypeDev, (void*)CmdPreprocessGeneratedCommandsNV}},
    {"vkCmdExecuteGeneratedCommandsNV", {kFuncTypeDev, (void*)CmdExecuteGeneratedCommandsNV}},
    {"vkCmdBindPipelineShaderGroupNV", {kFuncTypeDev, (void*)CmdBindPipelineShaderGroupNV}},
    {"vkCreateIndirectCommandsLayoutNV", {kFuncTypeDev, (void*)CreateIndirectCommandsLayoutNV}},
    {"vkDestroyIndirectCommandsLayoutNV", {kFuncTypeDev, (void*)DestroyIndirectCommandsLayoutNV}},
    {"vkAcquireDrmDisplayEXT", {kFuncTypePdev, (void*)AcquireDrmDisplayEXT}},
    {"vkGetDrmDisplayEXT", {kFuncTypePdev, (void*)GetDrmDisplayEXT}},
    {"vkCreatePrivateDataSlotEXT", {kFuncTypeDev, (void*)CreatePrivateDataSlotEXT}},
    {"vkDestroyPrivateDataSlotEXT", {kFuncTypeDev, (void*)DestroyPrivateDataSlotEXT}},
    {"vkSetPrivateDataEXT", {kFuncTypeDev, (void*)SetPrivateDataEXT}},
    {"vkGetPrivateDataEXT", {kFuncTypeDev, (void*)GetPrivateDataEXT}},
#ifdef VK_USE_PLATFORM_METAL_EXT
    {"vkExportMetalObjectsEXT", {kFuncTypeDev, (void*)ExportMetalObjectsEXT}},
#endif
    {"vkGetDescriptorSetLayoutSizeEXT", {kFuncTypeDev, (void*)GetDescriptorSetLayoutSizeEXT}},
    {"vkGetDescriptorSetLayoutBindingOffsetEXT", {kFuncTypeDev, (void*)GetDescriptorSetLayoutBindingOffsetEXT}},
    {"vkGetDescriptorEXT", {kFuncTypeDev, (void*)GetDescriptorEXT}},
    {"vkCmdBindDescriptorBuffersEXT", {kFuncTypeDev, (void*)CmdBindDescriptorBuffersEXT}},
    {"vkCmdSetDescriptorBufferOffsetsEXT", {kFuncTypeDev, (void*)CmdSetDescriptorBufferOffsetsEXT}},
    {"vkCmdBindDescriptorBufferEmbeddedSamplersEXT", {kFuncTypeDev, (void*)CmdBindDescriptorBufferEmbeddedSamplersEXT}},
    {"vkGetBufferOpaqueCaptureDescriptorDataEXT", {kFuncTypeDev, (void*)GetBufferOpaqueCaptureDescriptorDataEXT}},
    {"vkGetImageOpaqueCaptureDescriptorDataEXT", {kFuncTypeDev, (void*)GetImageOpaqueCaptureDescriptorDataEXT}},
    {"vkGetImageViewOpaqueCaptureDescriptorDataEXT", {kFuncTypeDev, (void*)GetImageViewOpaqueCaptureDescriptorDataEXT}},
    {"vkGetSamplerOpaqueCaptureDescriptorDataEXT", {kFuncTypeDev, (void*)GetSamplerOpaqueCaptureDescriptorDataEXT}},
    {"vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT", {kFuncTypeDev, (void*)GetAccelerationStructureOpaqueCaptureDescriptorDataEXT}},
    {"vkCmdSetFragmentShadingRateEnumNV", {kFuncTypeDev, (void*)CmdSetFragmentShadingRateEnumNV}},
    {"vkGetImageSubresourceLayout2EXT", {kFuncTypeDev, (void*)GetImageSubresourceLayout2EXT}},
    {"vkGetDeviceFaultInfoEXT", {kFuncTypeDev, (void*)GetDeviceFaultInfoEXT}},
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
    {"vkCmdSetVertexInputEXT", {kFuncTypeDev, (void*)CmdSetVertexInputEXT}},
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetMemoryZirconHandleFUCHSIA", {kFuncTypeDev, (void*)GetMemoryZirconHandleFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetMemoryZirconHandlePropertiesFUCHSIA", {kFuncTypeDev, (void*)GetMemoryZirconHandlePropertiesFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkImportSemaphoreZirconHandleFUCHSIA", {kFuncTypeDev, (void*)ImportSemaphoreZirconHandleFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetSemaphoreZirconHandleFUCHSIA", {kFuncTypeDev, (void*)GetSemaphoreZirconHandleFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkCreateBufferCollectionFUCHSIA", {kFuncTypeDev, (void*)CreateBufferCollectionFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkSetBufferCollectionImageConstraintsFUCHSIA", {kFuncTypeDev, (void*)SetBufferCollectionImageConstraintsFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkSetBufferCollectionBufferConstraintsFUCHSIA", {kFuncTypeDev, (void*)SetBufferCollectionBufferConstraintsFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkDestroyBufferCollectionFUCHSIA", {kFuncTypeDev, (void*)DestroyBufferCollectionFUCHSIA}},
#endif
#ifdef VK_USE_PLATFORM_FUCHSIA
    {"vkGetBufferCollectionPropertiesFUCHSIA", {kFuncTypeDev, (void*)GetBufferCollectionPropertiesFUCHSIA}},
#endif
    {"vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI", {kFuncTypeDev, (void*)GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI}},
    {"vkCmdSubpassShadingHUAWEI", {kFuncTypeDev, (void*)CmdSubpassShadingHUAWEI}},
    {"vkCmdBindInvocationMaskHUAWEI", {kFuncTypeDev, (void*)CmdBindInvocationMaskHUAWEI}},
    {"vkGetMemoryRemoteAddressNV", {kFuncTypeDev, (void*)GetMemoryRemoteAddressNV}},
    {"vkGetPipelinePropertiesEXT", {kFuncTypeDev, (void*)GetPipelinePropertiesEXT}},
    {"vkCmdSetPatchControlPointsEXT", {kFuncTypeDev, (void*)CmdSetPatchControlPointsEXT}},
    {"vkCmdSetRasterizerDiscardEnableEXT", {kFuncTypeDev, (void*)CmdSetRasterizerDiscardEnableEXT}},
    {"vkCmdSetDepthBiasEnableEXT", {kFuncTypeDev, (void*)CmdSetDepthBiasEnableEXT}},
    {"vkCmdSetLogicOpEXT", {kFuncTypeDev, (void*)CmdSetLogicOpEXT}},
    {"vkCmdSetPrimitiveRestartEnableEXT", {kFuncTypeDev, (void*)CmdSetPrimitiveRestartEnableEXT}},
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    {"vkCreateScreenSurfaceQNX", {kFuncTypeInst, (void*)CreateScreenSurfaceQNX}},
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
    {"vkGetPhysicalDeviceScreenPresentationSupportQNX", {kFuncTypePdev, (void*)GetPhysicalDeviceScreenPresentationSupportQNX}},
#endif
    {"vkCmdSetColorWriteEnableEXT", {kFuncTypeDev, (void*)CmdSetColorWriteEnableEXT}},
    {"vkCmdDrawMultiEXT", {kFuncTypeDev, (void*)CmdDrawMultiEXT}},
    {"vkCmdDrawMultiIndexedEXT", {kFuncTypeDev, (void*)CmdDrawMultiIndexedEXT}},
    {"vkCreateMicromapEXT", {kFuncTypeDev, (void*)CreateMicromapEXT}},
    {"vkDestroyMicromapEXT", {kFuncTypeDev, (void*)DestroyMicromapEXT}},
    {"vkCmdBuildMicromapsEXT", {kFuncTypeDev, (void*)CmdBuildMicromapsEXT}},
    {"vkBuildMicromapsEXT", {kFuncTypeDev, (void*)BuildMicromapsEXT}},
    {"vkCopyMicromapEXT", {kFuncTypeDev, (void*)CopyMicromapEXT}},
    {"vkCopyMicromapToMemoryEXT", {kFuncTypeDev, (void*)CopyMicromapToMemoryEXT}},
    {"vkCopyMemoryToMicromapEXT", {kFuncTypeDev, (void*)CopyMemoryToMicromapEXT}},
    {"vkWriteMicromapsPropertiesEXT", {kFuncTypeDev, (void*)WriteMicromapsPropertiesEXT}},
    {"vkCmdCopyMicromapEXT", {kFuncTypeDev, (void*)CmdCopyMicromapEXT}},
    {"vkCmdCopyMicromapToMemoryEXT", {kFuncTypeDev, (void*)CmdCopyMicromapToMemoryEXT}},
    {"vkCmdCopyMemoryToMicromapEXT", {kFuncTypeDev, (void*)CmdCopyMemoryToMicromapEXT}},
    {"vkCmdWriteMicromapsPropertiesEXT", {kFuncTypeDev, (void*)CmdWriteMicromapsPropertiesEXT}},
    {"vkGetDeviceMicromapCompatibilityEXT", {kFuncTypeDev, (void*)GetDeviceMicromapCompatibilityEXT}},
    {"vkGetMicromapBuildSizesEXT", {kFuncTypeDev, (void*)GetMicromapBuildSizesEXT}},
    {"vkCmdDrawClusterHUAWEI", {kFuncTypeDev, (void*)CmdDrawClusterHUAWEI}},
    {"vkCmdDrawClusterIndirectHUAWEI", {kFuncTypeDev, (void*)CmdDrawClusterIndirectHUAWEI}},
    {"vkSetDeviceMemoryPriorityEXT", {kFuncTypeDev, (void*)SetDeviceMemoryPriorityEXT}},
    {"vkGetDescriptorSetLayoutHostMappingInfoVALVE", {kFuncTypeDev, (void*)GetDescriptorSetLayoutHostMappingInfoVALVE}},
    {"vkGetDescriptorSetHostMappingVALVE", {kFuncTypeDev, (void*)GetDescriptorSetHostMappingVALVE}},
    {"vkCmdCopyMemoryIndirectNV", {kFuncTypeDev, (void*)CmdCopyMemoryIndirectNV}},
    {"vkCmdCopyMemoryToImageIndirectNV", {kFuncTypeDev, (void*)CmdCopyMemoryToImageIndirectNV}},
    {"vkCmdDecompressMemoryNV", {kFuncTypeDev, (void*)CmdDecompressMemoryNV}},
    {"vkCmdDecompressMemoryIndirectCountNV", {kFuncTypeDev, (void*)CmdDecompressMemoryIndirectCountNV}},
    {"vkCmdSetTessellationDomainOriginEXT", {kFuncTypeDev, (void*)CmdSetTessellationDomainOriginEXT}},
    {"vkCmdSetDepthClampEnableEXT", {kFuncTypeDev, (void*)CmdSetDepthClampEnableEXT}},
    {"vkCmdSetPolygonModeEXT", {kFuncTypeDev, (void*)CmdSetPolygonModeEXT}},
    {"vkCmdSetRasterizationSamplesEXT", {kFuncTypeDev, (void*)CmdSetRasterizationSamplesEXT}},
    {"vkCmdSetSampleMaskEXT", {kFuncTypeDev, (void*)CmdSetSampleMaskEXT}},
    {"vkCmdSetAlphaToCoverageEnableEXT", {kFuncTypeDev, (void*)CmdSetAlphaToCoverageEnableEXT}},
    {"vkCmdSetAlphaToOneEnableEXT", {kFuncTypeDev, (void*)CmdSetAlphaToOneEnableEXT}},
    {"vkCmdSetLogicOpEnableEXT", {kFuncTypeDev, (void*)CmdSetLogicOpEnableEXT}},
    {"vkCmdSetColorBlendEnableEXT", {kFuncTypeDev, (void*)CmdSetColorBlendEnableEXT}},
    {"vkCmdSetColorBlendEquationEXT", {kFuncTypeDev, (void*)CmdSetColorBlendEquationEXT}},
    {"vkCmdSetColorWriteMaskEXT", {kFuncTypeDev, (void*)CmdSetColorWriteMaskEXT}},
    {"vkCmdSetRasterizationStreamEXT", {kFuncTypeDev, (void*)CmdSetRasterizationStreamEXT}},
    {"vkCmdSetConservativeRasterizationModeEXT", {kFuncTypeDev, (void*)CmdSetConservativeRasterizationModeEXT}},
    {"vkCmdSetExtraPrimitiveOverestimationSizeEXT", {kFuncTypeDev, (void*)CmdSetExtraPrimitiveOverestimationSizeEXT}},
    {"vkCmdSetDepthClipEnableEXT", {kFuncTypeDev, (void*)CmdSetDepthClipEnableEXT}},
    {"vkCmdSetSampleLocationsEnableEXT", {kFuncTypeDev, (void*)CmdSetSampleLocationsEnableEXT}},
    {"vkCmdSetColorBlendAdvancedEXT", {kFuncTypeDev, (void*)CmdSetColorBlendAdvancedEXT}},
    {"vkCmdSetProvokingVertexModeEXT", {kFuncTypeDev, (void*)CmdSetProvokingVertexModeEXT}},
    {"vkCmdSetLineRasterizationModeEXT", {kFuncTypeDev, (void*)CmdSetLineRasterizationModeEXT}},
    {"vkCmdSetLineStippleEnableEXT", {kFuncTypeDev, (void*)CmdSetLineStippleEnableEXT}},
    {"vkCmdSetDepthClipNegativeOneToOneEXT", {kFuncTypeDev, (void*)CmdSetDepthClipNegativeOneToOneEXT}},
    {"vkCmdSetViewportWScalingEnableNV", {kFuncTypeDev, (void*)CmdSetViewportWScalingEnableNV}},
    {"vkCmdSetViewportSwizzleNV", {kFuncTypeDev, (void*)CmdSetViewportSwizzleNV}},
    {"vkCmdSetCoverageToColorEnableNV", {kFuncTypeDev, (void*)CmdSetCoverageToColorEnableNV}},
    {"vkCmdSetCoverageToColorLocationNV", {kFuncTypeDev, (void*)CmdSetCoverageToColorLocationNV}},
    {"vkCmdSetCoverageModulationModeNV", {kFuncTypeDev, (void*)CmdSetCoverageModulationModeNV}},
    {"vkCmdSetCoverageModulationTableEnableNV", {kFuncTypeDev, (void*)CmdSetCoverageModulationTableEnableNV}},
    {"vkCmdSetCoverageModulationTableNV", {kFuncTypeDev, (void*)CmdSetCoverageModulationTableNV}},
    {"vkCmdSetShadingRateImageEnableNV", {kFuncTypeDev, (void*)CmdSetShadingRateImageEnableNV}},
    {"vkCmdSetRepresentativeFragmentTestEnableNV", {kFuncTypeDev, (void*)CmdSetRepresentativeFragmentTestEnableNV}},
    {"vkCmdSetCoverageReductionModeNV", {kFuncTypeDev, (void*)CmdSetCoverageReductionModeNV}},
    {"vkGetShaderModuleIdentifierEXT", {kFuncTypeDev, (void*)GetShaderModuleIdentifierEXT}},
    {"vkGetShaderModuleCreateInfoIdentifierEXT", {kFuncTypeDev, (void*)GetShaderModuleCreateInfoIdentifierEXT}},
    {"vkGetPhysicalDeviceOpticalFlowImageFormatsNV", {kFuncTypePdev, (void*)GetPhysicalDeviceOpticalFlowImageFormatsNV}},
    {"vkCreateOpticalFlowSessionNV", {kFuncTypeDev, (void*)CreateOpticalFlowSessionNV}},
    {"vkDestroyOpticalFlowSessionNV", {kFuncTypeDev, (void*)DestroyOpticalFlowSessionNV}},
    {"vkBindOpticalFlowSessionImageNV", {kFuncTypeDev, (void*)BindOpticalFlowSessionImageNV}},
    {"vkCmdOpticalFlowExecuteNV", {kFuncTypeDev, (void*)CmdOpticalFlowExecuteNV}},
    {"vkGetFramebufferTilePropertiesQCOM", {kFuncTypeDev, (void*)GetFramebufferTilePropertiesQCOM}},
    {"vkGetDynamicRenderingTilePropertiesQCOM", {kFuncTypeDev, (void*)GetDynamicRenderingTilePropertiesQCOM}},
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
    {"vkCmdDrawMeshTasksEXT", {kFuncTypeDev, (void*)CmdDrawMeshTasksEXT}},
    {"vkCmdDrawMeshTasksIndirectEXT", {kFuncTypeDev, (void*)CmdDrawMeshTasksIndirectEXT}},
    {"vkCmdDrawMeshTasksIndirectCountEXT", {kFuncTypeDev, (void*)CmdDrawMeshTasksIndirectCountEXT}},
};


} // namespace vulkan_layer_chassis


VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    return vulkan_layer_chassis::GetPhysicalDeviceProcAddr(instance, funcName);
}

#if defined(__GNUC__) && __GNUC__ >= 4
#define VVL_EXPORT __attribute__((visibility("default")))
#else
#define VVL_EXPORT
#endif

// The following functions need to match the `/DEF` and `--version-script` files
// for consistency across platforms that don't accept those linker options.
extern "C" {

VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return vulkan_layer_chassis::GetInstanceProcAddr(instance, funcName);
}

VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return vulkan_layer_chassis::GetDeviceProcAddr(dev, funcName);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return vulkan_layer_chassis::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount, VkExtensionProperties *pProperties) {
    return vulkan_layer_chassis::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct) {
    assert(pVersionStruct != nullptr);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = vkGetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = vk_layerGetPhysicalDeviceProcAddr;
    }

    return VK_SUCCESS;
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount, VkLayerProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_layer_chassis::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pCount, VkExtensionProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return vulkan_layer_chassis::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}
#endif

}  // extern "C"

