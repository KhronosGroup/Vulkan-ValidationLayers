/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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
 * This file deals with anything related to Phyiscal Devices, Logical Devices, or Device Queues Families, Device Masks, etc
 */

#include <fstream>
#include <vector>

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <unistd.h>
#endif

#include <vulkan/vk_enum_string_helper.h>
#include "generated/chassis.h"
#include "core_validation.h"
#include "utils/shader_utils.h"

bool CoreChecks::ValidateDeviceQueueFamily(uint32_t queue_family, const Location &loc, const char *vuid,
                                           bool optional = false) const {
    bool skip = false;
    if (!optional && queue_family == VK_QUEUE_FAMILY_IGNORED) {
        skip |= LogError(vuid, device, loc,
                         "is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.");
    } else if (queue_family_index_set.find(queue_family) == queue_family_index_set.end()) {
        skip |=
            LogError(vuid, device, loc,
                     "(%" PRIu32
                     ") is not one of the queue families given via VkDeviceQueueCreateInfo structures when the device was created.",
                     queue_family);
    }

    return skip;
}

// Validate the specified queue families against the families supported by the physical device that owns this device
bool CoreChecks::ValidatePhysicalDeviceQueueFamilies(uint32_t queue_family_count, const uint32_t *queue_families,
                                                     const Location &loc, const char *vuid) const {
    bool skip = false;
    if (queue_families) {
        vvl::unordered_set<uint32_t> set;
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            if (set.count(queue_families[i])) {
                skip |= LogError(vuid, device, loc.dot(Field::pQueueFamilyIndices, i),
                                 "(%" PRIu32 ") is also in pQueueFamilyIndices[0].", queue_families[i]);
            } else {
                set.insert(queue_families[i]);
                if (queue_families[i] == VK_QUEUE_FAMILY_IGNORED) {
                    skip |= LogError(vuid, device, loc.dot(Field::pQueueFamilyIndices, i),
                                     "is VK_QUEUE_FAMILY_IGNORED, but it is required to provide a valid queue family index value.");
                } else if (queue_families[i] >= physical_device_state->queue_family_known_count) {
                    const LogObjectList objlist(physical_device, device);
                    skip |=
                        LogError(vuid, objlist, loc.dot(Field::pQueueFamilyIndices, i),
                                 "(%" PRIu32
                                 ") is not one of the queue families supported by the parent PhysicalDevice %s of this device %s.",
                                 queue_families[i], FormatHandle(physical_device).c_str(), FormatHandle(device).c_str());
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::GetPhysicalDeviceImageFormatProperties(IMAGE_STATE &image_state, const char *vuid_string,
                                                        const Location &loc) const {
    bool skip = false;
    const auto image_create_info = image_state.createInfo;
    VkResult image_properties_result = VK_SUCCESS;
    Func command = Func::vkGetPhysicalDeviceImageFormatProperties;
    if (image_create_info.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        image_properties_result = DispatchGetPhysicalDeviceImageFormatProperties(
            physical_device, image_create_info.format, image_create_info.imageType, image_create_info.tiling,
            image_create_info.usage, image_create_info.flags, &image_state.image_format_properties);
    } else {
        command = Func::vkGetPhysicalDeviceImageFormatProperties2;
        VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
        image_format_info.type = image_create_info.imageType;
        image_format_info.format = image_create_info.format;
        image_format_info.tiling = image_create_info.tiling;
        image_format_info.usage = image_create_info.usage;
        image_format_info.flags = image_create_info.flags;
        VkImageFormatProperties2 image_format_properties = vku::InitStructHelper();
        image_properties_result =
            DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_format_info, &image_format_properties);
        image_state.image_format_properties = image_format_properties.imageFormatProperties;
    }
    if (image_properties_result != VK_SUCCESS) {
        skip |= LogError(vuid_string, device, loc,
                         "internal call to %s unexpectedly "
                         "failed with result = %s, "
                         "when called for validation with following params: "
                         "format: %s, imageType: %s, "
                         "tiling: %s, usage: %s, "
                         "flags: %s.",
                         String(command), string_VkResult(image_properties_result), string_VkFormat(image_create_info.format),
                         string_VkImageType(image_create_info.imageType), string_VkImageTiling(image_create_info.tiling),
                         string_VkImageUsageFlags(image_create_info.usage).c_str(),
                         string_VkImageCreateFlags(image_create_info.flags).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToPhysicalDeviceCount(uint32_t deviceMask, const LogObjectList &objlist, const Location loc,
                                                         const char *vuid) const {
    bool skip = false;
    uint32_t count = 1 << physical_device_count;
    if (count <= deviceMask) {
        skip |= LogError(vuid, objlist, loc, "(0x%" PRIx32 ") is invalid, Physical device count is %" PRIu32 ".", deviceMask,
                         physical_device_count);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToZero(uint32_t deviceMask, const LogObjectList &objlist, const Location loc,
                                          const char *vuid) const {
    bool skip = false;
    if (deviceMask == 0) {
        skip |= LogError(vuid, objlist, loc, "is zero.");
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToCommandBuffer(const CMD_BUFFER_STATE &cb_state, uint32_t deviceMask,
                                                   const LogObjectList &objlist, const Location loc, const char *vuid) const {
    bool skip = false;
    if ((deviceMask & cb_state.initial_device_mask) != deviceMask) {
        skip |= LogError(vuid, objlist, loc, "(0x%" PRIx32 ") is not a subset of %s initial device mask (0x%" PRIx32 ").",
                         deviceMask, FormatHandle(cb_state).c_str(), cb_state.initial_device_mask);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceMaskToRenderPass(const CMD_BUFFER_STATE &cb_state, uint32_t deviceMask, const Location loc,
                                                const char *vuid) const {
    bool skip = false;
    if ((deviceMask & cb_state.active_render_pass_device_mask) != deviceMask) {
        skip |= LogError(vuid, cb_state.commandBuffer(), loc, "(0x%" PRIx32 ") is not a subset of %s device mask (0x%" PRIx32 ").",
                         deviceMask, FormatHandle(cb_state.activeRenderPass->renderPass()).c_str(),
                         cb_state.active_render_pass_device_mask);
    }
    return skip;
}

bool CoreChecks::ValidateQueueFamilyIndex(const PHYSICAL_DEVICE_STATE *pd_state, uint32_t requested_queue_family, const char *vuid,
                                          const Location &loc) const {
    bool skip = false;

    if (requested_queue_family >= pd_state->queue_family_known_count) {
        const char *conditional_ext_cmd =
            instance_extensions.vk_khr_get_physical_device_properties2 ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]" : "";

        skip |= LogError(vuid, pd_state->Handle(), loc,
                         "(%" PRIu32 ") is not less than any previously obtained pQueueFamilyPropertyCount %" PRIu32
                         " from "
                         "vkGetPhysicalDeviceQueueFamilyProperties%s.",
                         requested_queue_family, pd_state->queue_family_known_count, conditional_ext_cmd);
    }
    return skip;
}

bool CoreChecks::ValidateDeviceQueueCreateInfos(const PHYSICAL_DEVICE_STATE *pd_state, uint32_t info_count,
                                                const VkDeviceQueueCreateInfo *infos, const Location &loc) const {
    bool skip = false;

    const uint32_t not_used = std::numeric_limits<uint32_t>::max();
    struct create_flags {
        // uint32_t is to represent the queue family index to allow for better error messages
        uint32_t unprocted_index;
        uint32_t protected_index;
        create_flags(uint32_t a, uint32_t b) : unprocted_index(a), protected_index(b) {}
    };
    vvl::unordered_map<uint32_t, create_flags> queue_family_map;
    vvl::unordered_map<uint32_t, VkQueueGlobalPriorityKHR> global_priorities;

    for (uint32_t i = 0; i < info_count; ++i) {
        const Location info_loc = loc.dot(Field::pQueueCreateInfos, i);
        const uint32_t requested_queue_family = infos[i].queueFamilyIndex;
        const bool protected_create_bit = (infos[i].flags & VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT) != 0;

        skip |= ValidateQueueFamilyIndex(pd_state, requested_queue_family, "VUID-VkDeviceQueueCreateInfo-queueFamilyIndex-00381",
                                         info_loc.dot(Field::queueFamilyIndex));
        if (skip) {  // Skip if queue family index is invalid, as it will be used as index in arrays
            continue;
        }

        if (api_version == VK_API_VERSION_1_0) {
            // Vulkan 1.0 didn't have protected memory so always needed unique info
            create_flags flags = {requested_queue_family, not_used};
            if (queue_family_map.emplace(requested_queue_family, flags).second == false) {
                skip |= LogError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802", pd_state->Handle(),
                                 info_loc.dot(Field::queueFamilyIndex),
                                 "(%" PRIu32 ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%" PRIu32 "].",
                                 requested_queue_family, queue_family_map.at(requested_queue_family).unprocted_index);
            }
        } else {
            // Vulkan 1.1 and up can have 2 queues be same family index if one is protected and one isn't
            auto it = queue_family_map.find(requested_queue_family);
            if (it == queue_family_map.end()) {
                // Add first time seeing queue family index and what the create flags were
                create_flags new_flags = {not_used, not_used};
                if (protected_create_bit) {
                    new_flags.protected_index = requested_queue_family;
                } else {
                    new_flags.unprocted_index = requested_queue_family;
                }
                queue_family_map.emplace(requested_queue_family, new_flags);
            } else {
                // The queue family was seen, so now need to make sure the flags were different
                if (protected_create_bit) {
                    if (it->second.protected_index != not_used) {
                        skip |= LogError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802", pd_state->Handle(),
                                         info_loc.dot(Field::queueFamilyIndex),
                                         "(%" PRIu32 ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%" PRIu32
                                         "] which both have "
                                         "VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT.",
                                         requested_queue_family, queue_family_map.at(requested_queue_family).protected_index);
                    } else {
                        it->second.protected_index = requested_queue_family;
                    }
                } else {
                    if (it->second.unprocted_index != not_used) {
                        skip |= LogError("VUID-VkDeviceCreateInfo-queueFamilyIndex-02802", pd_state->Handle(),
                                         info_loc.dot(Field::queueFamilyIndex),
                                         "(%" PRIu32 ") is not unique and was also used in pCreateInfo->pQueueCreateInfos[%" PRIu32
                                         "].",
                                         requested_queue_family, queue_family_map.at(requested_queue_family).unprocted_index);
                    } else {
                        it->second.unprocted_index = requested_queue_family;
                    }
                }
            }
        }

        VkQueueGlobalPriorityKHR global_priority = VK_QUEUE_GLOBAL_PRIORITY_MEDIUM_KHR;  // Implicit default value
        const auto *global_priority_ci = vku::FindStructInPNextChain<VkDeviceQueueGlobalPriorityCreateInfoKHR>(infos[i].pNext);
        if (global_priority_ci) {
            global_priority = global_priority_ci->globalPriority;
        }
        const auto prev_global_priority = global_priorities.find(infos[i].queueFamilyIndex);
        if (prev_global_priority != global_priorities.end()) {
            if (prev_global_priority->second != global_priority) {
                skip |= LogError("VUID-VkDeviceCreateInfo-pQueueCreateInfos-06654", pd_state->Handle(), info_loc,
                                 "Multiple queues are created with queueFamilyIndex %" PRIu32
                                 ", but one has global priority %s and another %s.",
                                 infos[i].queueFamilyIndex, string_VkQueueGlobalPriorityKHR(prev_global_priority->second),
                                 string_VkQueueGlobalPriorityKHR(global_priority));
            }
        } else {
            global_priorities.insert({infos[i].queueFamilyIndex, global_priority});
        }

        const VkQueueFamilyProperties requested_queue_family_props = pd_state->queue_family_properties[requested_queue_family];

        // if using protected flag, make sure queue supports it
        if (protected_create_bit && ((requested_queue_family_props.queueFlags & VK_QUEUE_PROTECTED_BIT) == 0)) {
            skip |= LogError("VUID-VkDeviceQueueCreateInfo-flags-06449", pd_state->Handle(), info_loc.dot(Field::queueFamilyIndex),
                             "(%" PRIu32 ") does not have VK_QUEUE_PROTECTED_BIT supported, but pQueueCreateInfos[%" PRIu32
                             "].flags has VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT.",
                             requested_queue_family, i);
        }

        // Verify that requested queue count of queue family is known to be valid at this point in time
        if (requested_queue_family < pd_state->queue_family_known_count) {
            const auto requested_queue_count = infos[i].queueCount;
            const bool queue_family_has_props = requested_queue_family < pd_state->queue_family_properties.size();
            // spec guarantees at least one queue for each queue family
            const uint32_t available_queue_count = queue_family_has_props ? requested_queue_family_props.queueCount : 1;

            if (requested_queue_count > available_queue_count) {
                const char *conditional_ext_cmd = instance_extensions.vk_khr_get_physical_device_properties2
                                                      ? " or vkGetPhysicalDeviceQueueFamilyProperties2[KHR]"
                                                      : "";
                const std::string count_note =
                    queue_family_has_props
                        ? "i.e. is not less than or equal to " + std::to_string(requested_queue_family_props.queueCount)
                        : "the pQueueFamilyProperties[" + std::to_string(requested_queue_family) + "] was never obtained";

                skip |= LogError(
                    "VUID-VkDeviceQueueCreateInfo-queueCount-00382", pd_state->Handle(), info_loc.dot(Field::queueCount),
                    " (%" PRIu32
                    ") is not less than or equal to available queue count for this pCreateInfo->pQueueCreateInfos[%" PRIu32
                    "].queueFamilyIndex} (%" PRIu32 ") obtained previously from vkGetPhysicalDeviceQueueFamilyProperties%s (%s).",
                    requested_queue_count, i, requested_queue_family, conditional_ext_cmd, count_note.c_str());
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateDevice(VkPhysicalDevice gpu, const VkDeviceCreateInfo *pCreateInfo,
                                             const VkAllocationCallbacks *pAllocator, VkDevice *pDevice,
                                             const ErrorObject &error_obj) const {
    bool skip = false;
    auto pd_state = Get<PHYSICAL_DEVICE_STATE>(gpu);

    // TODO: object_tracker should perhaps do this instead
    //       and it does not seem to currently work anyway -- the loader just crashes before this point
    if (!pd_state) {
        skip |= LogError("VUID-vkCreateDevice-physicalDevice-parameter", device, error_obj.location,
                         "Have not called vkEnumeratePhysicalDevices() yet.");
    } else {
        skip |= ValidateDeviceQueueCreateInfos(pd_state.get(), pCreateInfo->queueCreateInfoCount, pCreateInfo->pQueueCreateInfos,
                                               error_obj.location.dot(Field::pCreateInfo));

        const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *fragment_shading_rate_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(pCreateInfo->pNext);

        if (fragment_shading_rate_features) {
            const VkPhysicalDeviceShadingRateImageFeaturesNV *shading_rate_image_features =
                vku::FindStructInPNextChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(pCreateInfo->pNext);

            if (shading_rate_image_features && shading_rate_image_features->shadingRateImage) {
                if (fragment_shading_rate_features->pipelineFragmentShadingRate) {
                    skip |= LogError("VUID-VkDeviceCreateInfo-shadingRateImage-04478", pd_state->Handle(), error_obj.location,
                                     "Cannot enable shadingRateImage and pipelineFragmentShadingRate features simultaneously.");
                }
                if (fragment_shading_rate_features->primitiveFragmentShadingRate) {
                    skip |= LogError("VUID-VkDeviceCreateInfo-shadingRateImage-04479", pd_state->Handle(), error_obj.location,
                                     "Cannot enable shadingRateImage and primitiveFragmentShadingRate features simultaneously.");
                }
                if (fragment_shading_rate_features->attachmentFragmentShadingRate) {
                    skip |= LogError("VUID-VkDeviceCreateInfo-shadingRateImage-04480", pd_state->Handle(), error_obj.location,
                                     "Cannot enable shadingRateImage and attachmentFragmentShadingRate features "
                                     "simultaneously.");
                }
            }

            const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *fragment_density_map_features =
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(pCreateInfo->pNext);

            if (fragment_density_map_features && fragment_density_map_features->fragmentDensityMap) {
                if (fragment_shading_rate_features->pipelineFragmentShadingRate) {
                    skip |= LogError("VUID-VkDeviceCreateInfo-fragmentDensityMap-04481", pd_state->Handle(), error_obj.location,
                                     "Cannot enable fragmentDensityMap and pipelineFragmentShadingRate features "
                                     "simultaneously.");
                }
                if (fragment_shading_rate_features->primitiveFragmentShadingRate) {
                    skip |= LogError("VUID-VkDeviceCreateInfo-fragmentDensityMap-04482", pd_state->Handle(), error_obj.location,
                                     "Cannot enable fragmentDensityMap and primitiveFragmentShadingRate features "
                                     "simultaneously.");
                }
                if (fragment_shading_rate_features->attachmentFragmentShadingRate) {
                    skip |= LogError("VUID-VkDeviceCreateInfo-fragmentDensityMap-04483", pd_state->Handle(), error_obj.location,
                                     "Cannot enable fragmentDensityMap and attachmentFragmentShadingRate features "
                                     "simultaneously.");
                }
            }
        }

        const auto *shader_image_atomic_int64_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(pCreateInfo->pNext);
        if (shader_image_atomic_int64_features) {
            if (shader_image_atomic_int64_features->sparseImageInt64Atomics &&
                !shader_image_atomic_int64_features->shaderImageInt64Atomics) {
                skip |= LogError("VUID-VkDeviceCreateInfo-None-04896", pd_state->Handle(), error_obj.location,
                                 "if shaderImageInt64Atomics feature is enabled then sparseImageInt64Atomics "
                                 "feature must also be enabled.");
            }
        }
        const auto *shader_atomic_float_features = vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float_features) {
            if (shader_atomic_float_features->sparseImageFloat32Atomics &&
                !shader_atomic_float_features->shaderImageFloat32Atomics) {
                skip |= LogError("VUID-VkDeviceCreateInfo-None-04897", pd_state->Handle(), error_obj.location,
                                 "if sparseImageFloat32Atomics feature is enabled then shaderImageFloat32Atomics "
                                 "feature must also be enabled.");
            }
            if (shader_atomic_float_features->sparseImageFloat32AtomicAdd &&
                !shader_atomic_float_features->shaderImageFloat32AtomicAdd) {
                skip |= LogError("VUID-VkDeviceCreateInfo-None-04898", pd_state->Handle(), error_obj.location,
                                 "if sparseImageFloat32AtomicAdd feature is enabled then shaderImageFloat32AtomicAdd "
                                 "feature must also be enabled.");
            }
        }
        const auto *shader_atomic_float2_features =
            vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(pCreateInfo->pNext);
        if (shader_atomic_float2_features) {
            if (shader_atomic_float2_features->sparseImageFloat32AtomicMinMax &&
                !shader_atomic_float2_features->shaderImageFloat32AtomicMinMax) {
                skip |=
                    LogError("VUID-VkDeviceCreateInfo-sparseImageFloat32AtomicMinMax-04975", pd_state->Handle(), error_obj.location,
                             "if sparseImageFloat32AtomicMinMax feature is enabled then shaderImageFloat32AtomicMinMax "
                             "feature must also be enabled.");
            }
        }
        const auto *device_group_ci = vku::FindStructInPNextChain<VkDeviceGroupDeviceCreateInfo>(pCreateInfo->pNext);
        if (device_group_ci) {
            for (uint32_t i = 0; i < device_group_ci->physicalDeviceCount - 1; ++i) {
                for (uint32_t j = i + 1; j < device_group_ci->physicalDeviceCount; ++j) {
                    if (device_group_ci->pPhysicalDevices[i] == device_group_ci->pPhysicalDevices[j]) {
                        skip |= LogError("VUID-VkDeviceGroupDeviceCreateInfo-pPhysicalDevices-00375", pd_state->Handle(),
                                         error_obj.location,
                                         "VkDeviceGroupDeviceCreateInfo has a duplicated physical device "
                                         "in pPhysicalDevices [%" PRIu32 "] and [%" PRIu32 "].",
                                         i, j);
                    }
                }
            }
        }
    }
    return skip;
}

void CoreChecks::CreateDevice(const VkDeviceCreateInfo *pCreateInfo) {
    // The state tracker sets up the device state
    StateTracker::CreateDevice(pCreateInfo);

    // Add the callback hooks for the functions that are either broadly or deeply used and that the ValidationStateTracker refactor
    // would be messier without.
    // TODO: Find a good way to do this hooklessly.
    SetSetImageViewInitialLayoutCallback(
        [](CMD_BUFFER_STATE *cb_state, const IMAGE_VIEW_STATE &iv_state, VkImageLayout layout) -> void {
            cb_state->SetImageViewInitialLayout(iv_state, layout);
        });

    // Allocate shader validation cache
    if (!disabled[shader_validation_caching] && !disabled[shader_validation] && !core_validation_cache) {
        auto tmp_path = GetTempFilePath();
        validation_cache_path = tmp_path + "/shader_validation_cache";
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        validation_cache_path += "-" + std::to_string(getuid());
#endif
        validation_cache_path += ".bin";

        std::vector<char> validation_cache_data;
        std::ifstream read_file(validation_cache_path.c_str(), std::ios::in | std::ios::binary);

        if (read_file) {
            std::copy(std::istreambuf_iterator<char>(read_file), {}, std::back_inserter(validation_cache_data));
            read_file.close();
        } else {
            Location loc(Func::vkCreateDevice);
            LogInfo("UNASSIGNED-cache-file-error", device, loc,
                    "Cannot open shader validation cache at %s for reading (it may not exist yet)", validation_cache_path.c_str());
        }

        VkValidationCacheCreateInfoEXT cacheCreateInfo = vku::InitStructHelper();
        cacheCreateInfo.initialDataSize = validation_cache_data.size();
        cacheCreateInfo.pInitialData = validation_cache_data.data();
        cacheCreateInfo.flags = 0;
        CoreLayerCreateValidationCacheEXT(device, &cacheCreateInfo, nullptr, &core_validation_cache);
    }
}

void CoreChecks::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    if (!device) return;

    StateTracker::PreCallRecordDestroyDevice(device, pAllocator);

    if (core_validation_cache) {
        Location loc(Func::vkDestroyDevice);
        size_t validation_cache_size = 0;
        void *validation_cache_data = nullptr;

        CoreLayerGetValidationCacheDataEXT(device, core_validation_cache, &validation_cache_size, nullptr);

        validation_cache_data = (char *)malloc(sizeof(char) * validation_cache_size);
        if (!validation_cache_data) {
            LogInfo("UNASSIGNED-cache-memory-error", device, loc, "Validation Cache Memory Error");
            return;
        }

        VkResult result =
            CoreLayerGetValidationCacheDataEXT(device, core_validation_cache, &validation_cache_size, validation_cache_data);

        if (result != VK_SUCCESS) {
            LogInfo("UNASSIGNED-cache-retrieval-error", device, loc, "Validation Cache Retrieval Error");
            free(validation_cache_data);
            return;
        }

        if (validation_cache_path.size() > 0) {
            std::ofstream write_file(validation_cache_path.c_str(), std::ios::out | std::ios::binary);
            if (write_file) {
                write_file.write(static_cast<char *>(validation_cache_data), validation_cache_size);
                write_file.close();
            } else {
                LogInfo("UNASSIGNED-cache-write-error", device, loc, "Cannot open shader validation cache at %s for writing",
                        validation_cache_path.c_str());
            }
        }
        free(validation_cache_data);
        CoreLayerDestroyValidationCacheEXT(device, core_validation_cache, NULL);
    }
}

bool CoreChecks::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue,
                                               const ErrorObject &error_obj) const {
    bool skip = false;

    skip |= ValidateDeviceQueueFamily(queueFamilyIndex, error_obj.location.dot(Field::queueFamilyIndex),
                                      "VUID-vkGetDeviceQueue-queueFamilyIndex-00384");

    for (size_t i = 0; i < device_queue_info_list.size(); i++) {
        const auto device_queue_info = device_queue_info_list.at(i);
        if (device_queue_info.queue_family_index != queueFamilyIndex) {
            continue;
        }

        // flag must be zero
        if (device_queue_info.flags != 0) {
            skip |= LogError(
                "VUID-vkGetDeviceQueue-flags-01841", device, error_obj.location.dot(Field::queueFamilyIndex),
                "(%" PRIu32
                ") was created with a non-zero VkDeviceQueueCreateFlags in vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                "]. Need to use vkGetDeviceQueue2 instead.",
                queueIndex, device_queue_info.index);
        }

        if (device_queue_info.queue_count <= queueIndex) {
            skip |= LogError("VUID-vkGetDeviceQueue-queueIndex-00385", device, error_obj.location.dot(Field::queueFamilyIndex),
                             "(%" PRIu32 ") is not less than the number of queues requested from queueFamilyIndex (=%" PRIu32
                             ") when the device was created vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                             "] (i.e. is not less than %" PRIu32 ").",
                             queueIndex, queueFamilyIndex, device_queue_info.index, device_queue_info.queue_count);
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue,
                                                const ErrorObject &error_obj) const {
    bool skip = false;

    if (pQueueInfo) {
        const Location queue_info_loc = error_obj.location.dot(Field::pQueueInfo);
        const uint32_t queueFamilyIndex = pQueueInfo->queueFamilyIndex;
        const uint32_t queueIndex = pQueueInfo->queueIndex;
        const VkDeviceQueueCreateFlags flags = pQueueInfo->flags;

        skip |= ValidateDeviceQueueFamily(queueFamilyIndex, queue_info_loc.dot(Field::queueFamilyIndex),
                                          "VUID-VkDeviceQueueInfo2-queueFamilyIndex-01842");

        // ValidateDeviceQueueFamily() already checks if queueFamilyIndex but need to make sure flags match with it
        bool valid_flags = false;

        for (size_t i = 0; i < device_queue_info_list.size(); i++) {
            const auto device_queue_info = device_queue_info_list.at(i);
            // vkGetDeviceQueue2 only checks if both family index AND flags are same as device creation
            // this handle case where the same queueFamilyIndex is used with/without the protected flag
            if ((device_queue_info.queue_family_index != queueFamilyIndex) || (device_queue_info.flags != flags)) {
                continue;
            }
            valid_flags = true;

            if (device_queue_info.queue_count <= queueIndex) {
                skip |= LogError(
                    "VUID-VkDeviceQueueInfo2-queueIndex-01843", device, error_obj.location.dot(Field::queueFamilyIndex),
                    "(%" PRIu32 ") is not less than the number of queues requested from [queueFamilyIndex (%" PRIu32
                    "), flags (%s)] combination when the device was created vkCreateDevice::pCreateInfo->pQueueCreateInfos[%" PRIu32
                    "] (requested %" PRIu32 " queues).",
                    queueIndex, queueFamilyIndex, string_VkDeviceQueueCreateFlags(flags).c_str(), device_queue_info.index,
                    device_queue_info.queue_count);
            }
        }

        // Don't double error message if already skipping from ValidateDeviceQueueFamily
        if (!valid_flags && !skip) {
            skip |= LogError("VUID-VkDeviceQueueInfo2-flags-06225", device, error_obj.location,
                             "The combination of queueFamilyIndex (%" PRIu32
                             ") and flags (%s) were never both set together in any element of "
                             "vkCreateDevice::pCreateInfo->pQueueCreateInfos at device creation time.",
                             queueFamilyIndex, string_VkDeviceQueueCreateFlags(flags).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetPhysicalDeviceImageFormatProperties2(const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                 VkImageFormatProperties2 *pImageFormatProperties,
                                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    const auto *copy_perf_query = vku::FindStructInPNextChain<VkHostImageCopyDevicePerformanceQueryEXT>(pImageFormatProperties->pNext);
    if (copy_perf_query) {
        if ((pImageFormatInfo->usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) == 0) {
            skip |= LogError("VUID-vkGetPhysicalDeviceImageFormatProperties2-pNext-09004", physical_device, error_obj.location,
                             "pImageFormatProperties includes a chained "
                             "VkHostImageCopyDevicePerformanceQueryEXT struct, but pImageFormatInfo->usage (%s) does not contain "
                             "VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT",
                             string_VkBufferUsageFlags(pImageFormatInfo->usage).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice,
                                                                        const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                        VkImageFormatProperties2 *pImageFormatProperties,
                                                                        const ErrorObject &error_obj) const {
    // Can't wrap AHB-specific validation in a device extension check here, but no harm
    bool skip = ValidateGetPhysicalDeviceImageFormatProperties2ANDROID(pImageFormatInfo, pImageFormatProperties, error_obj);
    skip |= ValidateGetPhysicalDeviceImageFormatProperties2(pImageFormatInfo, pImageFormatProperties, error_obj);
    return skip;
}

bool CoreChecks::PreCallValidateGetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
                                                                           VkImageFormatProperties2 *pImageFormatProperties,
                                                                           const ErrorObject &error_obj) const {
    return PreCallValidateGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties,
                                                                  error_obj);
}

// Access helper functions for external modules
VkFormatProperties3KHR CoreChecks::GetPDFormatProperties(const VkFormat format) const {
    VkFormatProperties3KHR fmt_props_3 = vku::InitStructHelper();
    VkFormatProperties2 fmt_props_2 = vku::InitStructHelper(&fmt_props_3);

    if (has_format_feature2) {
        DispatchGetPhysicalDeviceFormatProperties2(physical_device, format, &fmt_props_2);
        fmt_props_3.linearTilingFeatures |= fmt_props_2.formatProperties.linearTilingFeatures;
        fmt_props_3.optimalTilingFeatures |= fmt_props_2.formatProperties.optimalTilingFeatures;
        fmt_props_3.bufferFeatures |= fmt_props_2.formatProperties.bufferFeatures;
    } else {
        VkFormatProperties format_properties;
        DispatchGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
        fmt_props_3.linearTilingFeatures = format_properties.linearTilingFeatures;
        fmt_props_3.optimalTilingFeatures = format_properties.optimalTilingFeatures;
        fmt_props_3.bufferFeatures = format_properties.bufferFeatures;
    }
    return fmt_props_3;
}

VkResult CoreChecks::CoreLayerCreateValidationCacheEXT(VkDevice device, const VkValidationCacheCreateInfoEXT *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator,
                                                       VkValidationCacheEXT *pValidationCache) {
    *pValidationCache = ValidationCache::Create(pCreateInfo);
    return *pValidationCache ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED;
}

void CoreChecks::CoreLayerDestroyValidationCacheEXT(VkDevice device, VkValidationCacheEXT validationCache,
                                                    const VkAllocationCallbacks *pAllocator) {
    delete CastFromHandle<ValidationCache *>(validationCache);
}

VkResult CoreChecks::CoreLayerGetValidationCacheDataEXT(VkDevice device, VkValidationCacheEXT validationCache, size_t *pDataSize,
                                                        void *pData) {
    size_t in_size = *pDataSize;
    CastFromHandle<ValidationCache *>(validationCache)->Write(pDataSize, pData);
    return (pData && *pDataSize != in_size) ? VK_INCOMPLETE : VK_SUCCESS;
}

VkResult CoreChecks::CoreLayerMergeValidationCachesEXT(VkDevice device, VkValidationCacheEXT dstCache, uint32_t srcCacheCount,
                                                       const VkValidationCacheEXT *pSrcCaches) {
    bool skip = false;
    auto dst = CastFromHandle<ValidationCache *>(dstCache);
    VkResult result = VK_SUCCESS;
    for (uint32_t i = 0; i < srcCacheCount; i++) {
        auto src = CastFromHandle<const ValidationCache *>(pSrcCaches[i]);
        if (src == dst) {
            const Location loc(Func::vkMergePipelineCaches, Field::dstCache);
            skip |= LogError("VUID-vkMergeValidationCachesEXT-dstCache-01536", device, loc,
                             "(0x%" PRIx64 ") must not appear in pSrcCaches array.", HandleToUint64(dstCache));
            result = VK_ERROR_VALIDATION_FAILED_EXT;
        }
        if (!skip) {
            dst->Merge(src);
        }
    }

    return result;
}

bool CoreChecks::PreCallValidateCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<CMD_BUFFER_STATE>(commandBuffer);
    if (!cb_state_ptr) {
        return skip;
    }
    const CMD_BUFFER_STATE &cb_state = *cb_state_ptr;
    const LogObjectList objlist(commandBuffer);
    skip |= ValidateExtendedDynamicState(cb_state, error_obj.location, VK_TRUE, nullptr, nullptr);
    const Location loc = error_obj.location.dot(Field::deviceMask);
    skip |= ValidateDeviceMaskToPhysicalDeviceCount(deviceMask, objlist, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00108");
    skip |= ValidateDeviceMaskToZero(deviceMask, objlist, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00109");
    skip |= ValidateDeviceMaskToCommandBuffer(cb_state, deviceMask, objlist, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00110");
    if (cb_state.activeRenderPass) {
        skip |= ValidateDeviceMaskToRenderPass(cb_state, deviceMask, loc, "VUID-vkCmdSetDeviceMask-deviceMask-00111");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdSetDeviceMaskKHR(VkCommandBuffer commandBuffer, uint32_t deviceMask,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdSetDeviceMask(commandBuffer, deviceMask, error_obj);
}

bool CoreChecks::PreCallValidateCreatePrivateDataSlotEXT(VkDevice device, const VkPrivateDataSlotCreateInfoEXT *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkPrivateDataSlotEXT *pPrivateDataSlot,
                                                         const ErrorObject &error_obj) const {
    return PreCallValidateCreatePrivateDataSlot(device, pCreateInfo, pAllocator, pPrivateDataSlot, error_obj);
}

bool CoreChecks::PreCallValidateCreatePrivateDataSlot(VkDevice device, const VkPrivateDataSlotCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkPrivateDataSlot *pPrivateDataSlot,
                                                      const ErrorObject &error_obj) const {
    bool skip = false;
    if (!enabled_features.privateData) {
        skip |= LogError("VUID-vkCreatePrivateDataSlot-privateData-04564", device, error_obj.location,
                         "The privateData feature was not enabled.");
    }
    return skip;
}

bool CoreChecks::PreCallValidateCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
                                                  const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    skip |= ValidateDeviceQueueFamily(pCreateInfo->queueFamilyIndex, create_info_loc.dot(Field::queueFamilyIndex),
                                      "VUID-vkCreateCommandPool-queueFamilyIndex-01937");
    if ((enabled_features.protectedMemory == VK_FALSE) && ((pCreateInfo->flags & VK_COMMAND_POOL_CREATE_PROTECTED_BIT) != 0)) {
        skip |= LogError("VUID-VkCommandPoolCreateInfo-flags-02860", device, create_info_loc.dot(Field::flags),
                         "includes VK_COMMAND_POOL_CREATE_PROTECTED_BIT, but the protectedMemory feature was not enabled.");
    }

    return skip;
}

bool CoreChecks::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                   const VkAllocationCallbacks *pAllocator, const ErrorObject &error_obj) const {
    bool skip = false;
    auto cp_state = Get<COMMAND_POOL_STATE>(commandPool);
    if (!cp_state) { return false; }
    // Verify that command buffers in pool are complete (not in-flight)
    for (auto &entry : cp_state->commandBuffers) {
        auto cb_state = entry.second;
        if (cb_state->InUse()) {
            const LogObjectList objlist(cb_state->Handle(), commandPool);
            skip |= LogError("VUID-vkDestroyCommandPool-commandPool-00041", objlist, error_obj.location, "(%s) is in use.",
                             FormatHandle(cb_state->Handle()).c_str());
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto cp_state = Get<COMMAND_POOL_STATE>(commandPool);
    if (!cp_state) { return false; }
    // Verify that command buffers in pool are complete (not in-flight)
    for (auto &entry : cp_state->commandBuffers) {
        auto cb_state = entry.second;
        if (cb_state->InUse()) {
            const LogObjectList objlist(cb_state->Handle(), commandPool);
            skip |= LogError("VUID-vkResetCommandPool-commandPool-00040", objlist, error_obj.location, "(%s) is in use.",
                             FormatHandle(cb_state->Handle()).c_str());
        }
    }
    return skip;
}

// For given obj node, if it is use, flag a validation error and return callback result, else return false
bool CoreChecks::ValidateObjectNotInUse(const BASE_NODE *obj_node, const Location &loc, const char *error_code) const {
    if (disabled[object_in_use]) return false;
    auto obj_struct = obj_node->Handle();
    bool skip = false;

    const auto *used_handle = obj_node->InUse();
    if (used_handle) {
        skip |= LogError(error_code, device, loc, "can't be called on %s that is currently in use by %s.",
                         FormatHandle(obj_struct).c_str(), FormatHandle(*used_handle).c_str());
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetCalibratedTimestampsEXT(VkDevice device, uint32_t timestampCount,
                                                           const VkCalibratedTimestampInfoEXT *pTimestampInfos,
                                                           uint64_t *pTimestamps, uint64_t *pMaxDeviation,
                                                           const ErrorObject &error_obj) const {
    bool skip = false;

    uint32_t count = 0;
    DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_device, &count, nullptr);
    std::vector<VkTimeDomainEXT> valid_time_domains(count);
    DispatchGetPhysicalDeviceCalibrateableTimeDomainsEXT(physical_device, &count, valid_time_domains.data());

    vvl::unordered_map<VkTimeDomainEXT, uint32_t> time_domain_map;
    for (uint32_t i = 0; i < timestampCount; i++) {
        const VkTimeDomainEXT time_domain = pTimestampInfos[i].timeDomain;
        auto it = time_domain_map.find(time_domain);
        if (it != time_domain_map.end()) {
            skip |= LogError("VUID-vkGetCalibratedTimestampsEXT-timeDomain-09246", device,
                             error_obj.location.dot(Field::pTimestampInfos, i).dot(Field::timeDomain),
                             "and pTimestampInfos[%" PRIu32 "].timeDomain are both %s.", it->second,
                             string_VkTimeDomainEXT(time_domain));
            break;  // no reason to check after finding 1 duplicate
        } else if (!IsValueIn(time_domain, valid_time_domains)) {
            skip |= LogError("VUID-VkCalibratedTimestampInfoEXT-timeDomain-02354", device,
                             error_obj.location.dot(Field::pTimestampInfos, i).dot(Field::timeDomain), "is %s.",
                             string_VkTimeDomainEXT(time_domain));
        }
        time_domain_map[time_domain] = i;
    }
    return skip;
}