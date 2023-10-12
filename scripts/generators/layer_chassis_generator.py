#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# This script generates the dispatch portion of a factory layer which intercepts
# all Vulkan  functions. The resultant factory layer allows rapid development of
# layers and interceptors.

import os
from generators.vulkan_object import Command
from generators.base_generator import BaseGenerator

# This class is a container for any source code, data, or other behavior that is necessary to
# customize the generator script for a specific target API variant (e.g. Vulkan SC). As such,
# all of these API-specific interfaces and their use in the generator script are part of the
# contract between this repository and its downstream users. Changing or removing any of these
# interfaces or their use in the generator script will have downstream effects and thus
# should be avoided unless absolutely necessary.
class APISpecific:
    # Returns the list of validation layers for the target API
    @staticmethod
    def getValidationLayerList(targetApiName: str) -> list[dict[str, str]]:
        match targetApiName:

            # Vulkan specific validation layer list
            case 'vulkan':
                return [
                    {
                        'include': 'thread_tracker/thread_safety_validation.h',
                        'class': 'ThreadSafety',
                        'enabled': '!disables[thread_safety]'
                    },
                    {
                        'include': 'stateless/stateless_validation.h',
                        'class': 'StatelessValidation',
                        'enabled': '!disables[stateless_checks]'
                    },
                    {
                        'include': 'object_tracker/object_lifetime_validation.h',
                        'class': 'ObjectLifetimes',
                        'enabled': '!disables[object_tracking]'
                    },
                    {
                        'include': 'core_checks/core_validation.h',
                        'class': 'CoreChecks',
                        'enabled': '!disables[core_checks]'
                    },
                    {
                        'include': 'best_practices/best_practices_validation.h',
                        'class': 'BestPractices',
                        'enabled': 'enables[best_practices]'
                    },
                    {
                        'include': 'gpu_validation/gpu_validation.h',
                        'class': 'GpuAssisted',
                        'enabled': 'enables[gpu_validation]'
                    },
                    {
                        'include': 'gpu_validation/debug_printf.h',
                        'class': 'DebugPrintf',
                        'enabled': 'enables[debug_printf]'
                    },
                    {
                        'include': 'sync/sync_validation.h',
                        'class': 'SyncValidator',
                        'enabled': 'enables[sync_validation]'
                    }
                ]


    # Returns the list of instance extensions exposed by the validation layers
    @staticmethod
    def getInstanceExtensionList(targetApiName: str) -> list[str]:
        match targetApiName:

            # Vulkan specific instance extension list
            case 'vulkan':
                return [
                    'VK_EXT_debug_report',
                    'VK_EXT_debug_utils',
                    'VK_EXT_validation_features'
                ]


    # Returns the list of device extensions exposed by the validation layers
    @staticmethod
    def getDeviceExtensionList(targetApiName: str) -> list[str]:
        match targetApiName:

            # Vulkan specific device extension list
            case 'vulkan':
                return [
                'VK_EXT_validation_cache',
                'VK_EXT_debug_marker',
                'VK_EXT_tooling_info'
            ]


    # Generates source code for InitObjectDispatchVector
    @staticmethod
    def genInitObjectDispatchVectorSource(targetApiName: str) -> str:
        match targetApiName:

            # Vulkan specific InitObjectDispatchVector
            case 'vulkan':
                return '''
// clang-format off
void ValidationObject::InitObjectDispatchVectors() {

#define BUILD_DISPATCH_VECTOR(name) \\
    init_object_dispatch_vector(InterceptId ## name, \\
                                typeid(&ValidationObject::name), \\
                                typeid(&ThreadSafety::name), \\
                                typeid(&StatelessValidation::name), \\
                                typeid(&ObjectLifetimes::name), \\
                                typeid(&CoreChecks::name), \\
                                typeid(&BestPractices::name), \\
                                typeid(&GpuAssisted::name), \\
                                typeid(&DebugPrintf::name), \\
                                typeid(&SyncValidator::name));

    auto init_object_dispatch_vector = [this](InterceptId id,
                                              const std::type_info& vo_typeid,
                                              const std::type_info& tt_typeid,
                                              const std::type_info& tpv_typeid,
                                              const std::type_info& tot_typeid,
                                              const std::type_info& tcv_typeid,
                                              const std::type_info& tbp_typeid,
                                              const std::type_info& tga_typeid,
                                              const std::type_info& tdp_typeid,
                                              const std::type_info& tsv_typeid) {
        for (auto item : this->object_dispatch) {
            auto intercept_vector = &this->intercept_vectors[id];
            switch (item->container_type) {
            case LayerObjectTypeThreading:
                if (tt_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeParameterValidation:
                if (tpv_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeObjectTracker:
                if (tot_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeCoreValidation:
                if (tcv_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeBestPractices:
                if (tbp_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeGpuAssisted:
                if (tga_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeDebugPrintf:
                if (tdp_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeSyncValidation:
                if (tsv_typeid != vo_typeid) intercept_vector->push_back(item);
                break;
            case LayerObjectTypeInstance:
            case LayerObjectTypeDevice:
                break;
            default:
                /* Chassis codegen needs to be updated for unknown validation object type */
                assert(0);
            }
        }
    };
// clang-format on

    intercept_vectors.resize(InterceptIdCount);
'''


# Generates a LayerFactory layer that intercepts all API entrypoints
#  This is intended to be used as a starting point for creating custom layers
class LayerChassisOutputGenerator(BaseGenerator):
    ignore_functions = [
        'vkEnumerateInstanceVersion',
    ]

    manual_functions = [
        # Include functions here to be interecpted w/ manually implemented function bodies
        'vkGetDeviceProcAddr',
        'vkGetInstanceProcAddr',
        'vkCreateDevice',
        'vkDestroyDevice',
        'vkCreateInstance',
        'vkDestroyInstance',
        'vkEnumerateInstanceLayerProperties',
        'vkEnumerateInstanceExtensionProperties',
        'vkEnumerateDeviceLayerProperties',
        'vkEnumerateDeviceExtensionProperties',
        # Functions that are handled explicitly due to chassis architecture violations
        'vkCreateGraphicsPipelines',
        'vkCreateComputePipelines',
        'vkCreateRayTracingPipelinesNV',
        'vkCreateRayTracingPipelinesKHR',
        'vkCreatePipelineLayout',
        'vkCreateShaderModule',
        'vkCreateShadersEXT',
        'vkAllocateDescriptorSets',
        'vkCreateBuffer',
        # ValidationCache functions do not get dispatched
        'vkCreateValidationCacheEXT',
        'vkDestroyValidationCacheEXT',
        'vkMergeValidationCachesEXT',
        'vkGetValidationCacheDataEXT',
        'vkGetPhysicalDeviceToolPropertiesEXT',
    ]

    def __init__(self):
        BaseGenerator.__init__(self)

    def getApiFunctionType(self, command: Command) -> str:
            if command.name in [
                    'vkCreateInstance',
                    'vkEnumerateInstanceVersion',
                    'vkEnumerateInstanceLayerProperties',
                    'vkEnumerateInstanceExtensionProperties',
                ]:
                return 'kFuncTypeInst'
            elif command.params[0].type == 'VkInstance':
                return'kFuncTypeInst'
            elif command.params[0].type == 'VkPhysicalDevice':
                return'kFuncTypePdev'
            else:
                return'kFuncTypeDev'

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2023 The Khronos Group Inc.
            * Copyright (c) 2015-2023 Valve Corporation
            * Copyright (c) 2015-2023 LunarG, Inc.
            * Copyright (c) 2015-2023 Google Inc.
            * Copyright (c) 2023-2023 RasterGrid Kft.
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
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'chassis.h':
            self.generateHeader()
        elif self.filename == 'chassis.cpp':
            self.generateSource()
        elif self.filename == 'chassis_dispatch_helper.h':
            self.generateHelper()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include <atomic>
            #include <mutex>
            #include <cinttypes>
            #include <stdio.h>
            #include <stdlib.h>
            #include <string.h>
            #include <algorithm>
            #include <memory>
            #include <string_view>

            #include <vulkan/vulkan.h>
            #include <vulkan/vk_layer.h>
            #include <vulkan/vk_enum_string_helper.h>
            #include <vulkan/utility/vk_struct_helper.hpp>
            #include "utils/cast_utils.h"
            #include "vk_layer_config.h"
            #include "containers/custom_containers.h"
            #include "error_message/logging.h"
            #include "error_message/error_location.h"
            #include "error_message/record_object.h"
            #include "vk_object_types.h"
            #include "utils/vk_layer_extension_utils.h"
            #include "utils/vk_layer_utils.h"
            #include "vk_dispatch_table_helper.h"
            #include "vk_extension_helper.h"
            #include "vk_safe_struct.h"
            #include "gpu_validation/gpu_settings.h"

            extern std::atomic<uint64_t> global_unique_id;

            // To avoid re-hashing unique ids on each use, we precompute the hash and store the
            // hash's LSBs in the high 24 bits.
            struct HashedUint64 {
                static const int HASHED_UINT64_SHIFT = 40;
                size_t operator()(const uint64_t& t) const { return t >> HASHED_UINT64_SHIFT; }

                static uint64_t hash(uint64_t id) {
                    uint64_t h = (uint64_t)vvl::hash<uint64_t>()(id);
                    id |= h << HASHED_UINT64_SHIFT;
                    return id;
                }
            };

            extern vl_concurrent_unordered_map<uint64_t, uint64_t, 4, HashedUint64> unique_id_mapping;

            VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char* funcName);\n
            ''')

        for command in [x for x in self.vk.commands.values() if x.name not in self.ignore_functions]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            out.append(f'{command.cPrototype.replace("VKAPI_CALL vk", "VKAPI_CALL ")}\n\n')
            out.extend(['#endif\n'] if command.protect else [])

        out.append('''
            // Layer object type identifiers
            enum LayerObjectTypeId {
                LayerObjectTypeInstance,             // Container for an instance dispatch object
                LayerObjectTypeDevice,               // Container for a device dispatch object
                LayerObjectTypeThreading,            // Instance or device threading layer object
                LayerObjectTypeParameterValidation,  // Instance or device parameter validation layer object
                LayerObjectTypeObjectTracker,        // Instance or device object tracker layer object
                LayerObjectTypeCoreValidation,       // Instance or device core validation layer object
                LayerObjectTypeBestPractices,        // Instance or device best practices layer object
                LayerObjectTypeGpuAssisted,          // Instance or device gpu assisted validation layer object
                LayerObjectTypeDebugPrintf,          // Instance or device shader debug printf layer object
                LayerObjectTypeSyncValidation,       // Instance or device synchronization validation layer object
                LayerObjectTypeMaxEnum,              // Max enum count
            };

            struct TEMPLATE_STATE {
                VkDescriptorUpdateTemplate desc_update_template;
                safe_VkDescriptorUpdateTemplateCreateInfo create_info;
                bool destroyed;

                TEMPLATE_STATE(VkDescriptorUpdateTemplate update_template, safe_VkDescriptorUpdateTemplateCreateInfo* pCreateInfo)
                    : desc_update_template(update_template), create_info(*pCreateInfo), destroyed(false) {}
            };

            class LAYER_PHYS_DEV_PROPERTIES {
            public:
                VkPhysicalDeviceProperties properties;
                std::vector<VkQueueFamilyProperties> queue_family_properties;
            };

            typedef enum ValidationCheckDisables {
                VALIDATION_CHECK_DISABLE_COMMAND_BUFFER_STATE,
                VALIDATION_CHECK_DISABLE_OBJECT_IN_USE,
                VALIDATION_CHECK_DISABLE_QUERY_VALIDATION,
                VALIDATION_CHECK_DISABLE_IMAGE_LAYOUT_VALIDATION,
            } ValidationCheckDisables;

            typedef enum ValidationCheckEnables {
                VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM,
                VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_AMD,
                VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_IMG,
                VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_NVIDIA,
                VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL,
                VALIDATION_CHECK_ENABLE_SYNCHRONIZATION_VALIDATION_QUEUE_SUBMIT,
            } ValidationCheckEnables;

            typedef enum VkValidationFeatureEnable {
                VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION,
            } VkValidationFeatureEnable;

            // CHECK_DISABLED and CHECK_ENABLED vectors are containers for bools that can opt in or out of specific classes of validation
            // checks. Enum values can be specified via the vk_layer_settings.txt config file or at CreateInstance time via the
            // VK_EXT_validation_features extension that can selectively disable or enable checks.
            typedef enum DisableFlags {
                command_buffer_state,
                object_in_use,
                query_validation,
                image_layout_validation,
                object_tracking,
                core_checks,
                thread_safety,
                stateless_checks,
                handle_wrapping,
                shader_validation,
                shader_validation_caching,
                // Insert new disables above this line
                kMaxDisableFlags,
            } DisableFlags;

            typedef enum EnableFlags {
                gpu_validation,
                gpu_validation_reserve_binding_slot,
                best_practices,
                vendor_specific_arm,
                vendor_specific_amd,
                vendor_specific_img,
                vendor_specific_nvidia,
                debug_printf,
                sync_validation,
                sync_validation_queue_submit,
                // Insert new enables above this line
                kMaxEnableFlags,
            } EnableFlags;

            typedef std::array<bool, kMaxDisableFlags> CHECK_DISABLED;
            typedef std::array<bool, kMaxEnableFlags> CHECK_ENABLED;

            #if defined(__clang__)
            #define DECORATE_PRINTF(_fmt_argnum, _first_param_num) __attribute__((format(printf, _fmt_argnum, _first_param_num)))
            #elif defined(__GNUC__)
            #define DECORATE_PRINTF(_fmt_argnum, _first_param_num) __attribute__((format(gnu_printf, _fmt_argnum, _first_param_num)))
            #else
            #define DECORATE_PRINTF(_fmt_num, _first_param_num)
            #endif
            // Layer chassis validation object base class definition
            class ValidationObject {
            public:
                APIVersion api_version;
                debug_report_data* report_data = nullptr;
                template <typename T>
                std::string FormatHandle(T&& h) const {
                    return report_data->FormatHandle(std::forward<T>(h));
                }

                std::vector<std::vector<ValidationObject*>> intercept_vectors;

                VkLayerInstanceDispatchTable instance_dispatch_table;
                VkLayerDispatchTable device_dispatch_table;

                InstanceExtensions instance_extensions;
                DeviceExtensions device_extensions = {};
                CHECK_DISABLED disabled = {};
                CHECK_ENABLED enabled = {};
                bool fine_grained_locking{true};
                GpuAVSettings gpuav_settings = {};

                VkInstance instance = VK_NULL_HANDLE;
                VkPhysicalDevice physical_device = VK_NULL_HANDLE;
                VkDevice device = VK_NULL_HANDLE;
                LAYER_PHYS_DEV_PROPERTIES phys_dev_properties = {};

                std::vector<ValidationObject*> object_dispatch;
                LayerObjectTypeId container_type;

                vl_concurrent_unordered_map<VkDeferredOperationKHR, std::vector<std::function<void()>>, 0> deferred_operation_post_completion;
                vl_concurrent_unordered_map<VkDeferredOperationKHR, std::vector<std::function<void(const std::vector<VkPipeline>&)>>, 0>
                    deferred_operation_post_check;
                vl_concurrent_unordered_map<VkDeferredOperationKHR, std::vector<VkPipeline>, 0> deferred_operation_pipelines;

                std::string layer_name = "CHASSIS";

                // Constructor
                ValidationObject(){};
                // Destructor
                virtual ~ValidationObject(){};

                void InitObjectDispatchVectors();

                mutable std::shared_mutex validation_object_mutex;
                virtual ReadLockGuard ReadLock() const { return ReadLockGuard(validation_object_mutex); }
                virtual WriteLockGuard WriteLock() { return WriteLockGuard(validation_object_mutex); }

                // If the Record phase calls a function that blocks, we might need to release
                // the lock that protects Record itself in order to avoid mutual waiting.
                static thread_local WriteLockGuard* record_guard;

                // Should be used instead of WriteLock() if the Record phase wants to release
                // its lock during the blocking operation.
                struct BlockingOperationGuard {
                    WriteLockGuard lock;
                    ValidationObject* validation_object = nullptr;

                    BlockingOperationGuard(ValidationObject* validation_object) : validation_object(validation_object) {
                        // This assert detects recursive calls. It is here mostly for documentation purposes
                        // because WriteLock() also triggers errors during recursion.
                        // Recursion is not allowed since record_guard is a thread-local variable and it can
                        // reference only one frame of the callstack.
                        assert(validation_object->record_guard == nullptr);

                        lock = validation_object->WriteLock();

                        // Initialize record_guard only when Record is actually protected by the
                        // mutex. It's not the case when fine grained locking is enabled.
                        record_guard = lock.owns_lock() ? &lock : nullptr;
                    }

                    ~BlockingOperationGuard() { validation_object->record_guard = nullptr; }
                };

                // The following Begin/End methods should be called during the Record phase
                // around blocking operation that causes mutual waiting (deadlock).
                void BeginBlockingOperation() {
                    if (record_guard) {
                        record_guard->unlock();
                    }
                }
                void EndBlockingOperation() {
                    if (record_guard) {
                        record_guard->lock();
                    }
                }

                ValidationObject* GetValidationObject(LayerObjectTypeId object_type) const {
                    for (auto validation_object : object_dispatch) {
                        if (validation_object->container_type == object_type) {
                            return validation_object;
                        }
                    }
                    return nullptr;
                }

                template <typename ValidationObjectType>
                ValidationObjectType* GetValidationObject() const;

                // Debug Logging Helpers
                // deprecated LogError - moving to use one with Location
                bool DECORATE_PRINTF(4, 5) LogError(const LogObjectList& objlist, std::string_view vuid_text, const char* format, ...) const {
                    va_list argptr;
                    va_start(argptr, format);
                    const bool result = LogMsg(report_data, kErrorBit, objlist, nullptr, vuid_text, format, argptr);
                    va_end(argptr);
                    return result;
                }

                bool DECORATE_PRINTF(5, 6)
                    LogError(std::string_view vuid_text, const LogObjectList& objlist, const Location& loc, const char* format, ...) const {
                    va_list argptr;
                    va_start(argptr, format);
                    const bool result = LogMsg(report_data, kErrorBit, objlist, &loc, vuid_text, format, argptr);
                    va_end(argptr);
                    return result;
                }

                // Currently works like LogWarning, but allows developer to better categorize the warning
                bool DECORATE_PRINTF(5, 6) LogUndefinedValue(std::string_view vuid_text, const LogObjectList& objlist, const Location& loc, const char* format, ...) const {
                    va_list argptr;
                    va_start(argptr, format);
                    const bool result = LogMsg(report_data, kWarningBit, objlist, &loc, vuid_text, format, argptr);
                    va_end(argptr);
                    return result;
                }

                bool DECORATE_PRINTF(5, 6) LogWarning(std::string_view vuid_text, const LogObjectList& objlist, const Location& loc, const char* format, ...) const {
                    va_list argptr;
                    va_start(argptr, format);
                    const bool result = LogMsg(report_data, kWarningBit, objlist, &loc, vuid_text, format, argptr);
                    va_end(argptr);
                    return result;
                }

                bool DECORATE_PRINTF(5, 6) LogPerformanceWarning(std::string_view vuid_text, const LogObjectList& objlist, const Location& loc, const char* format, ...) const {
                    va_list argptr;
                    va_start(argptr, format);
                    const bool result = LogMsg(report_data, kPerformanceWarningBit, objlist, &loc, vuid_text, format, argptr);
                    va_end(argptr);
                    return result;
                }

                bool DECORATE_PRINTF(5, 6) LogInfo(std::string_view vuid_text, const LogObjectList& objlist, const Location& loc, const char* format, ...) const {
                    va_list argptr;
                    va_start(argptr, format);
                    const bool result = LogMsg(report_data, kInformationBit, objlist, &loc, vuid_text, format, argptr);
                    va_end(argptr);
                    return result;
                }

                bool DECORATE_PRINTF(5, 6) LogVerbose(std::string_view vuid_text, const LogObjectList& objlist, const Location& loc, const char* format, ...) const {
                    va_list argptr;
                    va_start(argptr, format);
                    const bool result = LogMsg(report_data, kVerboseBit, objlist, &loc, vuid_text, format, argptr);
                    va_end(argptr);
                    return result;
                }

                void LogInternalError(std::string_view failure_location, const LogObjectList& obj_list, std::string_view entrypoint,
                                    VkResult err) const {
                    const std::string_view err_string = string_VkResult(err);
                    std::string vuid = "INTERNAL-ERROR-";
                    vuid += entrypoint;
                    LogError(obj_list, vuid, "In %s: %s() was called in the Validation Layer state tracking and failed with result = %s.",
                            failure_location.data(), entrypoint.data(), err_string.data());
                }

                // Handle Wrapping Data
                // Reverse map display handles
                vl_concurrent_unordered_map<VkDisplayKHR, uint64_t, 0> display_id_reverse_mapping;
                // Wrapping Descriptor Template Update structures requires access to the template createinfo structs
                vvl::unordered_map<uint64_t, std::unique_ptr<TEMPLATE_STATE>> desc_template_createinfo_map;
                struct SubpassesUsageStates {
                    vvl::unordered_set<uint32_t> subpasses_using_color_attachment;
                    vvl::unordered_set<uint32_t> subpasses_using_depthstencil_attachment;
                };
                // Uses unwrapped handles
                vvl::unordered_map<VkRenderPass, SubpassesUsageStates> renderpasses_states;
                // Map of wrapped swapchain handles to arrays of wrapped swapchain image IDs
                // Each swapchain has an immutable list of wrapped swapchain image IDs -- always return these IDs if they exist
                vvl::unordered_map<VkSwapchainKHR, std::vector<VkImage>> swapchain_wrapped_image_handle_map;
                // Map of wrapped descriptor pools to set of wrapped descriptor sets allocated from each pool
                vvl::unordered_map<VkDescriptorPool, vvl::unordered_set<VkDescriptorSet>> pool_descriptor_sets_map;

                // Unwrap a handle.
                template <typename HandleType>
                HandleType Unwrap(HandleType wrappedHandle) {
                    if (wrappedHandle == (HandleType)VK_NULL_HANDLE) return wrappedHandle;
                    auto iter = unique_id_mapping.find(CastToUint64(wrappedHandle));
                    if (iter == unique_id_mapping.end()) return (HandleType)0;
                    return (HandleType)iter->second;
                }

                // Wrap a newly created handle with a new unique ID, and return the new ID.
                template <typename HandleType>
                HandleType WrapNew(HandleType newlyCreatedHandle) {
                    if (newlyCreatedHandle == (HandleType)VK_NULL_HANDLE) return newlyCreatedHandle;
                    auto unique_id = global_unique_id++;
                    unique_id = HashedUint64::hash(unique_id);
                    assert(unique_id != 0);  // can't be 0, otherwise unwrap will apply special rule for VK_NULL_HANDLE
                    unique_id_mapping.insert_or_assign(unique_id, CastToUint64(newlyCreatedHandle));
                    return (HandleType)unique_id;
                }

                // Specialized handling for VkDisplayKHR. Adds an entry to enable reverse-lookup.
                VkDisplayKHR WrapDisplay(VkDisplayKHR newlyCreatedHandle, ValidationObject* map_data) {
                    auto unique_id = global_unique_id++;
                    unique_id = HashedUint64::hash(unique_id);
                    unique_id_mapping.insert_or_assign(unique_id, CastToUint64(newlyCreatedHandle));
                    map_data->display_id_reverse_mapping.insert_or_assign(newlyCreatedHandle, unique_id);
                    return (VkDisplayKHR)unique_id;
                }

                // VkDisplayKHR objects don't have a single point of creation, so we need to see if one already exists in the map before
                // creating another.
                VkDisplayKHR MaybeWrapDisplay(VkDisplayKHR handle, ValidationObject* map_data) {
                    // See if this display is already known
                    auto it = map_data->display_id_reverse_mapping.find(handle);
                    if (it != map_data->display_id_reverse_mapping.end()) return (VkDisplayKHR)it->second;
                    // Unknown, so wrap
                    return WrapDisplay(handle, map_data);
                }
            ''')

        out.append('''
        // clang-format off
        // Pre/post hook point declarations
''')

        for command in [x for x in self.vk.commands.values() if x.name not in self.ignore_functions and 'ValidationCache' not in x.name]:
            parameters = (command.cPrototype.split('(')[1])[:-2] # leaves just the parameters
            parameters = parameters.replace('\n', '')
            parameters = ' '.join(parameters.split()) # remove duplicate whitespace

            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            out.append(f'        virtual bool PreCallValidate{command.name[2:]}({parameters}, const ErrorObject& error_obj) const {{ return false; }};\n')
            out.append(f'        virtual void PreCallRecord{command.name[2:]}({parameters}) {{}};\n')
            out.append(f'        virtual void PostCallRecord{command.name[2:]}({parameters}, const RecordObject& record_obj) {{}};\n')
            out.extend(['#endif\n'] if command.protect else [])

        out.append('''
        virtual void CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache, const VkAllocationCallbacks* pAllocator) {};
        virtual VkResult CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount, const VkValidationCacheEXT* pSrcCaches)  { return VK_SUCCESS; };
        virtual VkResult CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize, void* pData)  { return VK_SUCCESS; };

        // Allow additional state parameter for CreateGraphicsPipelines
        virtual bool PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const ErrorObject& error_obj, void* cgpl_state) const {
            return PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, error_obj);
        };
        virtual void PreCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* cgpl_state) {
            PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        };
        virtual void PostCallRecordCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const RecordObject& record_obj, void* cgpl_state) {
            PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, record_obj);
        };

        // Allow additional state parameter for CreateComputePipelines
        virtual bool PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const ErrorObject& error_obj, void* pipe_state) const {
            return PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, error_obj);
        };
        virtual void PreCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* ccpl_state) {
            PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        };
        virtual void PostCallRecordCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const RecordObject& record_obj, void* pipe_state) {
            PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, record_obj);
        };

        // Allow additional state parameter for CreateRayTracingPipelinesNV
        virtual bool PreCallValidateCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const ErrorObject& error_obj, void* pipe_state) const {
            return PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, error_obj);
        };
        virtual void PreCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* ccpl_state) {
            PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        };
        virtual void PostCallRecordCreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const RecordObject& record_obj, void* pipe_state) {
            PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, record_obj);
        };

        // Allow additional state parameter for CreateRayTracingPipelinesKHR
        virtual bool PreCallValidateCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const ErrorObject& error_obj, void* pipe_state) const {
            return PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, error_obj);
        };
        virtual void PreCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, void* ccpl_state) {
            PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
        };
        virtual void PostCallRecordCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines, const RecordObject& record_obj, void* pipe_state) {
            PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines, record_obj);
        };

        // Allow modification of a down-chain parameter for CreatePipelineLayout
        virtual void PreCallRecordCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout, void *cpl_state) {
            PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
        };

        // Enable the CreateShaderModule/CreateShaderEXT API to take an extra argument for state preservation and paramter modification
        virtual void PreCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, void* csm_state) {
            PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
        };
        virtual void PostCallRecordCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule, const RecordObject& record_obj, void* csm_state) {
            PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj);
        };
        virtual void PreCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, void* csm_state) {
            PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders);
        };
        virtual void PostCallRecordCreateShadersEXT(VkDevice device, uint32_t createInfoCount, const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkShaderEXT* pShaders, const RecordObject& record_obj, void* csm_state) {
            PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj);
        };

        // Allow AllocateDescriptorSets to use some local stack storage for performance purposes
        virtual bool PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, const ErrorObject& error_obj, void* ads_state) const {
            return PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, error_obj);
        };
        virtual void PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets, const RecordObject& record_obj, void* ads_state)  {
            PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj);
        };

        // Allow modification of a down-chain parameter for CreateBuffer
        virtual void PreCallRecordCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer, void *cb_state) {
            PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
        };

        // Modify a parameter to CreateDevice
        virtual void PreCallRecordCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice, void *modified_create_info) {
            PreCallRecordCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
        };

        template <typename T>
        std::vector<T> ValidParamValues() const;
};
// clang-format on
''')

        out.append('extern small_unordered_map<void*, ValidationObject*, 2> layer_data_map;')
        out.append('\n#include "valid_enum_values.h"')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include <array>
            #include <cstring>
            #include <mutex>

            #include "chassis.h"
            #include "layer_options.h"
            #include "layer_chassis_dispatch.h"

            thread_local WriteLockGuard* ValidationObject::record_guard{};

            small_unordered_map<void*, ValidationObject*, 2> layer_data_map;

            // Global unique object identifier.
            std::atomic<uint64_t> global_unique_id(1ULL);
            // Map uniqueID to actual object handle. Accesses to the map itself are
            // internally synchronized.
            vl_concurrent_unordered_map<uint64_t, uint64_t, 4, HashedUint64> unique_id_mapping;

            bool wrap_handles = true;

            #define OBJECT_LAYER_DESCRIPTION "khronos_validation"\n
            ''')

        out.append('// Include layer validation object definitions\n')
        # Add #include directives for the used layers
        for layer in APISpecific.getValidationLayerList(self.targetApiName):
            out.append(f'#include "{layer["include"]}"\n')
        out.append('\n')

        out.append('// This header file must be included after the above validation object class definitions\n')
        out.append('#include "chassis_dispatch_helper.h"\n')
        out.append('\n')

        out.append('// Extension exposed by the validation layer\n')
        out.append('static constexpr std::array kInstanceExtensions = {\n')
        for ext in [x.upper() for x in APISpecific.getInstanceExtensionList(self.targetApiName)]:
            out.append(f'    VkExtensionProperties{{{ext}_EXTENSION_NAME, {ext}_SPEC_VERSION}},\n')
        out.append('};\n')
        out.append('static constexpr std::array kDeviceExtensions = {\n')
        for ext in [x.upper() for x in APISpecific.getDeviceExtensionList(self.targetApiName)]:
            out.append(f'    VkExtensionProperties{{{ext}_EXTENSION_NAME, {ext}_SPEC_VERSION}},\n')
        out.append('};\n')

        out.append('''
            // Layer registration code
            static std::vector<ValidationObject*> CreateObjectDispatch(const CHECK_ENABLED &enables, const CHECK_DISABLED &disables) {
                std::vector<ValidationObject*> object_dispatch{};

                // Add VOs to dispatch vector. Order here will be the validation dispatch order!
            ''')

        for layer in APISpecific.getValidationLayerList(self.targetApiName):
            constructor = layer['class']
            constructor += '(nullptr)' if layer['class'] == 'ThreadSafety' else ''
            out.append(f'''
                if ({layer["enabled"]}) {{
                    object_dispatch.emplace_back(new {constructor});
                }}''')
        out.append('\n')
        out.append('    return object_dispatch;\n')
        out.append('}\n')

        out.append('''
            static void InitDeviceObjectDispatch(ValidationObject *instance_interceptor, ValidationObject *device_interceptor) {
                auto disables = instance_interceptor->disabled;
                auto enables = instance_interceptor->enabled;

                // Note that this DEFINES THE ORDER IN WHICH THE LAYER VALIDATION OBJECTS ARE CALLED
            ''')
        for layer in APISpecific.getValidationLayerList(self.targetApiName):
            constructor = layer['class']
            if layer['class'] == 'ThreadSafety':
                constructor += '(instance_interceptor->GetValidationObject<ThreadSafety>())'
            out.append(f'''
                if ({layer["enabled"]}) {{
                    device_interceptor->object_dispatch.emplace_back(new {constructor});
                }}''')
        out.append('\n')
        out.append('}\n')

        out.append('''
            // Global list of sType,size identifiers
            std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};

            template <typename ValidationObjectType>
            ValidationObjectType* ValidationObject::GetValidationObject() const {
                LayerObjectTypeId type_id;
                if constexpr (std::is_same_v<ValidationObjectType, ThreadSafety>) {
                    type_id = LayerObjectTypeThreading;
                } else if constexpr (std::is_same_v<ValidationObjectType, StatelessValidation>) {
                    type_id = LayerObjectTypeParameterValidation;
                } else if constexpr (std::is_same_v<ValidationObjectType, ObjectLifetimes>) {
                    type_id = LayerObjectTypeObjectTracker;
                } else if constexpr (std::is_same_v<ValidationObjectType, CoreChecks>) {
                    type_id = LayerObjectTypeCoreValidation;
                } else {
                    static_assert(vvl::dependent_false_v<ValidationObjectType>, "unsupported validation object type");
                }
                return static_cast<ValidationObjectType*>(GetValidationObject(type_id));
            }

            template ThreadSafety* ValidationObject::GetValidationObject<ThreadSafety>() const;
            template StatelessValidation* ValidationObject::GetValidationObject<StatelessValidation>() const;
            template ObjectLifetimes* ValidationObject::GetValidationObject<ObjectLifetimes>() const;
            template CoreChecks* ValidationObject::GetValidationObject<CoreChecks>() const;

            namespace vulkan_layer_chassis {

            static const VkLayerProperties global_layer = {
                OBJECT_LAYER_NAME,
                VK_LAYER_API_VERSION,
                1,
                "LunarG validation Layer",
            };

            typedef enum ApiFunctionType { kFuncTypeInst = 0, kFuncTypePdev = 1, kFuncTypeDev = 2 } ApiFunctionType;

            typedef struct {
                ApiFunctionType function_type;
                void* funcptr;
            } function_data;

            extern const vvl::unordered_map<std::string, function_data> name_to_funcptr_map;
            ''')

        out.append('''
            // Manually written functions

            // Check enabled instance extensions against supported instance extension whitelist
            static void InstanceExtensionWhitelist(ValidationObject* layer_data, const VkInstanceCreateInfo* pCreateInfo, VkInstance instance) {
                for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                    // Check for recognized instance extensions
                    if (!white_list(pCreateInfo->ppEnabledExtensionNames[i], kInstanceExtensionNames)) {
                        Location loc(vvl::Func::vkCreateInstance);
                        layer_data->LogWarning(kVUIDUndefined, layer_data->instance,
                                            loc.dot(vvl::Field::pCreateInfo).dot(vvl::Field::ppEnabledExtensionNames, i),
                                            "%s is not supported by this layer.  Using this extension may adversely affect validation "
                                            "results and/or produce undefined behavior.",
                                            pCreateInfo->ppEnabledExtensionNames[i]);
                    }
                }
            }

            // Check enabled device extensions against supported device extension whitelist
            static void DeviceExtensionWhitelist(ValidationObject* layer_data, const VkDeviceCreateInfo* pCreateInfo, VkDevice device) {
                for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
                    // Check for recognized device extensions
                    if (!white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceExtensionNames)) {
                        Location loc(vvl::Func::vkCreateDevice);
                        layer_data->LogWarning(kVUIDUndefined, layer_data->device,
                                            loc.dot(vvl::Field::pCreateInfo).dot(vvl::Field::ppEnabledExtensionNames, i),
                                            "%s is not supported by this layer.  Using this extension may adversely affect validation "
                                            "results and/or produce undefined behavior.",
                                            pCreateInfo->ppEnabledExtensionNames[i]);
                    }
                }
            }

            void OutputLayerStatusInfo(ValidationObject* context) {
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
                        case kLocal:  // Intentionally fall through
                        default:
                            settings_status.append("default location (current working directory).");
                            break;
                    }
                }

                Location loc(vvl::Func::vkCreateInstance);
                // Output layer status information message
                context->LogInfo("UNASSIGNED-CreateInstance-status-message", context->instance, loc,
                    "Khronos Validation Layer Active:\\n    Settings File: %s\\n    Current Enables: %s.\\n    Current Disables: %s.\\n",
                    settings_status.c_str(), list_of_enables.c_str(), list_of_disables.c_str());

                // Create warning message if user is running debug layers.
            #ifndef NDEBUG
                context->LogPerformanceWarning(
                    "UNASSIGNED-CreateInstance-debug-warning", context->instance, loc,
                    "VALIDATION LAYERS WARNING: Using debug builds of the validation layers *will* adversely affect performance.");
            #endif
                if (!context->fine_grained_locking) {
                    context->LogPerformanceWarning(
                        "UNASSIGNED-CreateInstance-locking-warning", context->instance, loc,
                        "Fine-grained locking is disabled, this will adversely affect performance of multithreaded applications.");
                }
            }

            // Non-code-generated chassis API functions

            VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char* funcName) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                if (!ApiParentExtensionEnabled(funcName, &layer_data->device_extensions)) {
                    return nullptr;
                }
                const auto& item = name_to_funcptr_map.find(funcName);
                if (item != name_to_funcptr_map.end()) {
                    if (item->second.function_type != kFuncTypeDev) {
                        return nullptr;
                    } else {
                        return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
                    }
                }
                auto& table = layer_data->device_dispatch_table;
                if (!table.GetDeviceProcAddr) return nullptr;
                return table.GetDeviceProcAddr(device, funcName);
            }

            VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char* funcName) {
                const auto& item = name_to_funcptr_map.find(funcName);
                if (item != name_to_funcptr_map.end()) {
                    return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
                }
                auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
                auto& table = layer_data->instance_dispatch_table;
                if (!table.GetInstanceProcAddr) return nullptr;
                return table.GetInstanceProcAddr(instance, funcName);
            }

            VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char* funcName) {
                const auto& item = name_to_funcptr_map.find(funcName);
                if (item != name_to_funcptr_map.end()) {
                    if (item->second.function_type != kFuncTypePdev) {
                        return nullptr;
                    } else {
                        return reinterpret_cast<PFN_vkVoidFunction>(item->second.funcptr);
                    }
                }
                auto layer_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
                auto& table = layer_data->instance_dispatch_table;
                if (!table.GetPhysicalDeviceProcAddr) return nullptr;
                return table.GetPhysicalDeviceProcAddr(instance, funcName);
            }

            VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties) {
                return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
            }

            VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount,
                                                                        VkLayerProperties* pProperties) {
                return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
            }

            VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount,
                                                                                VkExtensionProperties* pProperties) {
                if (pLayerName && !strcmp(pLayerName, global_layer.layerName)) {
                    return util_GetExtensionProperties(static_cast<uint32_t>(kInstanceExtensions.size()), kInstanceExtensions.data(), pCount,
                                                    pProperties);
                }

                return VK_ERROR_LAYER_NOT_PRESENT;
            }

            VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                            uint32_t* pCount, VkExtensionProperties* pProperties) {
                if (pLayerName && !strcmp(pLayerName, global_layer.layerName)) {
                    return util_GetExtensionProperties(static_cast<uint32_t>(kDeviceExtensions.size()), kDeviceExtensions.data(), pCount,
                                                    pProperties);
                }

                assert(physicalDevice);
                auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
                return layer_data->instance_dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pCount, pProperties);
            }

            VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                        VkInstance* pInstance) {
                VkLayerInstanceCreateInfo* chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

                assert(chain_info->u.pLayerInfo);
                PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
                PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(nullptr, "vkCreateInstance");
                if (fpCreateInstance == nullptr) return VK_ERROR_INITIALIZATION_FAILED;
                chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
                uint32_t specified_version = (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
                APIVersion api_version = VK_MAKE_API_VERSION(VK_API_VERSION_VARIANT(specified_version), VK_API_VERSION_MAJOR(specified_version),
                                                            VK_API_VERSION_MINOR(specified_version), 0);

                auto report_data = new debug_report_data{};
                report_data->instance_pnext_chain = SafePnextCopy(pCreateInfo->pNext);
                ActivateInstanceDebugCallbacks(report_data);

                // Set up enable and disable features flags
                CHECK_ENABLED local_enables{};
                CHECK_DISABLED local_disables{};
                bool lock_setting;
                // select_instrumented_shaders is the only gpu-av setting that is off by default
                GpuAVSettings local_gpuav_settings = {true, true, true, true, true, true, false, 10000};
                ConfigAndEnvSettings config_and_env_settings_data{OBJECT_LAYER_DESCRIPTION,
                                                                pCreateInfo,
                                                                local_enables,
                                                                local_disables,
                                                                report_data->filter_message_ids,
                                                                &report_data->duplicate_message_limit,
                                                                &lock_setting,
                                                                &local_gpuav_settings};
                ProcessConfigAndEnvSettings(&config_and_env_settings_data);
                layer_debug_messenger_actions(report_data, OBJECT_LAYER_DESCRIPTION);

                // Create temporary dispatch vector for pre-calls until instance is created
                std::vector<ValidationObject*> local_object_dispatch = CreateObjectDispatch(local_enables, local_disables);

                // If handle wrapping is disabled via the ValidationFeatures extension, override build flag
                if (local_disables[handle_wrapping]) {
                    wrap_handles = false;
                }

                // Initialize the validation objects
                for (auto* intercept : local_object_dispatch) {
                    intercept->api_version = api_version;
                    intercept->report_data = report_data;
                }

                // Define logic to cleanup everything in case of an error
                auto cleanup_allocations = [report_data, &local_object_dispatch]() {
                    DeactivateInstanceDebugCallbacks(report_data);
                    FreePnextChain(report_data->instance_pnext_chain);
                    LayerDebugUtilsDestroyInstance(report_data);
                    for (ValidationObject* object : local_object_dispatch) {
                        delete object;
                    }
                };

                // Init dispatch array and call registration functions
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateInstance, VulkanTypedHandle());
                for (const ValidationObject* intercept : local_object_dispatch) {
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateInstance(pCreateInfo, pAllocator, pInstance, error_obj);
                    if (skip) {
                        cleanup_allocations();
                        return VK_ERROR_VALIDATION_FAILED_EXT;
                    }
                }
                for (ValidationObject* intercept : local_object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance);
                }

                VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
                if (result != VK_SUCCESS) {
                    cleanup_allocations();
                    return result;
                }
                auto framework = GetLayerDataPtr(get_dispatch_key(*pInstance), layer_data_map);

                framework->object_dispatch = local_object_dispatch;
                framework->container_type = LayerObjectTypeInstance;
                framework->disabled = local_disables;
                framework->enabled = local_enables;
                framework->fine_grained_locking = lock_setting;
                framework->gpuav_settings = local_gpuav_settings;

                framework->instance = *pInstance;
                layer_init_instance_dispatch_table(*pInstance, &framework->instance_dispatch_table, fpGetInstanceProcAddr);
                framework->report_data = report_data;
                framework->api_version = api_version;
                framework->instance_extensions.InitFromInstanceCreateInfo(specified_version, pCreateInfo);

                // We need to call this to properly check which device extensions have been promoted when validating query functions
                // that take as input a physical device, which can be called before a logical device has been created.
                framework->device_extensions.InitFromDeviceCreateInfo(&framework->instance_extensions, specified_version);

                OutputLayerStatusInfo(framework);

                for (auto* intercept : framework->object_dispatch) {
                    intercept->instance_dispatch_table = framework->instance_dispatch_table;
                    intercept->enabled = framework->enabled;
                    intercept->disabled = framework->disabled;
                    intercept->fine_grained_locking = framework->fine_grained_locking;
                    intercept->gpuav_settings = framework->gpuav_settings;
                    intercept->instance = *pInstance;
                }

                RecordObject record_obj(vvl::Func::vkCreateInstance, result);
                for (ValidationObject* intercept : framework->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateInstance(pCreateInfo, pAllocator, pInstance, record_obj);
                }

                InstanceExtensionWhitelist(framework, pCreateInfo, *pInstance);
                DeactivateInstanceDebugCallbacks(report_data);
                return result;
            }

            VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
                dispatch_key key = get_dispatch_key(instance);
                auto layer_data = GetLayerDataPtr(key, layer_data_map);
                ActivateInstanceDebugCallbacks(layer_data->report_data);
                ErrorObject error_obj(vvl::Func::vkDestroyInstance, VulkanTypedHandle(instance, kVulkanObjectTypeInstance));

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    intercept->PreCallValidateDestroyInstance(instance, pAllocator, error_obj);
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordDestroyInstance(instance, pAllocator);
                }

                layer_data->instance_dispatch_table.DestroyInstance(instance, pAllocator);

                RecordObject record_obj(vvl::Func::vkDestroyInstance);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordDestroyInstance(instance, pAllocator, record_obj);
                }

                DeactivateInstanceDebugCallbacks(layer_data->report_data);
                FreePnextChain(layer_data->report_data->instance_pnext_chain);

                LayerDebugUtilsDestroyInstance(layer_data->report_data);

                for (auto item = layer_data->object_dispatch.begin(); item != layer_data->object_dispatch.end(); item++) {
                    delete *item;
                }
                FreeLayerDataPtr(key, layer_data_map);
            }

            VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
                VkLayerDeviceCreateInfo* chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

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
                auto effective_api_version = std::min(APIVersion(device_properties.apiVersion), instance_interceptor->api_version);

                DeviceExtensions device_extensions = {};
                device_extensions.InitFromDeviceCreateInfo(&instance_interceptor->instance_extensions, effective_api_version, pCreateInfo);
                for (auto item : instance_interceptor->object_dispatch) {
                    item->device_extensions = device_extensions;
                }

                safe_VkDeviceCreateInfo modified_create_info(pCreateInfo);

                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateDevice, VulkanTypedHandle(gpu, kVulkanObjectTypePhysicalDevice));
                for (const ValidationObject* intercept : instance_interceptor->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, error_obj);
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : instance_interceptor->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, &modified_create_info);
                }

                VkResult result = fpCreateDevice(gpu, reinterpret_cast<VkDeviceCreateInfo*>(&modified_create_info), pAllocator, pDevice);
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

                instance_interceptor->report_data->device_created++;

                InitDeviceObjectDispatch(instance_interceptor, device_interceptor);

                // Initialize all of the objects with the appropriate data
                for (auto* object : device_interceptor->object_dispatch) {
                    object->device = device_interceptor->device;
                    object->physical_device = device_interceptor->physical_device;
                    object->instance = instance_interceptor->instance;
                    object->report_data = instance_interceptor->report_data;
                    object->device_dispatch_table = device_interceptor->device_dispatch_table;
                    object->api_version = device_interceptor->api_version;
                    object->disabled = instance_interceptor->disabled;
                    object->enabled = instance_interceptor->enabled;
                    object->fine_grained_locking = instance_interceptor->fine_grained_locking;
                    object->gpuav_settings = instance_interceptor->gpuav_settings;
                    object->instance_dispatch_table = instance_interceptor->instance_dispatch_table;
                    object->instance_extensions = instance_interceptor->instance_extensions;
                    object->device_extensions = device_interceptor->device_extensions;
                }

                RecordObject record_obj(vvl::Func::vkCreateDevice, result);
                for (ValidationObject* intercept : instance_interceptor->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateDevice(gpu, pCreateInfo, pAllocator, pDevice, record_obj);
                }

                device_interceptor->InitObjectDispatchVectors();

                DeviceExtensionWhitelist(device_interceptor, pCreateInfo, *pDevice);

                return result;
            }

            // NOTE: Do _not_ skip the dispatch call when destroying a device. Whether or not there was a validation error,
            //       the loader will destroy the device, and know nothing about future references to this device making it
            //       impossible for the caller to use this device handle further. IOW, this is our _only_ chance to (potentially)
            //       dispatch the driver's DestroyDevice function.
            VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
                dispatch_key key = get_dispatch_key(device);
                auto layer_data = GetLayerDataPtr(key, layer_data_map);
                ErrorObject error_obj(vvl::Func::vkCreateDevice, VulkanTypedHandle(device, kVulkanObjectTypeDevice));
                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    intercept->PreCallValidateDestroyDevice(device, pAllocator, error_obj);
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordDestroyDevice(device, pAllocator);
                }

                layer_data->device_dispatch_table.DestroyDevice(device, pAllocator);

                RecordObject record_obj(vvl::Func::vkDestroyDevice);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordDestroyDevice(device, pAllocator, record_obj);
                }

                auto instance_interceptor = GetLayerDataPtr(get_dispatch_key(layer_data->physical_device), layer_data_map);
                instance_interceptor->report_data->device_created--;

                for (auto item = layer_data->object_dispatch.begin(); item != layer_data->object_dispatch.end(); item++) {
                    delete *item;
                }
                FreeLayerDataPtr(key, layer_data_map);
            }

            // Special-case APIs for which core_validation needs custom parameter lists and/or modifies parameters

            VKAPI_ATTR VkResult VKAPI_CALL CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                                const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateGraphicsPipelines, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_graphics_pipeline_api_state cgpl_state[LayerObjectTypeMaxEnum]{};

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    cgpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                            pPipelines, error_obj, &(cgpl_state[intercept->container_type]));
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                    pPipelines, &(cgpl_state[intercept->container_type]));
                }

                auto usepCreateInfos =
                    (!cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : cgpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
                if (cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = cgpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

                VkResult result =
                    DispatchCreateGraphicsPipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

                RecordObject record_obj(vvl::Func::vkCreateGraphicsPipelines, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                    pPipelines, record_obj, &(cgpl_state[intercept->container_type]));
                }
                return result;
            }

            // This API saves some core_validation pipeline state state on the stack for performance purposes
            VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                                const VkComputePipelineCreateInfo* pCreateInfos,
                                                                const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateComputePipelines, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_compute_pipeline_api_state ccpl_state[LayerObjectTypeMaxEnum]{};

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    ccpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                            pPipelines, error_obj, &(ccpl_state[intercept->container_type]));
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines,
                                                                &(ccpl_state[intercept->container_type]));
                }

                auto usepCreateInfos =
                    (!ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos) ? pCreateInfos : ccpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
                if (ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos) usepCreateInfos = ccpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

                VkResult result =
                    DispatchCreateComputePipelines(device, pipelineCache, createInfoCount, usepCreateInfos, pAllocator, pPipelines);

                RecordObject record_obj(vvl::Func::vkCreateComputePipelines, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                    pPipelines, record_obj, &(ccpl_state[intercept->container_type]));
                }
                return result;
            }

            VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                                    const VkRayTracingPipelineCreateInfoNV* pCreateInfos,
                                                                    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateRayTracingPipelinesNV, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_ray_tracing_pipeline_api_state crtpl_state[LayerObjectTypeMaxEnum]{};

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    crtpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
                    auto lock = intercept->ReadLock();
                    skip |=
                        intercept->PreCallValidateCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                            pPipelines, error_obj, &(crtpl_state[intercept->container_type]));
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                        pPipelines, &(crtpl_state[intercept->container_type]));
                }

                VkResult result =
                    DispatchCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

                RecordObject record_obj(vvl::Func::vkCreateRayTracingPipelinesNV, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateRayTracingPipelinesNV(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator,
                                                                        pPipelines, record_obj, &(crtpl_state[intercept->container_type]));
                }
                return result;
            }

            VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation,
                                                                        VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                                        const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateRayTracingPipelinesKHR, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_ray_tracing_pipeline_khr_api_state crtpl_state[LayerObjectTypeMaxEnum]{};

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    crtpl_state[intercept->container_type].pCreateInfos = pCreateInfos;
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount,
                                                                                pCreateInfos, pAllocator, pPipelines, error_obj,
                                                                                &(crtpl_state[intercept->container_type]));
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount,
                                                                        pCreateInfos, pAllocator, pPipelines,
                                                                        &(crtpl_state[intercept->container_type]));
                }

                auto usepCreateInfos = (!crtpl_state[LayerObjectTypeGpuAssisted].pCreateInfos)
                             ? pCreateInfos
                             : crtpl_state[LayerObjectTypeGpuAssisted].pCreateInfos;
                if (crtpl_state[LayerObjectTypeDebugPrintf].pCreateInfos)
                    usepCreateInfos = crtpl_state[LayerObjectTypeDebugPrintf].pCreateInfos;

                VkResult result = DispatchCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount,
                                                                       usepCreateInfos, pAllocator, pPipelines);

                RecordObject record_obj(vvl::Func::vkCreateRayTracingPipelinesKHR, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount,
                                                                        pCreateInfos, pAllocator, pPipelines, record_obj,
                                                                        &(crtpl_state[intercept->container_type]));
                }
                return result;
            }

            // This API needs the ability to modify a down-chain parameter
            VKAPI_ATTR VkResult VKAPI_CALL CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo,
                                                                const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreatePipelineLayout, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_pipeline_layout_api_state cpl_state{};
                cpl_state.modified_create_info = *pCreateInfo;

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, error_obj);
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, &cpl_state);
                }
                VkResult result = DispatchCreatePipelineLayout(device, &cpl_state.modified_create_info, pAllocator, pPipelineLayout);
                RecordObject record_obj(vvl::Func::vkCreatePipelineLayout, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout, record_obj);
                }
                return result;
            }

            // This API needs some local stack data for performance reasons and also may modify a parameter
            VKAPI_ATTR VkResult VKAPI_CALL CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo,
                                                            const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateShaderModule, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_shader_module_api_state csm_state{};
                csm_state.instrumented_create_info = *pCreateInfo;

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, error_obj);
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, &csm_state);
                }

                // Special extra check if SPIR-V itself fails runtime validation in PreCallRecord
                if (!csm_state.valid_spirv) return VK_ERROR_VALIDATION_FAILED_EXT;

                VkResult result = DispatchCreateShaderModule(device, &csm_state.instrumented_create_info, pAllocator, pShaderModule);
                RecordObject record_obj(vvl::Func::vkCreateShaderModule, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule, record_obj, &csm_state);
                }
                return result;
            }

            VKAPI_ATTR VkResult VKAPI_CALL CreateShadersEXT(VkDevice device, uint32_t createInfoCount,
                                                            const VkShaderCreateInfoEXT* pCreateInfos, const VkAllocationCallbacks* pAllocator,
                                                            VkShaderEXT* pShaders) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateShadersEXT, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_shader_object_api_state csm_state(createInfoCount, pCreateInfos);

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateShadersEXT(device, createInfoCount, csm_state.instrumented_create_info, pAllocator, pShaders, error_obj);
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, &csm_state);
                }

                // Special extra check if SPIR-V itself fails runtime validation in PreCallRecord
                if (!csm_state.valid_spirv) return VK_ERROR_VALIDATION_FAILED_EXT;

                VkResult result = DispatchCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders);
                RecordObject record_obj(vvl::Func::vkCreateShadersEXT, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateShadersEXT(device, createInfoCount, pCreateInfos, pAllocator, pShaders, record_obj,
                                                            &csm_state);
                }
                return result;
            }

            VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo,
                                                                VkDescriptorSet* pDescriptorSets) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkAllocateDescriptorSets, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                cvdescriptorset::AllocateDescriptorSetsData ads_state[LayerObjectTypeMaxEnum];

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    ads_state[intercept->container_type].Init(pAllocateInfo->descriptorSetCount);
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, error_obj,
                                                                            &(ads_state[intercept->container_type]));
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
                }
                VkResult result = DispatchAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
                RecordObject record_obj(vvl::Func::vkAllocateDescriptorSets, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets, record_obj,
                                                                    &(ads_state[intercept->container_type]));
                }
                return result;
            }

            // This API needs the ability to modify a down-chain parameter
            VKAPI_ATTR VkResult VKAPI_CALL CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                        const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkCreateBuffer, VulkanTypedHandle(device, kVulkanObjectTypeDevice));

                create_buffer_api_state cb_state{};
                cb_state.modified_create_info = *pCreateInfo;

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidateCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, error_obj);
                    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
                }
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PreCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, &cb_state);
                }
                VkResult result = DispatchCreateBuffer(device, &cb_state.modified_create_info, pAllocator, pBuffer);
                RecordObject record_obj(vvl::Func::vkCreateBuffer, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordCreateBuffer(device, pCreateInfo, pAllocator, pBuffer, record_obj);
                }
                return result;
            }

            // Handle tooling queries manually as this is a request for layer information

            VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceToolPropertiesEXT(VkPhysicalDevice physicalDevice, uint32_t* pToolCount,
                                                                            VkPhysicalDeviceToolPropertiesEXT* pToolProperties) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
                bool skip = false;
                ErrorObject error_obj(vvl::Func::vkGetPhysicalDeviceToolPropertiesEXT,
                                    VulkanTypedHandle(physicalDevice, kVulkanObjectTypePhysicalDevice));

                static const VkPhysicalDeviceToolPropertiesEXT khronos_layer_tool_props = {
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT,
                    nullptr,
                    "Khronos Validation Layer",
                    STRINGIFY(VK_HEADER_VERSION),
                    VK_TOOL_PURPOSE_VALIDATION_BIT_EXT | VK_TOOL_PURPOSE_ADDITIONAL_FEATURES_BIT_EXT | VK_TOOL_PURPOSE_DEBUG_REPORTING_BIT_EXT |
                        VK_TOOL_PURPOSE_DEBUG_MARKERS_BIT_EXT,
                    "Khronos Validation Layer",
                    OBJECT_LAYER_NAME};

                auto original_pToolProperties = pToolProperties;

                if (pToolProperties != nullptr) {
                    *pToolProperties = khronos_layer_tool_props;
                    pToolProperties = ((*pToolCount > 1) ? &pToolProperties[1] : nullptr);
                    (*pToolCount)--;
                }

                for (const ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->ReadLock();
                    skip |=
                        intercept->PreCallValidateGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, error_obj);
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

                RecordObject record_obj(vvl::Func::vkGetPhysicalDeviceToolPropertiesEXT, result);
                for (ValidationObject* intercept : layer_data->object_dispatch) {
                    auto lock = intercept->WriteLock();
                    intercept->PostCallRecordGetPhysicalDeviceToolPropertiesEXT(physicalDevice, pToolCount, pToolProperties, record_obj);
                }
                return result;
            }

            // ValidationCache APIs do not dispatch

            VKAPI_ATTR VkResult VKAPI_CALL CreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT* pCreateInfo,
                                                                    const VkAllocationCallbacks* pAllocator,
                                                                    VkValidationCacheEXT* pValidationCache) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                if (auto core_checks = layer_data->GetValidationObject<CoreChecks>()) {
                    auto lock = core_checks->WriteLock();
                    return core_checks->CoreLayerCreateValidationCacheEXT(device, pCreateInfo, pAllocator, pValidationCache);
                }
                return VK_SUCCESS;
            }

            VKAPI_ATTR void VKAPI_CALL DestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                                const VkAllocationCallbacks* pAllocator) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                if (auto core_checks = layer_data->GetValidationObject<CoreChecks>()) {
                    auto lock = core_checks->WriteLock();
                    core_checks->CoreLayerDestroyValidationCacheEXT(device, validationCache, pAllocator);
                }
            }

            VKAPI_ATTR VkResult VKAPI_CALL MergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                                    const VkValidationCacheEXT* pSrcCaches) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                if (auto core_checks = layer_data->GetValidationObject<CoreChecks>()) {
                    auto lock = core_checks->WriteLock();
                    return core_checks->CoreLayerMergeValidationCachesEXT(device, dstCache, srcCacheCount, pSrcCaches);
                }
                return VK_SUCCESS;
            }

            VKAPI_ATTR VkResult VKAPI_CALL GetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t* pDataSize,
                                                                    void* pData) {
                auto layer_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
                if (auto core_checks = layer_data->GetValidationObject<CoreChecks>()) {
                    auto lock = core_checks->WriteLock();
                    return core_checks->CoreLayerGetValidationCacheDataEXT(device, validationCache, pDataSize, pData);
                }
                return VK_SUCCESS;
            }
            ''')

        for command in [x for x in self.vk.commands.values() if x.name not in self.ignore_functions and x.name not in self.manual_functions]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            prototype = command.cPrototype.replace('VKAPI_CALL vk', 'VKAPI_CALL ').replace(');', ') {\n')
            out.append(prototype)

            paramsList = ', '.join([param.name for param in command.params])

            # Setup common to call wrappers. First parameter is always dispatchable
            out.append(f'auto layer_data = GetLayerDataPtr(get_dispatch_key({command.params[0].name}), layer_data_map);\n')

            # Declare result variable, if any.
            return_map = {
                'PFN_vkVoidFunction': 'return nullptr;',
                'VkBool32': 'return VK_FALSE;',
                'VkDeviceAddress': 'return 0;',
                'VkDeviceSize': 'return 0;',
                'VkResult': 'return VK_ERROR_VALIDATION_FAILED_EXT;',
                'void': 'return;',
                'uint32_t': 'return 0;',
                'uint64_t': 'return 0;'
            }

            # Set up skip and locking
            out.append('bool skip = false;\n')

            out.append(f'ErrorObject error_obj(vvl::Func::{command.name}, VulkanTypedHandle({command.params[0].name}, kVulkanObjectType{command.params[0].type[2:]}));\n')

            # Generate pre-call validation source code
            if not command.instance:
                out.append(f'for (const ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallValidate{command.name[2:]}]) {{\n')
            else:
                out.append('for (const ValidationObject* intercept : layer_data->object_dispatch) {\n')
            out.append(f'''
                auto lock = intercept->ReadLock();
                    skip |= intercept->PreCallValidate{command.name[2:]}({paramsList}, error_obj);
                    if (skip) {return_map[command.returnType]}
                }}\n''')

            # Generate pre-call state recording source code
            if not command.instance:
                out.append(f'for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPreCallRecord{command.name[2:]}]) {{\n')
            else:
                out.append('for (ValidationObject* intercept : layer_data->object_dispatch) {\n')
            out.append(f'''
                auto lock = intercept->WriteLock();
                intercept->PreCallRecord{command.name[2:]}({paramsList});
            }}\n''')

            # Insert pre-dispatch debug utils function call
            pre_dispatch_debug_utils_functions = {
                'vkDebugMarkerSetObjectNameEXT' : 'layer_data->report_data->DebugReportSetMarkerObjectName(pNameInfo);',
                'vkSetDebugUtilsObjectNameEXT' : 'layer_data->report_data->DebugReportSetUtilsObjectName(pNameInfo);',
                'vkQueueBeginDebugUtilsLabelEXT' : 'BeginQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);',
                'vkQueueInsertDebugUtilsLabelEXT' : 'InsertQueueDebugUtilsLabel(layer_data->report_data, queue, pLabelInfo);',
            }
            if command.name in pre_dispatch_debug_utils_functions:
                out.append(f'    {pre_dispatch_debug_utils_functions[command.name]}\n')

            # Output dispatch (down-chain) function call
            assignResult = f'{command.returnType} result = ' if (command.returnType != 'void') else ''
            out.append(f'    {assignResult}{command.name.replace("vk", "Dispatch")}({paramsList});\n')


            # Insert post-dispatch debug utils function call
            post_dispatch_debug_utils_functions = {
                'vkQueueEndDebugUtilsLabelEXT' : 'EndQueueDebugUtilsLabel(layer_data->report_data, queue);',
                'vkCreateDebugReportCallbackEXT' : 'LayerCreateReportCallback(layer_data->report_data, false, pCreateInfo, pCallback);',
                'vkDestroyDebugReportCallbackEXT' : 'LayerDestroyCallback(layer_data->report_data, callback);',
                'vkCreateDebugUtilsMessengerEXT' : 'LayerCreateMessengerCallback(layer_data->report_data, false, pCreateInfo, pMessenger);',
                'vkDestroyDebugUtilsMessengerEXT' : 'LayerDestroyCallback(layer_data->report_data, messenger);',
            }
            if command.name in post_dispatch_debug_utils_functions:
                out.append(f'    {post_dispatch_debug_utils_functions[command.name]}\n')

            # Generate post-call object processing source code
            recordObj = ', result' if command.returnType == 'VkResult' or command.returnType == 'VkDeviceAddress' else ''
            out.append(f'RecordObject record_obj(vvl::Func::{command.name}{recordObj});\n')
            if not command.instance:
                out.append(f'for (ValidationObject* intercept : layer_data->intercept_vectors[InterceptIdPostCallRecord{command.name[2:]}]) {{\n')
            else:
                out.append('for (ValidationObject* intercept : layer_data->object_dispatch) {\n')

            # These commands perform blocking operations during PostRecord phase. We might need to
            # release ValidationObject's lock for the period of blocking operation to avoid deadlocks.
            # The released mutex can be re-acquired by the command that sets wait finish condition.
            # This functionality is needed when fine grained locking is disabled or not implemented.
            commands_with_blocking_operations = [
                'vkWaitSemaphores',
                'vkWaitSemaphoresKHR',

                # Note that get semaphore counter API commands do not block, but here we consider only
                # PostRecord phase which might block
                'vkGetSemaphoreCounterValue',
                'vkGetSemaphoreCounterValueKHR',
            ]

            if command.name not in commands_with_blocking_operations:
                out.append('auto lock = intercept->WriteLock();\n')
            else:
                out.append('ValidationObject::BlockingOperationGuard lock(intercept);\n')
            out.append(f'intercept->PostCallRecord{command.name[2:]}({paramsList}, record_obj);\n')
            out.append('}\n')
            # Return result variable, if any.
            if command.returnType != 'void':
                out.append('    return result;\n')
            out.append('}\n')
            out.append('\n')

            out.extend(['#endif\n'] if command.protect else [])

        out.append('''
// Map of intercepted ApiName to its associated function data
#ifdef _MSC_VER
#pragma warning( suppress: 6262 ) // VS analysis: this uses more than 16 kiB, which is fine here at global scope
#endif
// clang-format off
const vvl::unordered_map<std::string, function_data> name_to_funcptr_map = {
    {"vk_layerGetPhysicalDeviceProcAddr", {kFuncTypeInst, (void*)GetPhysicalDeviceProcAddr}},
''')
        for command in [x for x in self.vk.commands.values() if x.name not in self.ignore_functions]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            out.append(f'    {{"{command.name}", {{{self.getApiFunctionType(command)}, (void*){command.name[2:]}}}}},\n')
            out.extend(['#endif\n'] if command.protect else [])
        out.append('};\n')
        out.append('} // namespace vulkan_layer_chassis\n')
        out.append('// clang-format on\n')

        out.append('''
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
            ''')
        self.write("".join(out))

    def generateHelper(self):
        out = []
        out.append('''
            #pragma once

            // This source code creates dispatch vectors for each chassis api intercept,
            // i.e., PreCallValidateFoo, PreCallRecordFoo, PostCallRecordFoo, etc., ensuring that
            // each vector contains only the validation objects that override that particular base
            // class virtual function. Preventing non-overridden calls from reaching the default
            // functions saved about 5% in multithreaded applications.

            ''')

        out.append('typedef enum InterceptId{\n')
        for command in [x for x in self.vk.commands.values() if not x.instance and x.name not in self.manual_functions]:
            out.append(f'    InterceptIdPreCallValidate{command.name[2:]},\n')
            out.append(f'    InterceptIdPreCallRecord{command.name[2:]},\n')
            out.append(f'    InterceptIdPostCallRecord{command.name[2:]},\n')
        out.append('    InterceptIdCount,\n')
        out.append('} InterceptId;\n')

        out.append(APISpecific.genInitObjectDispatchVectorSource(self.targetApiName))

        for command in [x for x in self.vk.commands.values() if not x.instance and x.name not in self.manual_functions]:
            out.extend([f'#ifdef {command.protect}\n'] if command.protect else [])
            out.append(f'    BUILD_DISPATCH_VECTOR(PreCallValidate{command.name[2:]});\n')
            out.append(f'    BUILD_DISPATCH_VECTOR(PreCallRecord{command.name[2:]});\n')
            out.append(f'    BUILD_DISPATCH_VECTOR(PostCallRecord{command.name[2:]});\n')
            out.extend(['#endif\n'] if command.protect else [])
        out.append('}\n')
        self.write("".join(out))
