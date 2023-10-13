/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "best_practices/best_practices_validation.h"
#include "generated/layer_chassis_dispatch.h"
#include "best_practices/best_practices_error_enums.h"

const SpecialUseVUIDs kSpecialUseInstanceVUIDs{
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_CADSupport,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_D3DEmulation,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_DevTools,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_Debugging,
    kVUID_BestPractices_CreateInstance_SpecialUseExtension_GLEmulation,
};

const SpecialUseVUIDs kSpecialUseDeviceVUIDs{
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_CADSupport,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_D3DEmulation,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_DevTools,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_Debugging,
    kVUID_BestPractices_CreateDevice_SpecialUseExtension_GLEmulation,
};

const char* DepReasonToString(ExtDeprecationReason reason) {
    switch (reason) {
        case kExtPromoted:
            return "promoted to";
            break;
        case kExtObsoleted:
            return "obsoleted by";
            break;
        case kExtDeprecated:
            return "deprecated by";
            break;
        default:
            return "";
            break;
    }
}

bool BestPractices::ValidateDeprecatedExtensions(const Location& loc, const char* extension_name, APIVersion version,
                                                 const char* vuid) const {
    bool skip = false;
    auto dep_info_it = deprecated_extensions.find(extension_name);
    if (dep_info_it != deprecated_extensions.end()) {
        auto dep_info = dep_info_it->second;
        if (((dep_info.target.compare("VK_VERSION_1_1") == 0) && (version >= VK_API_VERSION_1_1)) ||
            ((dep_info.target.compare("VK_VERSION_1_2") == 0) && (version >= VK_API_VERSION_1_2)) ||
            ((dep_info.target.compare("VK_VERSION_1_3") == 0) && (version >= VK_API_VERSION_1_3))) {
            skip |=
                LogWarning(vuid, instance, loc, "Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                           extension_name, DepReasonToString(dep_info.reason), (dep_info.target).c_str());
        } else if (dep_info.target.find("VK_VERSION") == std::string::npos) {
            if (dep_info.target.length() == 0) {
                skip |= LogWarning(vuid, instance, loc,
                                   "Attempting to enable deprecated extension %s, but this extension has been deprecated "
                                   "without replacement.",
                                   extension_name);
            } else {
                skip |= LogWarning(vuid, instance, loc,
                                   "Attempting to enable deprecated extension %s, but this extension has been %s %s.",
                                   extension_name, DepReasonToString(dep_info.reason), (dep_info.target).c_str());
            }
        }
    }
    return skip;
}

bool BestPractices::ValidateSpecialUseExtensions(const Location& loc, const char* extension_name,
                                                 const SpecialUseVUIDs& special_use_vuids) const {
    bool skip = false;
    auto dep_info_it = special_use_extensions.find(extension_name);

    if (dep_info_it != special_use_extensions.end()) {
        const char* const format =
            "Attempting to enable extension %s, but this extension is intended to support %s "
            "and it is strongly recommended that it be otherwise avoided.";
        auto& special_uses = dep_info_it->second;

        if (special_uses.find("cadsupport") != std::string::npos) {
            skip |= LogWarning(special_use_vuids.cadsupport, instance, loc, format, extension_name,
                               "specialized functionality used by CAD/CAM applications");
        }
        if (special_uses.find("d3demulation") != std::string::npos) {
            skip |= LogWarning(special_use_vuids.d3demulation, instance, loc, format, extension_name,
                               "D3D emulation layers, and applications ported from D3D, by adding functionality specific to D3D");
        }
        if (special_uses.find("devtools") != std::string::npos) {
            skip |= LogWarning(special_use_vuids.devtools, instance, loc, format, extension_name,
                               "developer tools such as capture-replay libraries");
        }
        if (special_uses.find("debugging") != std::string::npos) {
            skip |= LogWarning(special_use_vuids.debugging, instance, loc, format, extension_name,
                               "use by applications when debugging");
        }
        if (special_uses.find("glemulation") != std::string::npos) {
            skip |= LogWarning(
                special_use_vuids.glemulation, instance, loc, format, extension_name,
                "OpenGL and/or OpenGL ES emulation layers, and applications ported from those APIs, by adding functionality "
                "specific to those APIs");
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                  VkInstance* pInstance, const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceExtensionNames)) {
            skip |= LogWarning(kVUID_BestPractices_CreateInstance_ExtensionMismatch, instance, error_obj.location,
                               "Attempting to enable Device Extension %s at CreateInstance time.",
                               pCreateInfo->ppEnabledExtensionNames[i]);
        }
        uint32_t specified_version =
            (pCreateInfo->pApplicationInfo ? pCreateInfo->pApplicationInfo->apiVersion : VK_API_VERSION_1_0);
        skip |= ValidateDeprecatedExtensions(error_obj.location, pCreateInfo->ppEnabledExtensionNames[i], specified_version,
                                             kVUID_BestPractices_CreateInstance_DeprecatedExtension);
        skip |= ValidateSpecialUseExtensions(error_obj.location, pCreateInfo->ppEnabledExtensionNames[i], kSpecialUseInstanceVUIDs);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice,
                                                const ErrorObject& error_obj) const {
    bool skip = false;

    // get API version of physical device passed when creating device.
    VkPhysicalDeviceProperties physical_device_properties{};
    DispatchGetPhysicalDeviceProperties(physicalDevice, &physical_device_properties);
    auto device_api_version = physical_device_properties.apiVersion;

    // Check api versions and log an info message when instance api Version is higher than version on device.
    if (api_version > device_api_version) {
        std::string inst_api_name = StringAPIVersion(api_version);
        std::string dev_api_name = StringAPIVersion(device_api_version);

        Location loc(Func::vkCreateDevice);
        LogInfo(kVUID_BestPractices_CreateDevice_API_Mismatch, device, loc,
                "API Version of current instance, %s is higher than API Version on device, %s", inst_api_name.c_str(),
                dev_api_name.c_str());
    }

    std::vector<std::string> extensions;
    {
        uint32_t property_count = 0;
        if (DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &property_count, nullptr) == VK_SUCCESS) {
            std::vector<VkExtensionProperties> property_list(property_count);
            if (DispatchEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &property_count, property_list.data()) ==
                VK_SUCCESS) {
                extensions.reserve(property_list.size());
                for (const VkExtensionProperties& properties : property_list) {
                    extensions.push_back(properties.extensionName);
                }
            }
        }
    }

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        const char* extension_name = pCreateInfo->ppEnabledExtensionNames[i];

        APIVersion extension_api_version = std::min(api_version, APIVersion(device_api_version));

        if (white_list(extension_name, kInstanceExtensionNames)) {
            skip |= LogWarning(kVUID_BestPractices_CreateDevice_ExtensionMismatch, instance, error_obj.location,
                               "Attempting to enable Instance Extension %s at CreateDevice time.", extension_name);
            extension_api_version = api_version;
        }

        skip |= ValidateDeprecatedExtensions(error_obj.location, extension_name, extension_api_version,
                                             kVUID_BestPractices_CreateDevice_DeprecatedExtension);
        skip |= ValidateSpecialUseExtensions(error_obj.location, extension_name, kSpecialUseDeviceVUIDs);
    }

    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if ((bp_pd_state->vkGetPhysicalDeviceFeaturesState == UNCALLED) && (pCreateInfo->pEnabledFeatures != NULL)) {
        skip |= LogWarning(kVUID_BestPractices_CreateDevice_PDFeaturesNotCalled, instance, error_obj.location,
                           "called before getting physical device features from vkGetPhysicalDeviceFeatures().");
    }

    if ((VendorCheckEnabled(kBPVendorArm) || VendorCheckEnabled(kBPVendorAMD) || VendorCheckEnabled(kBPVendorIMG)) &&
        (pCreateInfo->pEnabledFeatures != nullptr) && (pCreateInfo->pEnabledFeatures->robustBufferAccess == VK_TRUE)) {
        skip |= LogPerformanceWarning(
            kVUID_BestPractices_CreateDevice_RobustBufferAccess, instance, error_obj.location,
            "%s %s %s: called with enabled robustBufferAccess. Use robustBufferAccess as a debugging tool during "
            "development. Enabling it causes loss in performance for accesses to uniform buffers and shader storage "
            "buffers. Disable robustBufferAccess in release builds. Only leave it enabled if the application use-case "
            "requires the additional level of reliability due to the use of unverified user-supplied draw parameters.",
            VendorSpecificTag(kBPVendorArm), VendorSpecificTag(kBPVendorAMD), VendorSpecificTag(kBPVendorIMG));
    }

    const bool enabled_pageable_device_local_memory = IsExtEnabled(device_extensions.vk_ext_pageable_device_local_memory);
    if (VendorCheckEnabled(kBPVendorNVIDIA) && !enabled_pageable_device_local_memory &&
        std::find(extensions.begin(), extensions.end(), VK_EXT_PAGEABLE_DEVICE_LOCAL_MEMORY_EXTENSION_NAME) != extensions.end()) {
        skip |=
            LogPerformanceWarning(kVUID_BestPractices_CreateDevice_PageableDeviceLocalMemory, instance, error_obj.location,
                                  "%s called without pageable device local memory. "
                                  "Use pageableDeviceLocalMemory from VK_EXT_pageable_device_local_memory when it is available.",
                                  VendorSpecificTag(kBPVendorNVIDIA));
    }

    return skip;
}

// Common function to handle validation for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
bool BestPractices::ValidateCommonGetPhysicalDeviceQueueFamilyProperties(const PHYSICAL_DEVICE_STATE* bp_pd_state,
                                                                         uint32_t requested_queue_family_property_count,
                                                                         const CALL_STATE call_state, const Location& loc) const {
    bool skip = false;
    // Verify that for each physical device, this command is called first with NULL pQueueFamilyProperties in order to get count
    if (UNCALLED == call_state) {
        skip |= LogWarning(
            kVUID_BestPractices_DevLimit_MissingQueryCount, bp_pd_state->Handle(), loc,
            "is called with non-NULL pQueueFamilyProperties before obtaining pQueueFamilyPropertyCount. It is "
            "recommended "
            "to first call %s with NULL pQueueFamilyProperties in order to obtain the maximal pQueueFamilyPropertyCount.",
            loc.StringFunc());
        // Then verify that pCount that is passed in on second call matches what was returned
    } else if (bp_pd_state->queue_family_known_count != requested_queue_family_property_count) {
        skip |= LogWarning(kVUID_BestPractices_DevLimit_CountMismatch, bp_pd_state->Handle(), loc,
                           "is called with non-NULL pQueueFamilyProperties and pQueueFamilyPropertyCount value %" PRIu32
                           ", but the largest previously returned pQueueFamilyPropertyCount for this physicalDevice is %" PRIu32
                           ". It is recommended to instead receive all the properties by calling %s with "
                           "pQueueFamilyPropertyCount that was "
                           "previously obtained by calling %s with NULL pQueueFamilyProperties.",
                           requested_queue_family_property_count, bp_pd_state->queue_family_known_count, loc.StringFunc(),
                           loc.StringFunc());
    }

    return skip;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                          uint32_t* pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                          const ErrorObject& error_obj) const {
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (pQueueFamilyProperties && bp_pd_state) {
        return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(bp_pd_state.get(), *pQueueFamilyPropertyCount,
                                                                    bp_pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState,
                                                                    error_obj.location);
    }
    return false;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                           uint32_t* pQueueFamilyPropertyCount,
                                                                           VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                           const ErrorObject& error_obj) const {
    const auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (pQueueFamilyProperties && bp_pd_state) {
        return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(bp_pd_state.get(), *pQueueFamilyPropertyCount,
                                                                    bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2State,
                                                                    error_obj.location);
    }
    return false;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                              uint32_t* pQueueFamilyPropertyCount,
                                                                              VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                              const ErrorObject& error_obj) const {
    return PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties,
                                                                  error_obj);
}

void BestPractices::CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(CALL_STATE& call_state, bool no_pointer) {
    if (no_pointer) {
        if (UNCALLED == call_state) {
            call_state = QUERY_COUNT;
        }
    } else {  // Save queue family properties
        call_state = QUERY_DETAILS;
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                         uint32_t* pQueueFamilyPropertyCount,
                                                                         VkQueueFamilyProperties* pQueueFamilyProperties,
                                                                         const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                                 pQueueFamilyProperties, record_obj);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(bp_pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState,
                                                                   nullptr == pQueueFamilyProperties);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                          uint32_t* pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                          const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                                  pQueueFamilyProperties, record_obj);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        CommonPostCallRecordGetPhysicalDeviceQueueFamilyProperties(bp_pd_state->vkGetPhysicalDeviceQueueFamilyProperties2State,
                                                                   nullptr == pQueueFamilyProperties);
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                             uint32_t* pQueueFamilyPropertyCount,
                                                                             VkQueueFamilyProperties2* pQueueFamilyProperties,
                                                                             const RecordObject& record_obj) {
    PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties,
                                                          record_obj);
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                            const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures(physicalDevice, pFeatures, record_obj);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures,
                                                             const RecordObject& record_obj) {
    ValidationStateTracker::PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures, record_obj);
    auto bp_pd_state = Get<bp_state::PhysicalDevice>(physicalDevice);
    if (bp_pd_state) {
        bp_pd_state->vkGetPhysicalDeviceFeaturesState = QUERY_DETAILS;
    }
}

void BestPractices::PostCallRecordGetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physicalDevice,
                                                                VkPhysicalDeviceFeatures2* pFeatures,
                                                                const RecordObject& record_obj) {
    PostCallRecordGetPhysicalDeviceFeatures2(physicalDevice, pFeatures, record_obj);
}

void BestPractices::PreCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    ValidationStateTracker::PreCallRecordQueueSubmit(queue, submitCount, pSubmits, fence);

    auto queue_state = Get<QUEUE_STATE>(queue);
    for (uint32_t submit = 0; submit < submitCount; submit++) {
        const auto& submit_info = pSubmits[submit];
        for (uint32_t cb_index = 0; cb_index < submit_info.commandBufferCount; cb_index++) {
            auto cb = GetWrite<bp_state::CommandBuffer>(submit_info.pCommandBuffers[cb_index]);
            for (auto& func : cb->queue_submit_functions) {
                func(*this, *queue_state, *cb);
            }
            cb->num_submits++;
        }
    }
}

bool BestPractices::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence,
                                               const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        const Location submit_loc = error_obj.location.dot(Field::pSubmits, submit);
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreCount; semaphore++) {
            skip |= CheckPipelineStageFlags(submit_loc.dot(Field::pWaitDstStageMask, semaphore),
                                            pSubmits[submit].pWaitDstStageMask[semaphore]);
        }
        if (pSubmits[submit].signalSemaphoreCount == 0 && pSubmits[submit].pSignalSemaphores != nullptr) {
            LogInfo(kVUID_BestPractices_SemaphoreCount, device, error_obj.location,
                    "pSubmits[%" PRIu32 "].pSignalSemaphores is set, but pSubmits[%" PRIu32 "].signalSemaphoreCount is 0.", submit,
                    submit);
        }
        if (pSubmits[submit].waitSemaphoreCount == 0 && pSubmits[submit].pWaitSemaphores != nullptr) {
            LogInfo(kVUID_BestPractices_SemaphoreCount, device, error_obj.location,
                    "pSubmits[%" PRIu32 "].pWaitSemaphores is set, but pSubmits[%" PRIu32 "].waitSemaphoreCount is 0.", submit,
                    submit);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueSubmit2KHR(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2KHR* pSubmits,
                                                   VkFence fence, const ErrorObject& error_obj) const {
    return PreCallValidateQueueSubmit2(queue, submitCount, pSubmits, fence, error_obj);
}

bool BestPractices::PreCallValidateQueueSubmit2(VkQueue queue, uint32_t submitCount, const VkSubmitInfo2* pSubmits, VkFence fence,
                                                const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        const Location submit_loc = error_obj.location.dot(Field::pSubmits, submit);
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreInfoCount; semaphore++) {
            const Location semaphore_loc = submit_loc.dot(Field::pWaitSemaphoreInfos, semaphore);
            skip |= CheckPipelineStageFlags(semaphore_loc.dot(Field::stageMask),
                                            pSubmits[submit].pWaitSemaphoreInfos[semaphore].stageMask);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                   VkFence fence, const ErrorObject& error_obj) const {
    bool skip = false;

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; bind_idx++) {
        const VkBindSparseInfo& bind_info = pBindInfo[bind_idx];
        // Store sparse binding image_state and after binding is complete make sure that any requiring metadata have it bound
        vvl::unordered_set<const IMAGE_STATE*> sparse_images;
        // Track images getting metadata bound by this call in a set, it'll be recorded into the image_state
        // in RecordQueueBindSparse.
        vvl::unordered_set<const IMAGE_STATE*> sparse_images_with_metadata;
        // If we're binding sparse image memory make sure reqs were queried and note if metadata is required and bound
        for (uint32_t i = 0; i < bind_info.imageBindCount; ++i) {
            const auto& image_bind = bind_info.pImageBinds[i];
            auto image_state = Get<IMAGE_STATE>(image_bind.image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            sparse_images.insert(image_state.get());
            if (image_state->sparse_residency) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning(kVUID_BestPractices_MemTrack_InvalidState, image_state->image(), error_obj.location,
                                       "Binding sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       FormatHandle(image_state->image()).c_str());
                }
            }
            if (!image_state->memory_requirements_checked[0]) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning(kVUID_BestPractices_MemTrack_InvalidState, image_state->image(), error_obj.location,
                                   "Binding sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   FormatHandle(image_state->image()).c_str());
            }
        }
        for (uint32_t i = 0; i < bind_info.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bind_info.pImageOpaqueBinds[i];
            auto image_state = Get<IMAGE_STATE>(bind_info.pImageOpaqueBinds[i].image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            sparse_images.insert(image_state.get());
            if (image_state->sparse_residency) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= LogWarning(kVUID_BestPractices_MemTrack_InvalidState, image_state->image(), error_obj.location,
                                       "Binding opaque sparse memory to %s without first calling "
                                       "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                       FormatHandle(image_state->image()).c_str());
                }
            }
            if (!image_state->memory_requirements_checked[0]) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= LogWarning(kVUID_BestPractices_MemTrack_InvalidState, image_state->image(), error_obj.location,
                                   "Binding opaque sparse memory to %s without first calling "
                                   "vkGetImageMemoryRequirements() to retrieve requirements.",
                                   FormatHandle(image_state->image()).c_str());
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    sparse_images_with_metadata.insert(image_state.get());
                }
            }
        }
        for (const auto& sparse_image_state : sparse_images) {
            if (sparse_image_state->sparse_metadata_required && !sparse_image_state->sparse_metadata_bound &&
                sparse_images_with_metadata.find(sparse_image_state) == sparse_images_with_metadata.end()) {
                // Warn if sparse image binding metadata required for image with sparse binding, but metadata not bound
                skip |= LogWarning(kVUID_BestPractices_MemTrack_InvalidState, sparse_image_state->image(), error_obj.location,
                                   "Binding sparse memory to %s which requires a metadata aspect but no "
                                   "binding with VK_SPARSE_MEMORY_BIND_METADATA_BIT set was made.",
                                   FormatHandle(sparse_image_state->image()).c_str());
            }
        }
    }

    if (VendorCheckEnabled(kBPVendorNVIDIA)) {
        auto queue_state = Get<QUEUE_STATE>(queue);
        if (queue_state && queue_state->queueFamilyProperties.queueFlags != (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)) {
            skip |= LogPerformanceWarning(kVUID_BestPractices_QueueBindSparse_NotAsync, queue, error_obj.location,
                                          "vkQueueBindSparse() issued on queue %s. All binds should happen on an asynchronous copy "
                                          "queue to hide the OS scheduling and submit costs.",
                                          FormatHandle(queue).c_str());
        }
    }

    return skip;
}

void BestPractices::ManualPostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                        VkFence fence, const RecordObject& record_obj) {
    if (record_obj.result != VK_SUCCESS) {
        return;
    }

    for (uint32_t bind_idx = 0; bind_idx < bindInfoCount; bind_idx++) {
        const VkBindSparseInfo& bind_info = pBindInfo[bind_idx];
        for (uint32_t i = 0; i < bind_info.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bind_info.pImageOpaqueBinds[i];
            auto image_state = Get<IMAGE_STATE>(bind_info.pImageOpaqueBinds[i].image);
            if (!image_state) {
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    image_state->sparse_metadata_bound = true;
                }
            }
        }
    }
}

void BestPractices::ManualPostCallRecordQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                                    VkFence fence, const RecordObject& record_obj) {
    // AMD best practice
    num_queue_submissions_ += submitCount;
}
