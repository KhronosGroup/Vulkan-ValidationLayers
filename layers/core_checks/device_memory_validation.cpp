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
 */

#include <assert.h>

#include "vk_enum_string_helper.h"
#include "chassis.h"
#include "core_checks/core_validation.h"

// For given mem object, verify that it is not null or UNBOUND, if it is, report error. Return skip value.
bool CoreChecks::VerifyBoundMemoryIsValid(const DEVICE_MEMORY_STATE *mem_state, const LogObjectList &objlist,
                                          const VulkanTypedHandle &typed_handle, const char *api_name,
                                          const char *error_code) const {
    return VerifyBoundMemoryIsValid<SimpleErrorLocation>(mem_state, objlist, typed_handle, {api_name, error_code});
}

template <typename LocType>
bool CoreChecks::VerifyBoundMemoryIsValid(const DEVICE_MEMORY_STATE *mem_state, const LogObjectList &objlist,
                                          const VulkanTypedHandle &typed_handle, const LocType &location) const {
    bool result = false;
    auto type_name = object_string[typed_handle.type];
    if (!mem_state) {
        result |= LogError(objlist, location.Vuid(),
                           "%s: %s used with no memory bound. Memory should be bound by calling vkBind%sMemory().",
                           location.FuncName(), report_data->FormatHandle(typed_handle).c_str(), type_name + 2);
    } else if (mem_state->Destroyed()) {
        result |= LogError(objlist, location.Vuid(),
                           "%s: %s used with no memory bound and previously bound memory was freed. Memory must not be freed "
                           "prior to this operation.",
                           location.FuncName(), report_data->FormatHandle(typed_handle).c_str());
    }
    return result;
}

// Check to see if memory was ever bound to this image
template <typename HandleT, typename LocType>
bool CoreChecks::ValidateMemoryIsBoundToImage(HandleT handle, const IMAGE_STATE &image_state, const LocType &location) const {
    bool result = false;
    if (image_state.create_from_swapchain != VK_NULL_HANDLE) {
        if (!image_state.bind_swapchain) {
            const LogObjectList objlist(handle, image_state.Handle(), image_state.create_from_swapchain);
            result |= LogError(
                objlist, location.Vuid(),
                "%s: %s is created by %s, and the image should be bound by calling vkBindImageMemory2(), and the pNext chain "
                "includes VkBindImageMemorySwapchainInfoKHR.",
                location.FuncName(), report_data->FormatHandle(image_state.image()).c_str(),
                report_data->FormatHandle(image_state.create_from_swapchain).c_str());
        } else if (image_state.create_from_swapchain != image_state.bind_swapchain->swapchain()) {
            const LogObjectList objlist(handle, image_state.Handle(), image_state.create_from_swapchain,
                                        image_state.bind_swapchain->Handle());
            result |=
                LogError(objlist, location.Vuid(),
                         "%s: %s is created by %s, but the image is bound by %s. The image should be created and bound by the same "
                         "swapchain",
                         location.FuncName(), report_data->FormatHandle(image_state.image()).c_str(),
                         report_data->FormatHandle(image_state.create_from_swapchain).c_str(),
                         report_data->FormatHandle(image_state.bind_swapchain->Handle()).c_str());
        }
    } else if (image_state.IsExternalAHB()) {
        // TODO look into how to properly check for a valid bound memory for an external AHB
    } else if (!image_state.sparse) {
        const LogObjectList objlist(handle, image_state.Handle());
        // No need to optimize this since the size will only be 3 at most
        const auto &memory_states = image_state.GetBoundMemoryStates();
        if (memory_states.empty()) {
            result |= LogError(objlist, location.Vuid(),
                               "%s: %s used with no memory bound. Memory should be bound by calling vkBindImageMemory().",
                               location.FuncName(), report_data->FormatHandle(image_state.Handle()).c_str());
        } else {
            for (const auto &state : memory_states) {
                result |= VerifyBoundMemoryIsValid(state.get(), objlist, image_state.Handle(), location);
            }
        }
    }
    return result;
}
// Instantiate the versions of the template needed by other .cpp files
template bool CoreChecks::ValidateMemoryIsBoundToImage<VkDevice, CoreChecks::SimpleErrorLocation>(
    VkDevice_T *, IMAGE_STATE const &, CoreChecks::SimpleErrorLocation const &) const;
template bool CoreChecks::ValidateMemoryIsBoundToImage<VkCommandBuffer,
                                                       core_error::LocationVuidAdapter<sync_vuid_maps::GetImageBarrierVUIDFunctor>>(
    VkCommandBuffer, IMAGE_STATE const &,
    core_error::LocationVuidAdapter<sync_vuid_maps::GetImageBarrierVUIDFunctor> const &) const;
template bool CoreChecks::ValidateMemoryIsBoundToImage<VkCommandBuffer, CoreChecks::SimpleErrorLocation>(
    VkCommandBuffer, IMAGE_STATE const &, CoreChecks::SimpleErrorLocation const &) const;

// Check to see if host-visible memory was bound to this buffer
bool CoreChecks::ValidateHostVisibleMemoryIsBoundToBuffer(const BUFFER_STATE &buffer_state, const char *api_name,
                                                          const char *error_code) const {
    bool result = false;
    result |= ValidateMemoryIsBoundToBuffer(device, buffer_state, api_name, error_code);
    if (!result) {
        const auto mem_state = buffer_state.MemState();
        if (mem_state) {
            if ((phys_dev_mem_props.memoryTypes[mem_state->alloc_info.memoryTypeIndex].propertyFlags &
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
                result |= LogError(buffer_state.Handle(), error_code, "%s: %s used with memory that is not host visible.", api_name,
                                   report_data->FormatHandle(buffer_state.Handle()).c_str());
            }
        }
    }
    return result;
}

// Valid usage checks for a call to SetMemBinding().
// For NULL mem case, output warning
// Make sure given object is in global object map
//  IF a previous binding existed, output validation error
//  Otherwise, add reference from objectInfo to memoryInfo
//  Add reference off of objInfo
// TODO: We may need to refactor or pass in multiple valid usage statements to handle multiple valid usage conditions.
bool CoreChecks::ValidateSetMemBinding(VkDeviceMemory mem, const BINDABLE &mem_binding, const char *apiName) const {
    bool skip = false;
    // It's an error to bind an object to NULL memory
    if (mem != VK_NULL_HANDLE) {
        auto typed_handle = mem_binding.Handle();
        if (mem_binding.sparse) {
            const char *error_code = nullptr;
            const char *handle_type = nullptr;
            if (typed_handle.type == kVulkanObjectTypeBuffer) {
                handle_type = "BUFFER";
                if (strcmp(apiName, "vkBindBufferMemory()") == 0) {
                    error_code = "VUID-vkBindBufferMemory-buffer-01030";
                } else {
                    error_code = "VUID-VkBindBufferMemoryInfo-buffer-01030";
                }
            } else if (typed_handle.type == kVulkanObjectTypeImage) {
                handle_type = "IMAGE";
                if (strcmp(apiName, "vkBindImageMemory()") == 0) {
                    error_code = "VUID-vkBindImageMemory-image-01045";
                } else {
                    error_code = "VUID-VkBindImageMemoryInfo-image-01045";
                }
            } else {
                // Unsupported object type
                assert(false);
            }

            const LogObjectList objlist(mem, typed_handle);
            skip |= LogError(objlist, error_code,
                             "In %s, attempting to bind %s to %s which was created with sparse memory flags "
                             "(VK_%s_CREATE_SPARSE_*_BIT).",
                             apiName, report_data->FormatHandle(mem).c_str(), report_data->FormatHandle(typed_handle).c_str(),
                             handle_type);
        }
        auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
        if (mem_info) {
            const auto *prev_binding = mem_binding.MemState();
            if (prev_binding) {
                const char *error_code = nullptr;
                if (typed_handle.type == kVulkanObjectTypeBuffer) {
                    if (strcmp(apiName, "vkBindBufferMemory()") == 0) {
                        error_code = "VUID-vkBindBufferMemory-buffer-07459";
                    } else {
                        error_code = "VUID-VkBindBufferMemoryInfo-buffer-07459";
                    }
                } else if (typed_handle.type == kVulkanObjectTypeImage) {
                    if (strcmp(apiName, "vkBindImageMemory()") == 0) {
                        error_code = "VUID-vkBindImageMemory-image-07460";
                    } else {
                        error_code = "VUID-VkBindImageMemoryInfo-image-07460";
                    }
                } else {
                    // Unsupported object type
                    assert(false);
                }

                const LogObjectList objlist(mem, typed_handle, prev_binding->mem());
                skip |= LogError(objlist, error_code, "In %s, attempting to bind %s to %s which has already been bound to %s.",
                                 apiName, report_data->FormatHandle(mem).c_str(), report_data->FormatHandle(typed_handle).c_str(),
                                 report_data->FormatHandle(prev_binding->mem()).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::IsZeroAllocationSizeAllowed(const VkMemoryAllocateInfo *pAllocateInfo) const {
    const VkExternalMemoryHandleTypeFlags ignored_allocation = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT |
                                                               VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT |
                                                               VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    const auto import_memory_win32 = LvlFindInChain<VkImportMemoryWin32HandleInfoKHR>(pAllocateInfo->pNext);
    if (import_memory_win32 && (import_memory_win32->handleType & ignored_allocation) != 0) {
        return true;
    }
#endif
    const auto import_memory_fd = LvlFindInChain<VkImportMemoryFdInfoKHR>(pAllocateInfo->pNext);
    if (import_memory_fd && (import_memory_fd->handleType & ignored_allocation) != 0) {
        return true;
    }
    const auto import_memory_host_pointer = LvlFindInChain<VkImportMemoryHostPointerInfoEXT>(pAllocateInfo->pNext);
    if (import_memory_host_pointer && (import_memory_host_pointer->handleType & ignored_allocation) != 0) {
        return true;
    }

    // Handles 01874 cases
    const auto export_info = LvlFindInChain<VkExportMemoryAllocateInfo>(pAllocateInfo->pNext);
    if (export_info && (export_info->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
        const auto dedicated_info = LvlFindInChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext);
        if (dedicated_info && dedicated_info->image) {
            return true;
        }
    }

#ifdef VK_USE_PLATFORM_FUCHSIA
    const auto import_memory_zircon = LvlFindInChain<VkImportMemoryZirconHandleInfoFUCHSIA>(pAllocateInfo->pNext);
    if (import_memory_zircon && (import_memory_zircon->handleType & ignored_allocation) != 0) {
        return true;
    }
#endif
    return false;
}

bool CoreChecks::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory) const {
    bool skip = false;
    if (Count<DEVICE_MEMORY_STATE>() >= phys_dev_props.limits.maxMemoryAllocationCount) {
        skip |= LogError(device, "VUID-vkAllocateMemory-maxMemoryAllocationCount-04101",
                         "vkAllocateMemory: Number of currently valid memory objects is not less than the maximum allowed (%u).",
                         phys_dev_props.limits.maxMemoryAllocationCount);
    }

    if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        skip |= ValidateAllocateMemoryANDROID(pAllocateInfo);
    } else {
        if (!IsZeroAllocationSizeAllowed(pAllocateInfo) && 0 == pAllocateInfo->allocationSize) {
            skip |= LogError(device, "VUID-VkMemoryAllocateInfo-allocationSize-00638", "vkAllocateMemory: allocationSize is 0.");
        }
    }

    auto chained_flags_struct = LvlFindInChain<VkMemoryAllocateFlagsInfo>(pAllocateInfo->pNext);
    if (chained_flags_struct && chained_flags_struct->flags == VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT) {
        const LogObjectList objlist(device);
        skip |= ValidateDeviceMaskToPhysicalDeviceCount(chained_flags_struct->deviceMask, objlist,
                                                        "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00675");
        skip |=
            ValidateDeviceMaskToZero(chained_flags_struct->deviceMask, objlist, "VUID-VkMemoryAllocateFlagsInfo-deviceMask-00676");
    }

    if (pAllocateInfo->memoryTypeIndex >= phys_dev_mem_props.memoryTypeCount) {
        skip |= LogError(device, "VUID-vkAllocateMemory-pAllocateInfo-01714",
                         "vkAllocateMemory: attempting to allocate memory type %u, which is not a valid index. Device only "
                         "advertises %u memory types.",
                         pAllocateInfo->memoryTypeIndex, phys_dev_mem_props.memoryTypeCount);
    } else {
        const VkMemoryType memory_type = phys_dev_mem_props.memoryTypes[pAllocateInfo->memoryTypeIndex];
        if (pAllocateInfo->allocationSize > phys_dev_mem_props.memoryHeaps[memory_type.heapIndex].size) {
            skip |= LogError(device, "VUID-vkAllocateMemory-pAllocateInfo-01713",
                             "vkAllocateMemory: attempting to allocate %" PRIu64
                             " bytes from heap %u,"
                             "but size of that heap is only %" PRIu64 " bytes.",
                             pAllocateInfo->allocationSize, memory_type.heapIndex,
                             phys_dev_mem_props.memoryHeaps[memory_type.heapIndex].size);
        }

        if (!enabled_features.device_coherent_memory_features.deviceCoherentMemory &&
            ((memory_type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) != 0)) {
            skip |= LogError(device, "VUID-vkAllocateMemory-deviceCoherentMemory-02790",
                             "vkAllocateMemory: attempting to allocate memory type %u, which includes the "
                             "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD memory property, but the deviceCoherentMemory feature "
                             "is not enabled.",
                             pAllocateInfo->memoryTypeIndex);
        }

        if ((enabled_features.core11.protectedMemory == VK_FALSE) &&
            ((memory_type.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) != 0)) {
            skip |= LogError(device, "VUID-VkMemoryAllocateInfo-memoryTypeIndex-01872",
                             "vkAllocateMemory(): attempting to allocate memory type %u, which includes the "
                             "VK_MEMORY_PROPERTY_PROTECTED_BIT memory property, but the protectedMemory feature "
                             "is not enabled.",
                             pAllocateInfo->memoryTypeIndex);
        }
    }

    bool imported_ahb = false;
#ifdef AHB_VALIDATION_SUPPORT
    //  "memory is not an imported Android Hardware Buffer" refers to VkImportAndroidHardwareBufferInfoANDROID with a non-NULL
    //  buffer value. Memory imported has another VUID to check size and allocationSize match up
    auto imported_ahb_info = LvlFindInChain<VkImportAndroidHardwareBufferInfoANDROID>(pAllocateInfo->pNext);
    if (imported_ahb_info != nullptr) {
        imported_ahb = imported_ahb_info->buffer != nullptr;
    }
#endif  // AHB_VALIDATION_SUPPORT
    auto dedicated_allocate_info = LvlFindInChain<VkMemoryDedicatedAllocateInfo>(pAllocateInfo->pNext);
    if (dedicated_allocate_info) {
        if ((dedicated_allocate_info->buffer != VK_NULL_HANDLE) && (dedicated_allocate_info->image != VK_NULL_HANDLE)) {
            skip |= LogError(device, "VUID-VkMemoryDedicatedAllocateInfo-image-01432",
                             "vkAllocateMemory: Either buffer or image has to be VK_NULL_HANDLE in VkMemoryDedicatedAllocateInfo");
        } else if (dedicated_allocate_info->image != VK_NULL_HANDLE) {
            // Dedicated VkImage
            auto image_state = Get<IMAGE_STATE>(dedicated_allocate_info->image);
            if (image_state->disjoint == true) {
                skip |= LogError(
                    device, "VUID-VkMemoryDedicatedAllocateInfo-image-01797",
                    "vkAllocateMemory: VkImage %s can't be used in VkMemoryDedicatedAllocateInfo because it was created with "
                    "VK_IMAGE_CREATE_DISJOINT_BIT",
                    report_data->FormatHandle(dedicated_allocate_info->image).c_str());
            } else {
                if (!IsZeroAllocationSizeAllowed(pAllocateInfo) &&
                    (pAllocateInfo->allocationSize != image_state->requirements[0].size) && (imported_ahb == false)) {
                    const char *vuid = IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)
                                           ? "VUID-VkMemoryDedicatedAllocateInfo-image-02964"
                                           : "VUID-VkMemoryDedicatedAllocateInfo-image-01433";
                    skip |=
                        LogError(device, vuid,
                                 "vkAllocateMemory: Allocation Size (%" PRIu64
                                 ") needs to be equal to VkImage %s VkMemoryRequirements::size (%" PRIu64 ")",
                                 pAllocateInfo->allocationSize, report_data->FormatHandle(dedicated_allocate_info->image).c_str(),
                                 image_state->requirements[0].size);
                }
                if ((image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != 0) {
                    skip |= LogError(
                        device, "VUID-VkMemoryDedicatedAllocateInfo-image-01434",
                        "vkAllocateMemory: VkImage %s can't be used in VkMemoryDedicatedAllocateInfo because it was created with "
                        "VK_IMAGE_CREATE_SPARSE_BINDING_BIT",
                        report_data->FormatHandle(dedicated_allocate_info->image).c_str());
                }
            }
        } else if (dedicated_allocate_info->buffer != VK_NULL_HANDLE) {
            // Dedicated VkBuffer
            auto buffer_state = Get<BUFFER_STATE>(dedicated_allocate_info->buffer);
            if (!IsZeroAllocationSizeAllowed(pAllocateInfo) && (pAllocateInfo->allocationSize != buffer_state->requirements.size) &&
                (imported_ahb == false)) {
                const char *vuid = IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)
                                       ? "VUID-VkMemoryDedicatedAllocateInfo-buffer-02965"
                                       : "VUID-VkMemoryDedicatedAllocateInfo-buffer-01435";
                skip |= LogError(device, vuid,
                                 "vkAllocateMemory: Allocation Size (%" PRIu64
                                 ") needs to be equal to VkBuffer %s VkMemoryRequirements::size (%" PRIu64 ")",
                                 pAllocateInfo->allocationSize, report_data->FormatHandle(dedicated_allocate_info->buffer).c_str(),
                                 buffer_state->requirements.size);
            }
            if ((buffer_state->createInfo.flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT) != 0) {
                skip |= LogError(
                    device, "VUID-VkMemoryDedicatedAllocateInfo-buffer-01436",
                    "vkAllocateMemory: VkBuffer %s can't be used in VkMemoryDedicatedAllocateInfo because it was created with "
                    "VK_BUFFER_CREATE_SPARSE_BINDING_BIT",
                    report_data->FormatHandle(dedicated_allocate_info->buffer).c_str());
            }
        }
    }

    // TODO: VUIDs ending in 00643, 00644, 00646, 00647, 01742, 01743, 01745, 00645, 00648, 01744
    return skip;
}

bool CoreChecks::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory mem, const VkAllocationCallbacks *pAllocator) const {
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
    bool skip = false;
    if (mem_info) {
        skip |= ValidateObjectNotInUse(mem_info.get(), "vkFreeMemory", "VUID-vkFreeMemory-memory-00677");
    }
    return skip;
}

// Validate that given Map memory range is valid. This means that the memory should not already be mapped,
//  and that the size of the map range should be:
//  1. Not zero
//  2. Within the size of the memory allocation
bool CoreChecks::ValidateMapMemRange(const DEVICE_MEMORY_STATE *mem_info, VkDeviceSize offset, VkDeviceSize size) const {
    bool skip = false;
    assert(mem_info);
    const auto mem = mem_info->mem();
    if (size == 0) {
        skip = LogError(mem, "VUID-vkMapMemory-size-00680", "VkMapMemory: Attempting to map memory range of size zero");
    }

    // It is an application error to call VkMapMemory on an object that is already mapped
    if (mem_info->mapped_range.size != 0) {
        skip = LogError(mem, "VUID-vkMapMemory-memory-00678", "VkMapMemory: Attempting to map memory on an already-mapped %s.",
                        report_data->FormatHandle(mem).c_str());
    }

    // Validate offset is not over allocaiton size
    if (offset >= mem_info->alloc_info.allocationSize) {
        skip = LogError(mem, "VUID-vkMapMemory-offset-00679",
                        "VkMapMemory: Attempting to map memory with an offset of 0x%" PRIx64
                        " which is larger than the total array size 0x%" PRIx64,
                        offset, mem_info->alloc_info.allocationSize);
    }
    // Validate that offset + size is within object's allocationSize
    if (size != VK_WHOLE_SIZE) {
        if ((offset + size) > mem_info->alloc_info.allocationSize) {
            skip = LogError(mem, "VUID-vkMapMemory-size-00681",
                            "VkMapMemory: Mapping Memory from 0x%" PRIx64 " to 0x%" PRIx64 " oversteps total array size 0x%" PRIx64
                            ".",
                            offset, size + offset, mem_info->alloc_info.allocationSize);
        }
    }
    return skip;
}

bool CoreChecks::ValidateInsertMemoryRange(const VulkanTypedHandle &typed_handle, const DEVICE_MEMORY_STATE *mem_info,
                                           VkDeviceSize memoryOffset, const char *api_name) const {
    bool skip = false;

    if (memoryOffset >= mem_info->alloc_info.allocationSize) {
        const char *error_code = nullptr;
        if (typed_handle.type == kVulkanObjectTypeBuffer) {
            if (strcmp(api_name, "vkBindBufferMemory()") == 0) {
                error_code = "VUID-vkBindBufferMemory-memoryOffset-01031";
            } else {
                error_code = "VUID-VkBindBufferMemoryInfo-memoryOffset-01031";
            }
        } else if (typed_handle.type == kVulkanObjectTypeImage) {
            if (strcmp(api_name, "vkBindImageMemory()") == 0) {
                error_code = "VUID-vkBindImageMemory-memoryOffset-01046";
            } else {
                error_code = "VUID-VkBindImageMemoryInfo-memoryOffset-01046";
            }
        } else if (typed_handle.type == kVulkanObjectTypeAccelerationStructureNV) {
            error_code = "VUID-VkBindAccelerationStructureMemoryInfoNV-memoryOffset-03621";
        } else {
            // Unsupported object type
            assert(false);
        }

        LogObjectList objlist(mem_info->mem(), typed_handle);
        skip = LogError(objlist, error_code,
                        "In %s, attempting to bind %s to %s, memoryOffset=0x%" PRIxLEAST64
                        " must be less than the memory allocation size 0x%" PRIxLEAST64 ".",
                        api_name, report_data->FormatHandle(mem_info->mem()).c_str(),
                        report_data->FormatHandle(typed_handle).c_str(), memoryOffset, mem_info->alloc_info.allocationSize);
    }

    return skip;
}

bool CoreChecks::ValidateInsertImageMemoryRange(VkImage image, const DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                const char *api_name) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(image, kVulkanObjectTypeImage), mem_info, mem_offset, api_name);
}

bool CoreChecks::ValidateInsertBufferMemoryRange(VkBuffer buffer, const DEVICE_MEMORY_STATE *mem_info, VkDeviceSize mem_offset,
                                                 const char *api_name) const {
    return ValidateInsertMemoryRange(VulkanTypedHandle(buffer, kVulkanObjectTypeBuffer), mem_info, mem_offset, api_name);
}

bool CoreChecks::ValidateMemoryTypes(const DEVICE_MEMORY_STATE *mem_info, const uint32_t memory_type_bits, const char *funcName,
                                     const char *msgCode) const {
    bool skip = false;
    if (((1 << mem_info->alloc_info.memoryTypeIndex) & memory_type_bits) == 0) {
        skip = LogError(mem_info->mem(), msgCode,
                        "%s(): MemoryRequirements->memoryTypeBits (0x%X) for this object type are not compatible with the memory "
                        "type (0x%X) of %s.",
                        funcName, memory_type_bits, mem_info->alloc_info.memoryTypeIndex,
                        report_data->FormatHandle(mem_info->mem()).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateBindBufferMemory(VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memoryOffset, const void *pNext,
                                          const char *api_name) const {
    auto buffer_state = Get<BUFFER_STATE>(buffer);
    const bool bind_buffer_mem_2 = strcmp(api_name, "vkBindBufferMemory()") != 0;

    bool skip = false;
    if (buffer_state) {
        // Track objects tied to memory
        skip = ValidateSetMemBinding(mem, *buffer_state, api_name);

        auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);

        // Validate memory requirements alignment
        if (SafeModulo(memoryOffset, buffer_state->requirements.alignment) != 0) {
            const char *vuid =
                bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memoryOffset-01036" : "VUID-vkBindBufferMemory-memoryOffset-01036";
            skip |= LogError(buffer, vuid,
                             "%s: memoryOffset is 0x%" PRIxLEAST64
                             " but must be an integer multiple of the VkMemoryRequirements::alignment value 0x%" PRIxLEAST64
                             ", returned from a call to vkGetBufferMemoryRequirements with buffer.",
                             api_name, memoryOffset, buffer_state->requirements.alignment);
        }

        if (mem_info) {
            // Validate bound memory range information
            skip |= ValidateInsertBufferMemoryRange(buffer, mem_info.get(), memoryOffset, api_name);

            const char *mem_type_vuid =
                bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memory-01035" : "VUID-vkBindBufferMemory-memory-01035";
            skip |= ValidateMemoryTypes(mem_info.get(), buffer_state->requirements.memoryTypeBits, api_name, mem_type_vuid);

            // Validate memory requirements size
            if (buffer_state->requirements.size > (mem_info->alloc_info.allocationSize - memoryOffset)) {
                const char *vuid =
                    bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-size-01037" : "VUID-vkBindBufferMemory-size-01037";
                skip |= LogError(buffer, vuid,
                                 "%s: memory size minus memoryOffset is 0x%" PRIxLEAST64
                                 " but must be at least as large as VkMemoryRequirements::size value 0x%" PRIxLEAST64
                                 ", returned from a call to vkGetBufferMemoryRequirements with buffer.",
                                 api_name, mem_info->alloc_info.allocationSize - memoryOffset, buffer_state->requirements.size);
            }

            // Validate dedicated allocation
            if (mem_info->IsDedicatedBuffer() &&
                ((mem_info->dedicated->handle.Cast<VkBuffer>() != buffer) || (memoryOffset != 0))) {
                const char *vuid =
                    bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memory-01508" : "VUID-vkBindBufferMemory-memory-01508";
                const LogObjectList objlist(buffer, mem, mem_info->dedicated->handle);
                skip |= LogError(objlist, vuid,
                                 "%s: for dedicated %s, VkMemoryDedicatedAllocateInfo::buffer %s must be equal "
                                 "to %s and memoryOffset 0x%" PRIxLEAST64 " must be zero.",
                                 api_name, report_data->FormatHandle(mem).c_str(),
                                 report_data->FormatHandle(mem_info->dedicated->handle).c_str(),
                                 report_data->FormatHandle(buffer).c_str(), memoryOffset);
            }

            auto chained_flags_struct = LvlFindInChain<VkMemoryAllocateFlagsInfo>(mem_info->alloc_info.pNext);
            if (enabled_features.core12.bufferDeviceAddress &&
                (buffer_state->createInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) &&
                (!chained_flags_struct || !(chained_flags_struct->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT))) {
                skip |= LogError(buffer, "VUID-vkBindBufferMemory-bufferDeviceAddress-03339",
                                 "%s: If buffer was created with the VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT bit set, "
                                 "memory must have been allocated with the VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT bit set.",
                                 api_name);
            }

            if (enabled_features.descriptor_buffer_features.descriptorBufferCaptureReplay &&
                (buffer_state->createInfo.flags & VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
                (!chained_flags_struct || !(chained_flags_struct->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT))) {
                const char *vuid = bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-descriptorBufferCaptureReplay-08112"
                                                     : "VUID-vkBindBufferMemory-descriptorBufferCaptureReplay-08112";
                skip |= LogError(
                    buffer, vuid,
                    "%s: If buffer was created with the VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT bit set, "
                    "memory must have been allocated with the VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT bit set.",
                    api_name);
            }

            // Validate export memory handles
            if (mem_info->export_handle_type_flags != 0) {
                auto external_info = LvlInitStruct<VkPhysicalDeviceExternalBufferInfo>();
                external_info.flags = buffer_state->createInfo.flags;
                external_info.usage = buffer_state->createInfo.usage;
                auto external_properties = LvlInitStruct<VkExternalBufferProperties>();
                bool export_supported = true;

                // Check export operation support
                auto check_export_support = [&](VkExternalMemoryHandleTypeFlagBits flag) {
                    external_info.handleType = flag;
                    DispatchGetPhysicalDeviceExternalBufferProperties(physical_device, &external_info, &external_properties);
                    if ((external_properties.externalMemoryProperties.externalMemoryFeatures &
                         VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) == 0) {
                        export_supported = false;
                        const LogObjectList objlist(buffer, mem);
                        skip |= LogError(objlist, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656",
                                         "%s: The VkDeviceMemory (%s) has VkExportMemoryAllocateInfo::handleTypes with the %s flag "
                                         "set, which does not support VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT with the buffer "
                                         "create flags (%s) and usage flags (%s).",
                                         api_name, report_data->FormatHandle(mem).c_str(),
                                         string_VkExternalMemoryHandleTypeFlagBits(flag),
                                         string_VkBufferCreateFlags(external_info.flags).c_str(),
                                         string_VkBufferUsageFlags(external_info.usage).c_str());
                    }
                };
                IterateFlags<VkExternalMemoryHandleTypeFlagBits>(mem_info->export_handle_type_flags, check_export_support);

                // The types of external memory handles must be compatible
                const auto compatible_types = external_properties.externalMemoryProperties.compatibleHandleTypes;
                if (export_supported &&
                    (mem_info->export_handle_type_flags & compatible_types) != mem_info->export_handle_type_flags) {
                    const LogObjectList objlist(buffer, mem);
                    skip |= LogError(objlist, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656",
                                     "%s: The VkDeviceMemory (%s) has VkExportMemoryAllocateInfo::handleTypes (%s) that are not "
                                     "reported as compatible by vkGetPhysicalDeviceExternalBufferProperties with the buffer create "
                                     "flags (%s) and usage flags (%s).",
                                     api_name, report_data->FormatHandle(mem).c_str(),
                                     string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_type_flags).c_str(),
                                     string_VkBufferCreateFlags(external_info.flags).c_str(),
                                     string_VkBufferUsageFlags(external_info.usage).c_str());
                }

                // Check if the memory meets the buffer's external memory requirements
                if ((mem_info->export_handle_type_flags & buffer_state->external_memory_handle) == 0) {
                    const char *vuid =
                        bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-memory-02726" : "VUID-vkBindBufferMemory-memory-02726";
                    const LogObjectList objlist(buffer, mem);
                    skip |=
                        LogError(objlist, vuid,
                                 "%s: The VkDeviceMemory (%s) has an external handleType of %s which does not include at least one "
                                 "handle from VkBuffer (%s) handleType %s.",
                                 api_name, report_data->FormatHandle(mem).c_str(),
                                 string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_type_flags).c_str(),
                                 report_data->FormatHandle(buffer).c_str(),
                                 string_VkExternalMemoryHandleTypeFlags(buffer_state->external_memory_handle).c_str());
                }
            }

            // Validate import memory handles
            if (mem_info->IsImportAHB() == true) {
                skip |= ValidateBufferImportedHandleANDROID(api_name, buffer_state->external_memory_handle, mem, buffer);
            } else if (mem_info->IsImport() == true) {
                if ((mem_info->import_handle_type_flags & buffer_state->external_memory_handle) == 0) {
                    const char *vuid = nullptr;
                    if ((bind_buffer_mem_2) && IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                        vuid = "VUID-VkBindBufferMemoryInfo-memory-02985";
                    } else if ((!bind_buffer_mem_2) &&
                               IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                        vuid = "VUID-vkBindBufferMemory-memory-02985";
                    } else if ((bind_buffer_mem_2) &&
                               !IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                        vuid = "VUID-VkBindBufferMemoryInfo-memory-02727";
                    } else if ((!bind_buffer_mem_2) &&
                               !IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                        vuid = "VUID-vkBindBufferMemory-memory-02727";
                    }
                    const LogObjectList objlist(buffer, mem);
                    skip |= LogError(objlist, vuid,
                                     "%s: The VkDeviceMemory (%s) was created with an import operation with handleType of %s which "
                                     "is not set in the VkBuffer (%s) VkExternalMemoryBufferCreateInfo::handleType (%s)",
                                     api_name, report_data->FormatHandle(mem).c_str(),
                                     string_VkExternalMemoryHandleTypeFlags(mem_info->import_handle_type_flags).c_str(),
                                     report_data->FormatHandle(buffer).c_str(),
                                     string_VkExternalMemoryHandleTypeFlags(buffer_state->external_memory_handle).c_str());
                }
            }

            // Validate mix of protected buffer and memory
            if ((buffer_state->unprotected == false) && (mem_info->unprotected == true)) {
                const char *vuid =
                    bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-None-01898" : "VUID-vkBindBufferMemory-None-01898";
                const LogObjectList objlist(buffer, mem);
                skip |= LogError(objlist, vuid,
                                 "%s: The VkDeviceMemory (%s) was not created with protected memory but the VkBuffer (%s) was set "
                                 "to use protected memory.",
                                 api_name, report_data->FormatHandle(mem).c_str(), report_data->FormatHandle(buffer).c_str());
            } else if ((buffer_state->unprotected == true) && (mem_info->unprotected == false)) {
                const char *vuid =
                    bind_buffer_mem_2 ? "VUID-VkBindBufferMemoryInfo-None-01899" : "VUID-vkBindBufferMemory-None-01899";
                const LogObjectList objlist(buffer, mem);
                skip |= LogError(objlist, vuid,
                                 "%s: The VkDeviceMemory (%s) was created with protected memory but the VkBuffer (%s) was not set "
                                 "to use protected memory.",
                                 api_name, report_data->FormatHandle(mem).c_str(), report_data->FormatHandle(buffer).c_str());
            }
        }
        const auto *bind_buffer_memory_device_group_info = LvlFindInChain<VkBindBufferMemoryDeviceGroupInfo>(pNext);
        if (bind_buffer_memory_device_group_info) {
            if (bind_buffer_memory_device_group_info->deviceIndexCount != 0 &&
                bind_buffer_memory_device_group_info->deviceIndexCount != device_group_create_info.physicalDeviceCount &&
                device_group_create_info.physicalDeviceCount > 0) {
                skip |= LogError(buffer, "VUID-VkBindBufferMemoryDeviceGroupInfo-deviceIndexCount-01606",
                                 "%s: The number of physical devices in the logical device is %" PRIu32
                                 ", but VkBindBufferMemoryDeviceGroupInfo::deviceIndexCount is %" PRIu32 ".",
                                 api_name, device_group_create_info.physicalDeviceCount,
                                 bind_buffer_memory_device_group_info->deviceIndexCount);
            } else {
                for (uint32_t i = 0; i < bind_buffer_memory_device_group_info->deviceIndexCount; ++i) {
                    if (bind_buffer_memory_device_group_info->pDeviceIndices[i] >= device_group_create_info.physicalDeviceCount) {
                        skip |= LogError(buffer, "VUID-VkBindBufferMemoryDeviceGroupInfo-pDeviceIndices-01607",
                                         "%s: The number of physical devices in the logical device is %" PRIu32
                                         ", but VkBindBufferMemoryDeviceGroupInfo::pDeviceIndices[%" PRIu32 "] is %" PRIu32 ".",
                                         api_name, device_group_create_info.physicalDeviceCount, i,
                                         bind_buffer_memory_device_group_info->pDeviceIndices[i]);
                    }
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem,
                                                 VkDeviceSize memoryOffset) const {
    const char *api_name = "vkBindBufferMemory()";
    return ValidateBindBufferMemory(buffer, mem, memoryOffset, nullptr, api_name);
}

bool CoreChecks::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                  const VkBindBufferMemoryInfo *pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindBufferMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset,
                                         pBindInfos[i].pNext, api_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo *pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        snprintf(api_name, sizeof(api_name), "vkBindBufferMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, pBindInfos[i].memory, pBindInfos[i].memoryOffset,
                                         pBindInfos[i].pNext, api_name);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements(VkDevice device, VkImage image,
                                                           VkMemoryRequirements *pMemoryRequirements) const {
    bool skip = false;
    if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        skip |= ValidateGetImageMemoryRequirementsANDROID(image, "vkGetImageMemoryRequirements()");
    }

    auto image_state = Get<IMAGE_STATE>(image);
    if (image_state) {
        // Checks for no disjoint bit
        if (image_state->disjoint == true) {
            skip |= LogError(image, "VUID-vkGetImageMemoryRequirements-image-01588",
                             "vkGetImageMemoryRequirements(): %s must not have been created with the VK_IMAGE_CREATE_DISJOINT_BIT "
                             "(need to use vkGetImageMemoryRequirements2).",
                             report_data->FormatHandle(image).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateGetImageMemoryRequirements2(const VkImageMemoryRequirementsInfo2 *pInfo, const char *func_name) const {
    bool skip = false;
    if (IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
        skip |= ValidateGetImageMemoryRequirementsANDROID(pInfo->image, func_name);
    }

    auto image_state = Get<IMAGE_STATE>(pInfo->image);
    const VkFormat image_format = image_state->createInfo.format;
    const VkImageTiling image_tiling = image_state->createInfo.tiling;
    const VkImagePlaneMemoryRequirementsInfo *image_plane_info = LvlFindInChain<VkImagePlaneMemoryRequirementsInfo>(pInfo->pNext);

    if ((FormatIsMultiplane(image_format)) && (image_state->disjoint == true) && (image_plane_info == nullptr)) {
        skip |= LogError(pInfo->image, "VUID-VkImageMemoryRequirementsInfo2-image-01589",
                         "%s: %s image was created with a multi-planar format (%s) and "
                         "VK_IMAGE_CREATE_DISJOINT_BIT, but the current pNext doesn't include a "
                         "VkImagePlaneMemoryRequirementsInfo struct",
                         func_name, report_data->FormatHandle(pInfo->image).c_str(), string_VkFormat(image_format));
    }

    if ((image_state->disjoint == false) && (image_plane_info != nullptr)) {
        skip |= LogError(pInfo->image, "VUID-VkImageMemoryRequirementsInfo2-image-01590",
                         "%s: %s image was not created with VK_IMAGE_CREATE_DISJOINT_BIT,"
                         "but the current pNext includes a VkImagePlaneMemoryRequirementsInfo struct",
                         func_name, report_data->FormatHandle(pInfo->image).c_str());
    }

    if ((FormatIsMultiplane(image_format) == false) && (image_tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) &&
        (image_plane_info != nullptr)) {
        const char *vuid = IsExtEnabled(device_extensions.vk_ext_image_drm_format_modifier)
                               ? "VUID-VkImageMemoryRequirementsInfo2-image-02280"
                               : "VUID-VkImageMemoryRequirementsInfo2-image-01591";
        skip |= LogError(pInfo->image, vuid,
                         "%s: %s image is a single-plane format (%s) and does not have tiling of "
                         "VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT,"
                         "but the current pNext includes a VkImagePlaneMemoryRequirementsInfo struct",
                         func_name, report_data->FormatHandle(pInfo->image).c_str(), string_VkFormat(image_format));
    }

    if (image_state->disjoint && (image_state->createInfo.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) && !image_plane_info) {
        skip |= LogError(
            pInfo->image, "VUID-VkImageMemoryRequirementsInfo2-image-02279",
            "%s: %s image was created with VK_IMAGE_CREATE_DISJOINT_BIT and has tiling of VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, "
            "but the current pNext does not include a VkImagePlaneMemoryRequirementsInfo struct",
            func_name, report_data->FormatHandle(pInfo->image).c_str());
    }

    if (image_plane_info != nullptr) {
        if ((image_tiling == VK_IMAGE_TILING_LINEAR) || (image_tiling == VK_IMAGE_TILING_OPTIMAL)) {
            // Make sure planeAspect is only a single, valid plane
            uint32_t planes = FormatPlaneCount(image_format);
            VkImageAspectFlags aspect = image_plane_info->planeAspect;
            if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                skip |= LogError(
                    pInfo->image, "VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281",
                    "%s: Image %s VkImagePlaneMemoryRequirementsInfo::planeAspect is %s but can only be VK_IMAGE_ASPECT_PLANE_0_BIT"
                    "or VK_IMAGE_ASPECT_PLANE_1_BIT.",
                    func_name, report_data->FormatHandle(image_state->image()).c_str(), string_VkImageAspectFlags(aspect).c_str());
            }
            if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                skip |= LogError(
                    pInfo->image, "VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281",
                    "%s: Image %s VkImagePlaneMemoryRequirementsInfo::planeAspect is %s but can only be VK_IMAGE_ASPECT_PLANE_0_BIT"
                    "or VK_IMAGE_ASPECT_PLANE_1_BIT or VK_IMAGE_ASPECT_PLANE_2_BIT.",
                    func_name, report_data->FormatHandle(image_state->image()).c_str(), string_VkImageAspectFlags(aspect).c_str());
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                            VkMemoryRequirements2 *pMemoryRequirements) const {
    return ValidateGetImageMemoryRequirements2(pInfo, "vkGetImageMemoryRequirements2()");
}

bool CoreChecks::PreCallValidateGetImageMemoryRequirements2KHR(VkDevice device, const VkImageMemoryRequirementsInfo2 *pInfo,
                                                               VkMemoryRequirements2 *pMemoryRequirements) const {
    return ValidateGetImageMemoryRequirements2(pInfo, "vkGetImageMemoryRequirements2KHR()");
}

bool CoreChecks::PreCallValidateMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size,
                                          VkFlags flags, void **ppData) const {
    bool skip = false;
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
    if (mem_info) {
        if ((phys_dev_mem_props.memoryTypes[mem_info->alloc_info.memoryTypeIndex].propertyFlags &
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0) {
            skip = LogError(mem, "VUID-vkMapMemory-memory-00682",
                            "Mapping Memory without VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set: %s.",
                            report_data->FormatHandle(mem).c_str());
        }

        if (mem_info->multi_instance) {
            skip = LogError(mem, "VUID-vkMapMemory-memory-00683",
                            "Memory (%s) must not have been allocated with multiple instances -- either by supplying a deviceMask "
                            "with more than one bit set, or by allocation from a heap with the MULTI_INSTANCE heap flag set.",
                            report_data->FormatHandle(mem).c_str());
        }

        skip |= ValidateMapMemRange(mem_info.get(), offset, size);
    }
    return skip;
}

bool CoreChecks::PreCallValidateUnmapMemory(VkDevice device, VkDeviceMemory mem) const {
    bool skip = false;
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);
    if (mem_info && !mem_info->mapped_range.size) {
        // Valid Usage: memory must currently be mapped
        skip |= LogError(mem, "VUID-vkUnmapMemory-memory-00689", "Unmapping Memory without memory being mapped: %s.",
                         report_data->FormatHandle(mem).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateMemoryIsMapped(const char *funcName, uint32_t memRangeCount, const VkMappedMemoryRange *pMemRanges) const {
    bool skip = false;
    for (uint32_t i = 0; i < memRangeCount; ++i) {
        auto mem_info = Get<DEVICE_MEMORY_STATE>(pMemRanges[i].memory);
        if (mem_info) {
            // Makes sure the memory is already mapped
            if (mem_info->mapped_range.size == 0) {
                skip = LogError(pMemRanges[i].memory, "VUID-VkMappedMemoryRange-memory-00684",
                                "%s: Attempting to use memory (%s) that is not currently host mapped.", funcName,
                                report_data->FormatHandle(pMemRanges[i].memory).c_str());
            }

            if (pMemRanges[i].size == VK_WHOLE_SIZE) {
                if (mem_info->mapped_range.offset > pMemRanges[i].offset) {
                    skip |= LogError(pMemRanges[i].memory, "VUID-VkMappedMemoryRange-size-00686",
                                     "%s: Flush/Invalidate offset (%zu) is less than Memory Object's offset (%zu).", funcName,
                                     static_cast<size_t>(pMemRanges[i].offset), static_cast<size_t>(mem_info->mapped_range.offset));
                }
            } else {
                const uint64_t data_end = (mem_info->mapped_range.size == VK_WHOLE_SIZE)
                                              ? mem_info->alloc_info.allocationSize
                                              : (mem_info->mapped_range.offset + mem_info->mapped_range.size);
                if ((mem_info->mapped_range.offset > pMemRanges[i].offset) ||
                    (data_end < (pMemRanges[i].offset + pMemRanges[i].size))) {
                    skip |= LogError(pMemRanges[i].memory, "VUID-VkMappedMemoryRange-size-00685",
                                     "%s: Flush/Invalidate size or offset (%zu, %zu) "
                                     "exceed the Memory Object's upper-bound (%zu).",
                                     funcName, static_cast<size_t>(pMemRanges[i].offset + pMemRanges[i].size),
                                     static_cast<size_t>(pMemRanges[i].offset), static_cast<size_t>(data_end));
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::ValidateMappedMemoryRangeDeviceLimits(const char *func_name, uint32_t mem_range_count,
                                                       const VkMappedMemoryRange *mem_ranges) const {
    bool skip = false;
    for (uint32_t i = 0; i < mem_range_count; ++i) {
        const uint64_t atom_size = phys_dev_props.limits.nonCoherentAtomSize;
        const VkDeviceSize offset = mem_ranges[i].offset;
        const VkDeviceSize size = mem_ranges[i].size;

        if (SafeModulo(offset, atom_size) != 0) {
            skip |= LogError(mem_ranges->memory, "VUID-VkMappedMemoryRange-offset-00687",
                             "%s: Offset in pMemRanges[%d] is 0x%" PRIxLEAST64
                             ", which is not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (0x%" PRIxLEAST64 ").",
                             func_name, i, offset, atom_size);
        }
        auto mem_info = Get<DEVICE_MEMORY_STATE>(mem_ranges[i].memory);
        if (mem_info) {
            const auto allocation_size = mem_info->alloc_info.allocationSize;
            if (size == VK_WHOLE_SIZE) {
                const auto mapping_offset = mem_info->mapped_range.offset;
                const auto mapping_size = mem_info->mapped_range.size;
                const auto mapping_end = ((mapping_size == VK_WHOLE_SIZE) ? allocation_size : mapping_offset + mapping_size);
                if (SafeModulo(mapping_end, atom_size) != 0 && mapping_end != allocation_size) {
                    skip |= LogError(mem_ranges->memory, "VUID-VkMappedMemoryRange-size-01389",
                                     "%s: Size in pMemRanges[%d] is VK_WHOLE_SIZE and the mapping end (0x%" PRIxLEAST64
                                     " = 0x%" PRIxLEAST64 " + 0x%" PRIxLEAST64
                                     ") not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (0x%" PRIxLEAST64
                                     ") and not equal to the end of the memory object (0x%" PRIxLEAST64 ").",
                                     func_name, i, mapping_end, mapping_offset, mapping_size, atom_size, allocation_size);
                }
            } else {
                const auto range_end = size + offset;
                if (range_end != allocation_size && SafeModulo(size, atom_size) != 0) {
                    skip |= LogError(mem_ranges->memory, "VUID-VkMappedMemoryRange-size-01390",
                                     "%s: Size in pMemRanges[%d] is 0x%" PRIxLEAST64
                                     ", which is not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (0x%" PRIxLEAST64
                                     ") and offset + size (0x%" PRIxLEAST64 " + 0x%" PRIxLEAST64 " = 0x%" PRIxLEAST64
                                     ") not equal to the memory size (0x%" PRIxLEAST64 ").",
                                     func_name, i, size, atom_size, offset, size, range_end, allocation_size);
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateFlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                        const VkMappedMemoryRange *pMemRanges) const {
    bool skip = false;
    skip |= ValidateMappedMemoryRangeDeviceLimits("vkFlushMappedMemoryRanges", memRangeCount, pMemRanges);
    skip |= ValidateMemoryIsMapped("vkFlushMappedMemoryRanges", memRangeCount, pMemRanges);
    return skip;
}

bool CoreChecks::PreCallValidateInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount,
                                                             const VkMappedMemoryRange *pMemRanges) const {
    bool skip = false;
    skip |= ValidateMappedMemoryRangeDeviceLimits("vkInvalidateMappedMemoryRanges", memRangeCount, pMemRanges);
    skip |= ValidateMemoryIsMapped("vkInvalidateMappedMemoryRanges", memRangeCount, pMemRanges);
    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory mem, VkDeviceSize *pCommittedMem) const {
    bool skip = false;
    auto mem_info = Get<DEVICE_MEMORY_STATE>(mem);

    if (mem_info) {
        if ((phys_dev_mem_props.memoryTypes[mem_info->alloc_info.memoryTypeIndex].propertyFlags &
             VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) == 0) {
            skip = LogError(mem, "VUID-vkGetDeviceMemoryCommitment-memory-00690",
                            "vkGetDeviceMemoryCommitment(): Querying commitment for memory without "
                            "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT set: %s.",
                            report_data->FormatHandle(mem).c_str());
        }
    }
    return skip;
}

bool CoreChecks::ValidateBindImageMemory(uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                         const char *api_name) const {
    bool skip = false;

    const bool bind_image_mem_2 = strcmp(api_name, "vkBindImageMemory()") != 0;
    char error_prefix[128];
    strcpy(error_prefix, api_name);

    // Track all image sub resources if they are bound for bind_image_mem_2
    // uint32_t[3] is which index in pBindInfos for max 3 planes
    // Non disjoint images act as a single plane
    vvl::unordered_map<VkImage, std::array<uint32_t, 3>> resources_bound;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        if (bind_image_mem_2 == true) {
            snprintf(error_prefix, sizeof(error_prefix), "%s pBindInfos[%u]", api_name, i);
        }

        const VkBindImageMemoryInfo &bind_info = pBindInfos[i];
        auto image_state = Get<IMAGE_STATE>(bind_info.image);
        if (image_state) {
            // Track objects tied to memory
            skip |= ValidateSetMemBinding(bind_info.memory, *image_state, error_prefix);

            const auto plane_info = LvlFindInChain<VkBindImagePlaneMemoryInfo>(bind_info.pNext);
            auto mem_info = Get<DEVICE_MEMORY_STATE>(bind_info.memory);

            if (image_state->disjoint && plane_info == nullptr) {
                skip |= LogError(bind_info.image, "VUID-VkBindImageMemoryInfo-image-07736",
                                 "%s: In order to bind planes of a disjoint image, add a VkBindImagePlaneMemoryInfo structure to "
                                 "the pNext chain of VkBindImageMemoryInfo.",
                                 error_prefix);
            }

            // Need extra check for disjoint flag incase called without bindImage2 and don't want false positive errors
            // no 'else' case as if that happens another VUID is already being triggered for it being invalid
            if ((plane_info == nullptr) && (image_state->disjoint == false)) {
                // Check non-disjoint images VkMemoryRequirements

                // All validation using the image_state->requirements for external AHB is check in android only section
                if (image_state->IsExternalAHB() == false) {
                    const VkMemoryRequirements &mem_req = image_state->requirements[0];

                    // Validate memory requirements alignment
                    if (SafeModulo(bind_info.memoryOffset, mem_req.alignment) != 0) {
                        const char *validation_error;
                        if (bind_image_mem_2 == false) {
                            validation_error = "VUID-vkBindImageMemory-memoryOffset-01048";
                        } else if (IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                            validation_error = "VUID-VkBindImageMemoryInfo-pNext-01616";
                        } else {
                            validation_error = "VUID-VkBindImageMemoryInfo-memoryOffset-01613";
                        }
                        skip |=
                            LogError(bind_info.image, validation_error,
                                     "%s: memoryOffset is 0x%" PRIxLEAST64
                                     " but must be an integer multiple of the VkMemoryRequirements::alignment value 0x%" PRIxLEAST64
                                     ", returned from a call to vkGetImageMemoryRequirements with image.",
                                     error_prefix, bind_info.memoryOffset, mem_req.alignment);
                    }

                    if (mem_info) {
                        safe_VkMemoryAllocateInfo alloc_info = mem_info->alloc_info;
                        // Validate memory requirements size
                        if (mem_req.size > alloc_info.allocationSize - bind_info.memoryOffset) {
                            const char *validation_error;
                            if (bind_image_mem_2 == false) {
                                validation_error = "VUID-vkBindImageMemory-size-01049";
                            } else if (IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                                validation_error = "VUID-VkBindImageMemoryInfo-pNext-01617";
                            } else {
                                validation_error = "VUID-VkBindImageMemoryInfo-memory-01614";
                            }
                            skip |= LogError(bind_info.image, validation_error,
                                             "%s: memory size minus memoryOffset is 0x%" PRIxLEAST64
                                             " but must be at least as large as VkMemoryRequirements::size value 0x%" PRIxLEAST64
                                             ", returned from a call to vkGetImageMemoryRequirements with image.",
                                             error_prefix, alloc_info.allocationSize - bind_info.memoryOffset, mem_req.size);
                        }

                        // Validate memory type used
                        {
                            const char *validation_error;
                            if (bind_image_mem_2 == false) {
                                validation_error = "VUID-vkBindImageMemory-memory-01047";
                            } else if (IsExtEnabled(device_extensions.vk_khr_sampler_ycbcr_conversion)) {
                                validation_error = "VUID-VkBindImageMemoryInfo-pNext-01615";
                            } else {
                                validation_error = "VUID-VkBindImageMemoryInfo-memory-01612";
                            }
                            skip |= ValidateMemoryTypes(mem_info.get(), mem_req.memoryTypeBits, error_prefix, validation_error);
                        }
                    }
                }

                if (bind_image_mem_2 == true) {
                    // since its a non-disjoint image, finding VkImage in map is a duplicate
                    auto it = resources_bound.find(image_state->image());
                    if (it == resources_bound.end()) {
                        std::array<uint32_t, 3> bound_index = {i, vvl::kU32Max, vvl::kU32Max};
                        resources_bound.emplace(image_state->image(), bound_index);
                    } else {
                        skip |= LogError(
                            bind_info.image, "VUID-vkBindImageMemory2-pBindInfos-04006",
                            "%s: The same non-disjoint image resource is being bound twice at pBindInfos[%d] and pBindInfos[%d]",
                            error_prefix, it->second[0], i);
                    }
                }
            } else if ((plane_info != nullptr) && (image_state->disjoint == true)) {
                // Check disjoint images VkMemoryRequirements for given plane
                int plane = 0;

                // All validation using the image_state->plane*_requirements for external AHB is check in android only section
                if (image_state->IsExternalAHB() == false) {
                    const VkImageAspectFlagBits aspect = plane_info->planeAspect;
                    switch (aspect) {
                        case VK_IMAGE_ASPECT_PLANE_0_BIT:
                            plane = 0;
                            break;
                        case VK_IMAGE_ASPECT_PLANE_1_BIT:
                            plane = 1;
                            break;
                        case VK_IMAGE_ASPECT_PLANE_2_BIT:
                            plane = 2;
                            break;
                        default:
                            assert(false);  // parameter validation should have caught this
                            break;
                    }
                    const VkMemoryRequirements &disjoint_mem_req = image_state->requirements[plane];

                    // Validate memory requirements alignment
                    if (SafeModulo(bind_info.memoryOffset, disjoint_mem_req.alignment) != 0) {
                        skip |= LogError(
                            bind_info.image, "VUID-VkBindImageMemoryInfo-pNext-01620",
                            "%s: memoryOffset is 0x%" PRIxLEAST64
                            " but must be an integer multiple of the VkMemoryRequirements::alignment value 0x%" PRIxLEAST64
                            ", returned from a call to vkGetImageMemoryRequirements2 with disjoint image for aspect plane %s.",
                            error_prefix, bind_info.memoryOffset, disjoint_mem_req.alignment, string_VkImageAspectFlagBits(aspect));
                    }

                    if (mem_info) {
                        safe_VkMemoryAllocateInfo alloc_info = mem_info->alloc_info;

                        // Validate memory requirements size
                        if (disjoint_mem_req.size > alloc_info.allocationSize - bind_info.memoryOffset) {
                            skip |= LogError(
                                bind_info.image, "VUID-VkBindImageMemoryInfo-pNext-01621",
                                "%s: memory size minus memoryOffset is 0x%" PRIxLEAST64
                                " but must be at least as large as VkMemoryRequirements::size value 0x%" PRIxLEAST64
                                ", returned from a call to vkGetImageMemoryRequirements with disjoint image for aspect plane %s.",
                                error_prefix, alloc_info.allocationSize - bind_info.memoryOffset, disjoint_mem_req.size,
                                string_VkImageAspectFlagBits(aspect));
                        }

                        // Validate memory type used
                        {
                            skip |= ValidateMemoryTypes(mem_info.get(), disjoint_mem_req.memoryTypeBits, error_prefix,
                                                        "VUID-VkBindImageMemoryInfo-pNext-01619");
                        }
                    }
                }

                auto it = resources_bound.find(image_state->image());
                if (it == resources_bound.end()) {
                    std::array<uint32_t, 3> bound_index = {vvl::kU32Max, vvl::kU32Max, vvl::kU32Max};
                    bound_index[plane] = i;
                    resources_bound.emplace(image_state->image(), bound_index);
                } else {
                    if (it->second[plane] == vvl::kU32Max) {
                        it->second[plane] = i;
                    } else {
                        skip |= LogError(bind_info.image, "VUID-vkBindImageMemory2-pBindInfos-04006",
                                         "%s: The same disjoint image sub-resource for plane %d is being bound twice at "
                                         "pBindInfos[%d] and pBindInfos[%d]",
                                         error_prefix, plane, it->second[plane], i);
                    }
                }
            }

            if (mem_info) {
                // Validate bound memory range information
                // if memory is exported to an AHB then the mem_info->allocationSize must be zero and this check is not needed
                if ((mem_info->IsExport() == false) ||
                    ((mem_info->export_handle_type_flags & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) ==
                     0)) {
                    skip |= ValidateInsertImageMemoryRange(bind_info.image, mem_info.get(), bind_info.memoryOffset, error_prefix);
                }

                // Validate dedicated allocation
                if (mem_info->IsDedicatedImage()) {
                    if (enabled_features.dedicated_allocation_image_aliasing_features.dedicatedAllocationImageAliasing) {
                        auto current_image_state = Get<IMAGE_STATE>(bind_info.image);
                        if ((bind_info.memoryOffset != 0) || !current_image_state ||
                            !current_image_state->IsCreateInfoDedicatedAllocationImageAliasingCompatible(
                                mem_info->dedicated->create_info.image)) {
                            const char *validation_error;
                            if (bind_image_mem_2 == false) {
                                validation_error = "VUID-vkBindImageMemory-memory-02629";
                            } else {
                                validation_error = "VUID-VkBindImageMemoryInfo-memory-02629";
                            }
                            const LogObjectList objlist(bind_info.image, bind_info.memory, mem_info->dedicated->handle);
                            skip |= LogError(
                                objlist, validation_error,
                                "%s: for dedicated memory allocation %s, VkMemoryDedicatedAllocateInfo:: %s must compatible "
                                "with %s and memoryOffset 0x%" PRIxLEAST64 " must be zero.",
                                error_prefix, report_data->FormatHandle(bind_info.memory).c_str(),
                                report_data->FormatHandle(mem_info->dedicated->handle).c_str(),
                                report_data->FormatHandle(bind_info.image).c_str(), bind_info.memoryOffset);
                        }
                    } else {
                        if ((bind_info.memoryOffset != 0) || (mem_info->dedicated->handle.Cast<VkImage>() != bind_info.image)) {
                            const char *validation_error;
                            if (bind_image_mem_2 == false) {
                                validation_error = "VUID-vkBindImageMemory-memory-01509";
                            } else {
                                validation_error = "VUID-VkBindImageMemoryInfo-memory-01509";
                            }
                            const LogObjectList objlist(bind_info.image, bind_info.memory, mem_info->dedicated->handle);
                            skip |=
                                LogError(objlist, validation_error,
                                         "%s: for dedicated memory allocation %s, VkMemoryDedicatedAllocateInfo:: %s must be equal "
                                         "to %s and memoryOffset 0x%" PRIxLEAST64 " must be zero.",
                                         error_prefix, report_data->FormatHandle(bind_info.memory).c_str(),
                                         report_data->FormatHandle(mem_info->dedicated->handle).c_str(),
                                         report_data->FormatHandle(bind_info.image).c_str(), bind_info.memoryOffset);
                        }
                    }
                }

                auto chained_flags_struct = LvlFindInChain<VkMemoryAllocateFlagsInfo>(mem_info->alloc_info.pNext);
                if (enabled_features.descriptor_buffer_features.descriptorBufferCaptureReplay &&
                    (image_state->createInfo.flags & VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
                    (!chained_flags_struct || !(chained_flags_struct->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT))) {
                    const char *vuid = bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-descriptorBufferCaptureReplay-08113"
                                                        : "VUID-vkBindImageMemory-descriptorBufferCaptureReplay-08113";
                    skip |= LogError(
                        bind_info.image, vuid,
                        "%s: If image was created with the VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT bit set, "
                        "memory must have been allocated with the VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT bit set.",
                        api_name);
                }

                // Validate export memory handles
                if (mem_info->export_handle_type_flags != 0) {
                    auto external_info = LvlInitStruct<VkPhysicalDeviceExternalImageFormatInfo>();
                    auto image_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&external_info);
                    image_info.format = image_state->createInfo.format;
                    image_info.type = image_state->createInfo.imageType;
                    image_info.tiling = image_state->createInfo.tiling;
                    image_info.usage = image_state->createInfo.usage;
                    image_info.flags = image_state->createInfo.flags;
                    auto external_properties = LvlInitStruct<VkExternalImageFormatProperties>();
                    auto image_properties = LvlInitStruct<VkImageFormatProperties2>(&external_properties);
                    bool export_supported = true;

                    // Check export operation support
                    auto check_export_support = [&](VkExternalMemoryHandleTypeFlagBits flag) {
                        external_info.handleType = flag;
                        auto result =
                            DispatchGetPhysicalDeviceImageFormatProperties2(physical_device, &image_info, &image_properties);
                        if (result != VK_SUCCESS) {
                            export_supported = false;
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError(
                                objlist, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656",
                                "%s: The handle type (%s) specified by the memory's VkExportMemoryAllocateInfo, and format (%s), "
                                "type (%s), tiling (%s), usage (%s), flags (%s) specified by the image's VkImageCreateInfo is not "
                                "supported combination of parameters. vkGetPhysicalDeviceImageFormatProperties2 returned back %s.",
                                api_name, string_VkExternalMemoryHandleTypeFlagBits(flag), string_VkFormat(image_info.format),
                                string_VkImageType(image_info.type), string_VkImageTiling(image_info.tiling),
                                string_VkImageUsageFlags(image_info.usage).c_str(),
                                string_VkImageCreateFlags(image_info.flags).c_str(), string_VkResult(result));
                        } else if ((external_properties.externalMemoryProperties.externalMemoryFeatures &
                                    VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) == 0) {
                            export_supported = false;
                            const LogObjectList objlist(bind_info.image, bind_info.memory);
                            skip |= LogError(objlist, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656",
                                             "%s: The VkDeviceMemory (%s) has VkExportMemoryAllocateInfo::handleTypes with the %s "
                                             "flag set, which does not support VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT with the "
                                             "image create format (%s), type (%s), tiling (%s), usage (%s), flags (%s).",
                                             api_name, report_data->FormatHandle(bind_info.memory).c_str(),
                                             string_VkExternalMemoryHandleTypeFlagBits(flag), string_VkFormat(image_info.format),
                                             string_VkImageType(image_info.type), string_VkImageTiling(image_info.tiling),
                                             string_VkImageUsageFlags(image_info.usage).c_str(),
                                             string_VkImageCreateFlags(image_info.flags).c_str());
                        }
                    };
                    IterateFlags<VkExternalMemoryHandleTypeFlagBits>(mem_info->export_handle_type_flags, check_export_support);

                    // The types of external memory handles must be compatible
                    const auto compatible_types = external_properties.externalMemoryProperties.compatibleHandleTypes;
                    if (export_supported &&
                        (mem_info->export_handle_type_flags & compatible_types) != mem_info->export_handle_type_flags) {
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |=
                            LogError(objlist, "VUID-VkExportMemoryAllocateInfo-handleTypes-00656",
                                     "%s: The VkDeviceMemory (%s) has VkExportMemoryAllocateInfo::handleTypes (%s) that are not "
                                     "reported as compatible by vkGetPhysicalDeviceImageFormatProperties2 with the image create "
                                     "format (%s), type (%s), tiling (%s), usage (%s), flags (%s).",
                                     api_name, report_data->FormatHandle(bind_info.memory).c_str(),
                                     string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_type_flags).c_str(),
                                     string_VkFormat(image_info.format), string_VkImageType(image_info.type),
                                     string_VkImageTiling(image_info.tiling), string_VkImageUsageFlags(image_info.usage).c_str(),
                                     string_VkImageCreateFlags(image_info.flags).c_str());
                    }

                    // Check if the memory meets the image's external memory requirements
                    if ((mem_info->export_handle_type_flags & image_state->external_memory_handle) == 0) {
                        const char *vuid =
                            bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-memory-02728" : "VUID-vkBindImageMemory-memory-02728";
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |=
                            LogError(objlist, vuid,
                                     "%s: The VkDeviceMemory (%s) has an external handleType of %s which does not include at least "
                                     "one handle from VkImage (%s) handleType %s.",
                                     error_prefix, report_data->FormatHandle(bind_info.memory).c_str(),
                                     string_VkExternalMemoryHandleTypeFlags(mem_info->export_handle_type_flags).c_str(),
                                     report_data->FormatHandle(bind_info.image).c_str(),
                                     string_VkExternalMemoryHandleTypeFlags(image_state->external_memory_handle).c_str());
                    }
                }

                // Validate import memory handles
                if (mem_info->IsImportAHB() == true) {
                    skip |= ValidateImageImportedHandleANDROID(api_name, image_state->external_memory_handle, bind_info.memory,
                                                               bind_info.image);
                } else if (mem_info->IsImport() == true) {
                    if ((mem_info->import_handle_type_flags & image_state->external_memory_handle) == 0) {
                        const char *vuid = nullptr;
                        if ((bind_image_mem_2) &&
                            IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                            vuid = "VUID-VkBindImageMemoryInfo-memory-02989";
                        } else if ((!bind_image_mem_2) &&
                                   IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                            vuid = "VUID-vkBindImageMemory-memory-02989";
                        } else if ((bind_image_mem_2) &&
                                   !IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                            vuid = "VUID-VkBindImageMemoryInfo-memory-02729";
                        } else if ((!bind_image_mem_2) &&
                                   !IsExtEnabled(device_extensions.vk_android_external_memory_android_hardware_buffer)) {
                            vuid = "VUID-vkBindImageMemory-memory-02729";
                        }
                        const LogObjectList objlist(bind_info.image, bind_info.memory);
                        skip |= LogError(objlist, vuid,
                                         "%s: The VkDeviceMemory (%s) was created with an import operation with handleType of %s "
                                         "which is not set in the VkImage (%s) VkExternalMemoryImageCreateInfo::handleType (%s)",
                                         api_name, report_data->FormatHandle(bind_info.memory).c_str(),
                                         string_VkExternalMemoryHandleTypeFlags(mem_info->import_handle_type_flags).c_str(),
                                         report_data->FormatHandle(bind_info.image).c_str(),
                                         string_VkExternalMemoryHandleTypeFlags(image_state->external_memory_handle).c_str());
                    }
                }

                // Validate mix of protected buffer and memory
                if ((image_state->unprotected == false) && (mem_info->unprotected == true)) {
                    const char *vuid =
                        bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-None-01901" : "VUID-vkBindImageMemory-None-01901";
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(objlist, vuid,
                                     "%s: The VkDeviceMemory (%s) was not created with protected memory but the VkImage (%s) was "
                                     "set to use protected memory.",
                                     api_name, report_data->FormatHandle(bind_info.memory).c_str(),
                                     report_data->FormatHandle(bind_info.image).c_str());
                } else if ((image_state->unprotected == true) && (mem_info->unprotected == false)) {
                    const char *vuid =
                        bind_image_mem_2 ? "VUID-VkBindImageMemoryInfo-None-01902" : "VUID-vkBindImageMemory-None-01902";
                    const LogObjectList objlist(bind_info.image, bind_info.memory);
                    skip |= LogError(objlist, vuid,
                                     "%s: The VkDeviceMemory (%s) was created with protected memory but the VkImage (%s) was not "
                                     "set to use protected memory.",
                                     api_name, report_data->FormatHandle(bind_info.memory).c_str(),
                                     report_data->FormatHandle(bind_info.image).c_str());
                }
            }

            const auto swapchain_info = LvlFindInChain<VkBindImageMemorySwapchainInfoKHR>(bind_info.pNext);
            if (swapchain_info) {
                if (bind_info.memory != VK_NULL_HANDLE) {
                    skip |= LogError(bind_info.image, "VUID-VkBindImageMemoryInfo-pNext-01631", "%s: %s is not VK_NULL_HANDLE.",
                                     error_prefix, report_data->FormatHandle(bind_info.memory).c_str());
                }
                if (image_state->create_from_swapchain != swapchain_info->swapchain) {
                    const LogObjectList objlist(image_state->image(), image_state->create_from_swapchain,
                                                swapchain_info->swapchain);
                    skip |= LogError(
                        objlist, kVUID_Core_BindImageMemory_Swapchain,
                        "%s: %s is created by %s, but the image is bound by %s. The image should be created and bound by the same "
                        "swapchain",
                        error_prefix, report_data->FormatHandle(image_state->image()).c_str(),
                        report_data->FormatHandle(image_state->create_from_swapchain).c_str(),
                        report_data->FormatHandle(swapchain_info->swapchain).c_str());
                }
                auto swapchain_state = Get<SWAPCHAIN_NODE>(swapchain_info->swapchain);
                if (swapchain_state) {
                    if (swapchain_state->images.size() <= swapchain_info->imageIndex) {
                        skip |= LogError(bind_info.image, "VUID-VkBindImageMemorySwapchainInfoKHR-imageIndex-01644",
                                         "%s: imageIndex (%" PRIu32 ") is out of bounds of %s images (size: %zu)", error_prefix,
                                         swapchain_info->imageIndex, report_data->FormatHandle(swapchain_info->swapchain).c_str(),
                                         swapchain_state->images.size());
                    }
                    if (IsExtEnabled(device_extensions.vk_ext_swapchain_maintenance1) &&
                        (swapchain_state->createInfo.flags & VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT)) {
                        if (swapchain_state->images[swapchain_info->imageIndex].acquired == false) {
                            skip |= LogError(
                                bind_info.image, "VUID-VkBindImageMemorySwapchainInfoKHR-swapchain-07756",
                                "%s: The swapchain was created with VK_SWAPCHAIN_CREATE_DEFERRED_MEMORY_ALLOCATION_BIT_EXT but "
                                "imageIndex (%" PRIu32 ") has not been acquired",
                                error_prefix, swapchain_info->imageIndex);
                        }
                    }
                }
            } else {
                if (image_state->create_from_swapchain) {
                    skip |= LogError(bind_info.image, "VUID-VkBindImageMemoryInfo-image-01630",
                                     "%s: pNext of VkBindImageMemoryInfo doesn't include VkBindImageMemorySwapchainInfoKHR.",
                                     error_prefix);
                }
                if (!mem_info) {
                    skip |= LogError(bind_info.image, "VUID-VkBindImageMemoryInfo-pNext-01632", "%s: %s is invalid.", error_prefix,
                                     report_data->FormatHandle(bind_info.memory).c_str());
                }
            }

            const auto bind_image_memory_device_group_info = LvlFindInChain<VkBindImageMemoryDeviceGroupInfo>(bind_info.pNext);
            if (bind_image_memory_device_group_info && bind_image_memory_device_group_info->splitInstanceBindRegionCount != 0) {
                if (!(image_state->createInfo.flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT)) {
                    skip |= LogError(bind_info.image, "VUID-VkBindImageMemoryInfo-pNext-01627",
                                     "%s: pNext of VkBindImageMemoryInfo contains VkBindImageMemoryDeviceGroupInfo with "
                                     "splitInstanceBindRegionCount (%" PRIi32
                                     ") not equal to 0 and %s is not created with "
                                     "VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT.",
                                     error_prefix, bind_image_memory_device_group_info->splitInstanceBindRegionCount,
                                     report_data->FormatHandle(image_state->image()).c_str());
                }
                uint32_t phy_dev_square = 1;
                if (device_group_create_info.physicalDeviceCount > 0) {
                    phy_dev_square = device_group_create_info.physicalDeviceCount * device_group_create_info.physicalDeviceCount;
                }
                if (bind_image_memory_device_group_info->splitInstanceBindRegionCount != phy_dev_square) {
                    skip |= LogError(
                        bind_info.image, "VUID-VkBindImageMemoryDeviceGroupInfo-splitInstanceBindRegionCount-01636",
                        "%s: pNext of VkBindImageMemoryInfo contains VkBindImageMemoryDeviceGroupInfo with "
                        "splitInstanceBindRegionCount (%" PRIi32
                        ") which is not 0 and different from the number of physical devices in the logical device squared (%" PRIu32
                        ").",
                        error_prefix, bind_image_memory_device_group_info->splitInstanceBindRegionCount, phy_dev_square);
                }
            }

            if (plane_info) {
                // Checks for disjoint bit in image
                if (image_state->disjoint == false) {
                    skip |= LogError(
                        bind_info.image, "VUID-VkBindImageMemoryInfo-pNext-01618",
                        "%s: pNext of VkBindImageMemoryInfo contains VkBindImagePlaneMemoryInfo and %s is not created with "
                        "VK_IMAGE_CREATE_DISJOINT_BIT.",
                        error_prefix, report_data->FormatHandle(image_state->image()).c_str());
                }

                // Make sure planeAspect is only a single, valid plane
                uint32_t planes = FormatPlaneCount(image_state->createInfo.format);
                VkImageAspectFlags aspect = plane_info->planeAspect;
                if ((2 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
                    skip |= LogError(
                        bind_info.image, "VUID-VkBindImagePlaneMemoryInfo-planeAspect-02283",
                        "%s: Image %s VkBindImagePlaneMemoryInfo::planeAspect is %s but can only be VK_IMAGE_ASPECT_PLANE_0_BIT"
                        "or VK_IMAGE_ASPECT_PLANE_1_BIT.",
                        error_prefix, report_data->FormatHandle(image_state->image()).c_str(),
                        string_VkImageAspectFlags(aspect).c_str());
                }
                if ((3 == planes) && (aspect != VK_IMAGE_ASPECT_PLANE_0_BIT) && (aspect != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
                    (aspect != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
                    skip |= LogError(
                        bind_info.image, "VUID-VkBindImagePlaneMemoryInfo-planeAspect-02283",
                        "%s: Image %s VkBindImagePlaneMemoryInfo::planeAspect is %s but can only be VK_IMAGE_ASPECT_PLANE_0_BIT"
                        "or VK_IMAGE_ASPECT_PLANE_1_BIT or VK_IMAGE_ASPECT_PLANE_2_BIT.",
                        error_prefix, report_data->FormatHandle(image_state->image()).c_str(),
                        string_VkImageAspectFlags(aspect).c_str());
                }
            }
        }

        const auto bind_image_memory_device_group = LvlFindInChain<VkBindImageMemoryDeviceGroupInfo>(bind_info.pNext);
        if (bind_image_memory_device_group) {
            if (bind_image_memory_device_group->deviceIndexCount > 0 &&
                bind_image_memory_device_group->splitInstanceBindRegionCount > 0) {
                skip |= LogError(bind_info.image, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01633",
                                 "%s: VkBindImageMemoryDeviceGroupInfo in pNext of pBindInfos[%" PRIu32
                                 "] has both deviceIndexCount and splitInstanceBindRegionCount greater than 0.",
                                 error_prefix, i);
            }
            if (bind_image_memory_device_group->deviceIndexCount != 0 &&
                bind_image_memory_device_group->deviceIndexCount != device_group_create_info.physicalDeviceCount &&
                device_group_create_info.physicalDeviceCount > 0) {
                skip |= LogError(bind_info.image, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01634",
                                 "%s: The number of physical devices in the logical device is %" PRIu32
                                 ", but VkBindImageMemoryDeviceGroupInfo::deviceIndexCount is %" PRIu32 ".",
                                 api_name, device_group_create_info.physicalDeviceCount,
                                 bind_image_memory_device_group->deviceIndexCount);
            }
        }
    }

    // Check to make sure all disjoint planes were bound
    for (auto &resource : resources_bound) {
        auto image_state = Get<IMAGE_STATE>(resource.first);
        if (image_state->disjoint == true) {
            uint32_t total_planes = FormatPlaneCount(image_state->createInfo.format);
            for (uint32_t i = 0; i < total_planes; i++) {
                if (resource.second[i] == vvl::kU32Max) {
                    skip |= LogError(resource.first, "VUID-vkBindImageMemory2-pBindInfos-02858",
                                     "%s: Plane %u of the disjoint image was not bound. All %d planes need to bound individually "
                                     "in separate pBindInfos in a single call.",
                                     api_name, i, total_planes);
                }
            }
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem,
                                                VkDeviceSize memoryOffset) const {
    bool skip = false;
    auto image_state = Get<IMAGE_STATE>(image);
    if (image_state) {
        // Checks for no disjoint bit
        if (image_state->disjoint == true) {
            skip |=
                LogError(image, "VUID-vkBindImageMemory-image-01608",
                         "%s must not have been created with the VK_IMAGE_CREATE_DISJOINT_BIT (need to use vkBindImageMemory2).",
                         report_data->FormatHandle(image).c_str());
        }
    }

    auto bind_info = LvlInitStruct<VkBindImageMemoryInfo>();
    bind_info.image = image;
    bind_info.memory = mem;
    bind_info.memoryOffset = memoryOffset;
    skip |= ValidateBindImageMemory(1, &bind_info, "vkBindImageMemory()");
    return skip;
}

void CoreChecks::PostCallRecordBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memoryOffset,
                                               VkResult result) {
    if (VK_SUCCESS != result) return;

    StateTracker::PostCallRecordBindImageMemory(device, image, mem, memoryOffset, result);
    auto image_state = Get<IMAGE_STATE>(image);
    if (image_state) {
        image_state->SetInitialLayoutMap();
    }
}

bool CoreChecks::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                 const VkBindImageMemoryInfo *pBindInfos) const {
    return ValidateBindImageMemory(bindInfoCount, pBindInfos, "vkBindImageMemory2()");
}

void CoreChecks::PostCallRecordBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                VkResult result) {
    if (VK_SUCCESS != result) return;
    StateTracker::PostCallRecordBindImageMemory2(device, bindInfoCount, pBindInfos, result);

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        auto image_state = Get<IMAGE_STATE>(pBindInfos[i].image);
        if (image_state) {
            image_state->SetInitialLayoutMap();
        }
    }
}

bool CoreChecks::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo *pBindInfos) const {
    return ValidateBindImageMemory(bindInfoCount, pBindInfos, "vkBindImageMemory2KHR()");
}

void CoreChecks::PostCallRecordBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo *pBindInfos,
                                                   VkResult result) {
    if (VK_SUCCESS != result) return;
    StateTracker::PostCallRecordBindImageMemory2KHR(device, bindInfoCount, pBindInfos, result);
    for (uint32_t i = 0; i < bindInfoCount; i++) {
        auto image_state = Get<IMAGE_STATE>(pBindInfos[i].image);
        if (image_state) {
            image_state->SetInitialLayoutMap();
        }
    }
}

bool CoreChecks::ValidateSparseMemoryBind(const VkSparseMemoryBind &bind, VkDeviceSize resource_size, const char *func_name,
                                          const char *parameter_name) const {
    bool skip = false;
    auto mem_info = Get<DEVICE_MEMORY_STATE>(bind.memory);
    if (mem_info) {
        if (phys_dev_mem_props.memoryTypes[mem_info->alloc_info.memoryTypeIndex].propertyFlags &
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) {
            skip |= LogError(bind.memory, "VUID-VkSparseMemoryBind-memory-01097",
                             "%s: %s memory type has VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT bit set.", func_name, parameter_name);
        }

        if (bind.memoryOffset >= mem_info->alloc_info.allocationSize) {
            skip |= LogError(bind.memory, "VUID-VkSparseMemoryBind-memoryOffset-01101",
                             "%s: %s memoryOffset (%" PRIu64 ") must be less than the size of memory (%" PRIu64 ")", func_name,
                             parameter_name, bind.memoryOffset, mem_info->alloc_info.allocationSize);
        }

        if ((mem_info->alloc_info.allocationSize - bind.memoryOffset) < bind.size) {
            skip |= LogError(bind.memory, "VUID-VkSparseMemoryBind-size-01102",
                             "%s: %s size (%" PRIu64 ") must be less than or equal to the size of memory (%" PRIu64
                             ") minus memoryOffset (%" PRIu64 ").",
                             func_name, parameter_name, bind.size, mem_info->alloc_info.allocationSize, bind.memoryOffset);
        }
    }

    if (bind.size <= 0) {
        skip |= LogError(bind.memory, "VUID-VkSparseMemoryBind-size-01098", "%s: %s size (%" PRIu64 ") must be greater than 0.",
                         func_name, parameter_name, bind.size);
    }

    if (resource_size <= bind.resourceOffset) {
        skip |= LogError(bind.memory, "VUID-VkSparseMemoryBind-resourceOffset-01099",
                         "%s: %s resourceOffset (%" PRIu64 ") must be less than the size of the resource (%" PRIu64 ").", func_name,
                         parameter_name, bind.resourceOffset, resource_size);
    }

    if ((resource_size - bind.resourceOffset) < bind.size) {
        skip |= LogError(bind.memory, "VUID-VkSparseMemoryBind-size-01100",
                         "%s: %s size (%" PRIu64 ") must be less than or equal to the size of the resource (%" PRIu64
                         ") minus resourceOffset (%" PRIu64 ").",
                         func_name, parameter_name, bind.size, resource_size, bind.resourceOffset);
    }

    return skip;
}

bool CoreChecks::ValidateImageSubresourceSparseImageMemoryBind(IMAGE_STATE const &image_state,
                                                               VkImageSubresource const &subresource, uint32_t image_idx,
                                                               uint32_t bind_idx) const {
    bool skip =
        ValidateImageAspectMask(image_state.image(), image_state.createInfo.format, subresource.aspectMask, image_state.disjoint,
                                "vkQueueSparseBind()", "VUID-VkSparseImageMemoryBind-subresource-01106");

    if (subresource.mipLevel >= image_state.createInfo.mipLevels) {
        skip |=
            LogError(image_state.Handle(), "VUID-VkSparseImageMemoryBind-subresource-01106",
                     "vkQueueBindSparse(): pBindInfo[%" PRIu32 "].pImageBinds[%" PRIu32 "].subresource.mipLevel (%" PRIu32
                     ") is not less than mipLevels (%" PRIu32 ") of image pBindInfo[%" PRIu32 "].pImageBinds[%" PRIu32 "].image.",
                     bind_idx, image_idx, subresource.mipLevel, image_state.createInfo.mipLevels, bind_idx, image_idx);
    }

    if (subresource.arrayLayer >= image_state.createInfo.arrayLayers) {
        skip |=
            LogError(image_state.Handle(), "VUID-VkSparseImageMemoryBind-subresource-01106",
                     "vkQueueBindSparse(): pBindInfo[%" PRIu32 "].pImageBinds[%" PRIu32 "].subresource.arrayLayer (%" PRIu32
                     ") is not less than arrayLayers (%" PRIu32 ") of image pBindInfo[%" PRIu32 "].pImageBinds[%" PRIu32 "].image.",
                     bind_idx, image_idx, subresource.arrayLayer, image_state.createInfo.arrayLayers, bind_idx, image_idx);
    }

    return skip;
}

// This will only be called after we are sure the image was created with VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT
bool CoreChecks::ValidateSparseImageMemoryBind(IMAGE_STATE const *image_state, VkSparseImageMemoryBind const &bind,
                                               uint32_t image_idx, uint32_t bind_idx) const {
    bool skip = false;

    auto const mem_info = Get<DEVICE_MEMORY_STATE>(bind.memory);
    if (mem_info) {
        // TODO: The closest one should be VUID-VkSparseImageMemoryBind-memory-01105 instead of the mentioned
        // one. We also need to check memory_bind.memory
        if (bind.memoryOffset >= mem_info->alloc_info.allocationSize) {
            skip |= LogError(bind.memory, "VUID-VkSparseMemoryBind-memoryOffset-01101",
                             "vkQueueBindSparse(): pBindInfo[%" PRIu32 "].pImageBinds[%" PRIu32 "]: memoryOffset (%" PRIu64
                             ") is not less than the size (%" PRIu64 ") of memory",
                             bind_idx, image_idx, bind.memoryOffset, mem_info->alloc_info.allocationSize);
        }
    }

    if (image_state) {
        skip |= ValidateImageSubresourceSparseImageMemoryBind(*image_state, bind.subresource, image_idx, bind_idx);

        for (auto const &requirements : image_state->sparse_requirements) {
            VkExtent3D const &granularity = requirements.formatProperties.imageGranularity;
            if (SafeModulo(bind.offset.x, granularity.width) != 0) {
                skip |= LogError(image_state->Handle(), "VUID-VkSparseImageMemoryBind-offset-01107",
                                 "vkQueueBindSparse(): pImageBinds[%" PRIu32 "].pBindInfo[%" PRIu32 "]: offset.x (%" PRIi32
                                 ") must be a multiple of the sparse image block width "
                                 "(VkSparseImageFormatProperties::imageGranularity.width (%" PRIu32 ")) of the image",
                                 bind_idx, image_idx, bind.offset.x, granularity.width);
            }

            if (SafeModulo(bind.offset.y, granularity.height) != 0) {
                skip |= LogError(image_state->Handle(), "VUID-VkSparseImageMemoryBind-offset-01109",
                                 "vkQueueBindSparse(): pImageBinds[%" PRIu32 "].pBindInfo[%" PRIu32 "]: offset.x (%" PRIi32
                                 ") must be a multiple of the sparse image block height "
                                 "(VkSparseImageFormatProperties::imageGranularity.height (%" PRIu32 ")) of the image",
                                 bind_idx, image_idx, bind.offset.y, granularity.height);
            }

            if (SafeModulo(bind.offset.z, granularity.depth) != 0) {
                skip |= LogError(image_state->Handle(), "VUID-VkSparseImageMemoryBind-offset-01111",
                                 "vkQueueBindSparse(): pImageBinds[%" PRIu32 "].pBindInfo[%" PRIu32 "]: offset.z (%" PRIi32
                                 ") must be a multiple of the sparse image block depth "
                                 "(VkSparseImageFormatProperties::imageGranularity.depth (%" PRIu32 ")) of the image",
                                 bind_idx, image_idx, bind.offset.z, granularity.depth);
            }

            VkExtent3D const subresource_extent =
                image_state->GetSubresourceExtent(bind.subresource.aspectMask, bind.subresource.mipLevel);
            if ((SafeModulo(bind.extent.width, granularity.width) != 0) &&
                ((bind.extent.width + bind.offset.x) != subresource_extent.width)) {
                skip |= LogError(image_state->Handle(), "VUID-VkSparseImageMemoryBind-extent-01108",
                                 "vkQueueBindSparse(): pImageBinds[%" PRIu32 "].pBindInfo[%" PRIu32 "]: extent.width (%" PRIu32
                                 ") must either be a multiple of the sparse image block width "
                                 "(VkSparseImageFormatProperties::imageGranularity.width (%" PRIu32
                                 ")) of the image, or else (extent.width + offset.x) (%" PRIu32
                                 ") must equal the width of the image subresource (%" PRIu32 ")",
                                 bind_idx, image_idx, bind.extent.width, granularity.width, bind.extent.width + bind.offset.x,
                                 subresource_extent.width);
            }

            if ((SafeModulo(bind.extent.height, granularity.height) != 0) &&
                ((bind.extent.height + bind.offset.y) != subresource_extent.height)) {
                skip |= LogError(image_state->Handle(), "VUID-VkSparseImageMemoryBind-extent-01110",
                                 "vkQueueBindSparse(): pImageBinds[%" PRIu32 "].pBindInfo[%" PRIu32 "]: extent.height (%" PRIu32
                                 ") must either be a multiple of the sparse image block height "
                                 "(VkSparseImageFormatProperties::imageGranularity.height (%" PRIu32
                                 ")) of the image, or else (extent.height + offset.y) (%" PRIu32
                                 ") must equal the height of the image subresource (%" PRIu32 ")",
                                 bind_idx, image_idx, bind.extent.height, granularity.height, bind.extent.height + bind.offset.y,
                                 subresource_extent.height);
            }

            if ((SafeModulo(bind.extent.depth, granularity.depth) != 0) &&
                ((bind.extent.depth + bind.offset.z) != subresource_extent.depth)) {
                skip |= LogError(image_state->Handle(), "VUID-VkSparseImageMemoryBind-extent-01112",
                                 "vkQueueBindSparse(): pImageBinds[%" PRIu32 "].pBindInfo[%" PRIu32 "]: extent.depth (%" PRIu32
                                 ") must either be a multiple of the sparse image block depth "
                                 "(VkSparseImageFormatProperties::imageGranularity.depth (%" PRIu32
                                 ")) of the image, or else (extent.depth + offset.z) (%" PRIu32
                                 ") must equal the depth of the image subresource (%" PRIu32 ")",
                                 bind_idx, image_idx, bind.extent.depth, granularity.depth, bind.extent.depth + bind.offset.z,
                                 subresource_extent.depth);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                const char *apiName) const {
    bool skip = false;

    if (!enabled_features.core12.bufferDeviceAddress && !enabled_features.buffer_device_address_ext_features.bufferDeviceAddress) {
        skip |= LogError(pInfo->buffer, "VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324",
                         "%s: The bufferDeviceAddress feature must: be enabled.", apiName);
    }

    if (physical_device_count > 1 && !enabled_features.core12.bufferDeviceAddressMultiDevice &&
        !enabled_features.buffer_device_address_ext_features.bufferDeviceAddressMultiDevice) {
        skip |= LogError(pInfo->buffer, "VUID-vkGetBufferDeviceAddress-device-03325",
                         "%s: If device was created with multiple physical devices, then the "
                         "bufferDeviceAddressMultiDevice feature must: be enabled.",
                         apiName);
    }

    auto buffer_state = Get<BUFFER_STATE>(pInfo->buffer);
    if (buffer_state) {
        if (!(buffer_state->createInfo.flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
            skip |= ValidateMemoryIsBoundToBuffer(device, *buffer_state, apiName, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
        }

        skip |= ValidateBufferUsageFlags(device, *buffer_state, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, true,
                                         "VUID-VkBufferDeviceAddressInfo-buffer-02601", apiName,
                                         "VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT");
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddressEXT(VkDevice device, const VkBufferDeviceAddressInfo *pInfo) const {
    return ValidateGetBufferDeviceAddress(device, static_cast<const VkBufferDeviceAddressInfo *>(pInfo),
                                          "vkGetBufferDeviceAddressEXT");
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo) const {
    return ValidateGetBufferDeviceAddress(device, static_cast<const VkBufferDeviceAddressInfo *>(pInfo),
                                          "vkGetBufferDeviceAddressKHR");
}

bool CoreChecks::PreCallValidateGetBufferDeviceAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo) const {
    return ValidateGetBufferDeviceAddress(device, static_cast<const VkBufferDeviceAddressInfo *>(pInfo),
                                          "vkGetBufferDeviceAddress");
}

bool CoreChecks::ValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo,
                                                       const char *apiName) const {
    bool skip = false;

    if (!enabled_features.core12.bufferDeviceAddress) {
        skip |= LogError(pInfo->buffer, "VUID-vkGetBufferOpaqueCaptureAddress-None-03326",
                         "%s(): The bufferDeviceAddress feature must: be enabled.", apiName);
    }

    if (physical_device_count > 1 && !enabled_features.core12.bufferDeviceAddressMultiDevice) {
        skip |= LogError(pInfo->buffer, "VUID-vkGetBufferOpaqueCaptureAddress-device-03327",
                         "%s(): If device was created with multiple physical devices, then the "
                         "bufferDeviceAddressMultiDevice feature must: be enabled.",
                         apiName);
    }
    return skip;
}

bool CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddressKHR(VkDevice device, const VkBufferDeviceAddressInfo *pInfo) const {
    return ValidateGetBufferOpaqueCaptureAddress(device, static_cast<const VkBufferDeviceAddressInfo *>(pInfo),
                                                 "vkGetBufferOpaqueCaptureAddressKHR");
}

bool CoreChecks::PreCallValidateGetBufferOpaqueCaptureAddress(VkDevice device, const VkBufferDeviceAddressInfo *pInfo) const {
    return ValidateGetBufferOpaqueCaptureAddress(device, static_cast<const VkBufferDeviceAddressInfo *>(pInfo),
                                                 "vkGetBufferOpaqueCaptureAddress");
}

bool CoreChecks::ValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo,
                                                             const char *apiName) const {
    bool skip = false;

    if (!enabled_features.core12.bufferDeviceAddress) {
        skip |= LogError(pInfo->memory, "VUID-vkGetDeviceMemoryOpaqueCaptureAddress-None-03334",
                         "%s(): The bufferDeviceAddress feature must: be enabled.", apiName);
    }

    if (physical_device_count > 1 && !enabled_features.core12.bufferDeviceAddressMultiDevice) {
        skip |= LogError(pInfo->memory, "VUID-vkGetDeviceMemoryOpaqueCaptureAddress-device-03335",
                         "%s(): If device was created with multiple physical devices, then the "
                         "bufferDeviceAddressMultiDevice feature must: be enabled.",
                         apiName);
    }

    auto mem_info = Get<DEVICE_MEMORY_STATE>(pInfo->memory);
    if (mem_info) {
        auto chained_flags_struct = LvlFindInChain<VkMemoryAllocateFlagsInfo>(mem_info->alloc_info.pNext);
        if (!chained_flags_struct || !(chained_flags_struct->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)) {
            skip |= LogError(pInfo->memory, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336",
                             "%s(): memory must have been allocated with VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT.", apiName);
        }
    }

    return skip;
}

bool CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddressKHR(VkDevice device,
                                                                       const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo) const {
    return ValidateGetDeviceMemoryOpaqueCaptureAddress(device, static_cast<const VkDeviceMemoryOpaqueCaptureAddressInfo *>(pInfo),
                                                       "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
}

bool CoreChecks::PreCallValidateGetDeviceMemoryOpaqueCaptureAddress(VkDevice device,
                                                                    const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo) const {
    return ValidateGetDeviceMemoryOpaqueCaptureAddress(device, static_cast<const VkDeviceMemoryOpaqueCaptureAddressInfo *>(pInfo),
                                                       "vkGetDeviceMemoryOpaqueCaptureAddress");
}
