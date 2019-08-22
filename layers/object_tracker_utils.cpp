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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Tobin Ehlis <tobin@lunarg.com>
 */

#include "chassis.h"

#include "object_lifetime_validation.h"

uint64_t object_track_index = 0;

VulkanTypedHandle ObjTrackStateTypedHandle(const ObjTrackState &track_state) {
    // TODO: Unify Typed Handle representation (i.e. VulkanTypedHandle everywhere there are handle/type pairs)
    VulkanTypedHandle typed_handle;
    typed_handle.handle = track_state.handle;
    typed_handle.type = track_state.object_type;
    return typed_handle;
}

// Destroy memRef lists and free all memory
void ObjectLifetimes::DestroyQueueDataStructures(VkDevice device) {
    // Destroy the items in the queue map
    auto snapshot = object_map[kVulkanObjectTypeQueue].snapshot();
    for (const auto &queue : snapshot) {
        uint32_t obj_index = queue.second->object_type;
        assert(num_total_objects > 0);
        num_total_objects--;
        assert(num_objects[obj_index] > 0);
        num_objects[obj_index]--;
        object_map[kVulkanObjectTypeQueue].erase(queue.first);
    }
}

// Look for this device object in any of the instance child devices lists.
// NOTE: This is of dubious value. In most circumstances Vulkan will die a flaming death if a dispatchable object is invalid.
// However, if this layer is loaded first and GetProcAddress is used to make API calls, it will detect bad DOs.
bool ObjectLifetimes::ValidateDeviceObject(const VulkanTypedHandle &device_typed, const char *invalid_handle_code,
                                           const char *wrong_device_code) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    auto instance_object_lifetime_data = GetObjectLifetimeData(instance_data->object_dispatch);
    if (instance_object_lifetime_data->object_map[kVulkanObjectTypeDevice].contains(device_typed.handle)) {
        return false;
    }
    return log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, device_typed.handle,
                   invalid_handle_code, "Invalid %s.", report_data->FormatHandle(device_typed).c_str());
}

void ObjectLifetimes::AllocateCommandBuffer(VkDevice device, const VkCommandPool command_pool, const VkCommandBuffer command_buffer,
                                            VkCommandBufferLevel level) {
    auto pNewObjNode = std::make_shared<ObjTrackState>();
    pNewObjNode->object_type = kVulkanObjectTypeCommandBuffer;
    pNewObjNode->handle = HandleToUint64(command_buffer);
    pNewObjNode->parent_object = HandleToUint64(command_pool);
    if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        pNewObjNode->status = OBJSTATUS_COMMAND_BUFFER_SECONDARY;
    } else {
        pNewObjNode->status = OBJSTATUS_NONE;
    }
    InsertObject(object_map[kVulkanObjectTypeCommandBuffer], HandleToUint64(command_buffer), kVulkanObjectTypeCommandBuffer,
                 pNewObjNode);
    num_objects[kVulkanObjectTypeCommandBuffer]++;
    num_total_objects++;
}

bool ObjectLifetimes::ValidateCommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer) {
    bool skip = false;
    uint64_t object_handle = HandleToUint64(command_buffer);
    auto iter = object_map[kVulkanObjectTypeCommandBuffer].find(object_handle);
    if (iter != object_map[kVulkanObjectTypeCommandBuffer].end()) {
        auto pNode = iter->second;

        if (pNode->parent_object != HandleToUint64(command_pool)) {
            // We know that the parent *must* be a command pool
            const auto parent_pool = CastFromUint64<VkCommandPool>(pNode->parent_object);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            object_handle, "VUID-vkFreeCommandBuffers-pCommandBuffers-parent",
                            "FreeCommandBuffers is attempting to free %s belonging to %s from %s).",
                            report_data->FormatHandle(command_buffer).c_str(), report_data->FormatHandle(parent_pool).c_str(),
                            report_data->FormatHandle(command_pool).c_str());
        }
    } else {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, object_handle,
                        "VUID-vkFreeCommandBuffers-pCommandBuffers-00048", "Invalid %s.",
                        report_data->FormatHandle(command_buffer).c_str());
    }
    return skip;
}

void ObjectLifetimes::AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) {
    auto pNewObjNode = std::make_shared<ObjTrackState>();
    pNewObjNode->object_type = kVulkanObjectTypeDescriptorSet;
    pNewObjNode->status = OBJSTATUS_NONE;
    pNewObjNode->handle = HandleToUint64(descriptor_set);
    pNewObjNode->parent_object = HandleToUint64(descriptor_pool);
    InsertObject(object_map[kVulkanObjectTypeDescriptorSet], HandleToUint64(descriptor_set), kVulkanObjectTypeDescriptorSet,
                 pNewObjNode);
    num_objects[kVulkanObjectTypeDescriptorSet]++;
    num_total_objects++;

    auto itr = object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptor_pool));
    if (itr != object_map[kVulkanObjectTypeDescriptorPool].end()) {
        itr->second->child_objects->insert(HandleToUint64(descriptor_set));
    }
}

bool ObjectLifetimes::ValidateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) {
    bool skip = false;
    uint64_t object_handle = HandleToUint64(descriptor_set);
    auto dsItem = object_map[kVulkanObjectTypeDescriptorSet].find(object_handle);
    if (dsItem != object_map[kVulkanObjectTypeDescriptorSet].end()) {
        if (dsItem->second->parent_object != HandleToUint64(descriptor_pool)) {
            // We know that the parent *must* be a descriptor pool
            const auto parent_pool = CastFromUint64<VkDescriptorPool>(dsItem->second->parent_object);
            skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                            object_handle, "VUID-vkFreeDescriptorSets-pDescriptorSets-parent",
                            "FreeDescriptorSets is attempting to free %s"
                            " belonging to %s from %s).",
                            report_data->FormatHandle(descriptor_set).c_str(), report_data->FormatHandle(parent_pool).c_str(),
                            report_data->FormatHandle(descriptor_pool).c_str());
        }
    } else {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, object_handle,
                        "VUID-vkFreeDescriptorSets-pDescriptorSets-00310", "Invalid %s.",
                        report_data->FormatHandle(descriptor_set).c_str());
    }
    return skip;
}

template <typename DispObj>
bool ObjectLifetimes::ValidateDescriptorWrite(DispObj disp, VkWriteDescriptorSet const *desc, bool isPush) {
    bool skip = false;

    if (!isPush && desc->dstSet) {
        skip |= ValidateObject(disp, desc->dstSet, kVulkanObjectTypeDescriptorSet, false, "VUID-VkWriteDescriptorSet-dstSet-00320",
                               "VUID-VkWriteDescriptorSet-commonparent");
    }

    if ((desc->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
        for (uint32_t idx2 = 0; idx2 < desc->descriptorCount; ++idx2) {
            skip |= ValidateObject(disp, desc->pTexelBufferView[idx2], kVulkanObjectTypeBufferView, false,
                                   "VUID-VkWriteDescriptorSet-descriptorType-00323", "VUID-VkWriteDescriptorSet-commonparent");
        }
    }

    if ((desc->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        for (uint32_t idx3 = 0; idx3 < desc->descriptorCount; ++idx3) {
            skip |= ValidateObject(disp, desc->pImageInfo[idx3].imageView, kVulkanObjectTypeImageView, false,
                                   "VUID-VkWriteDescriptorSet-descriptorType-00326", "VUID-VkDescriptorImageInfo-commonparent");
        }
    }

    if ((desc->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
        for (uint32_t idx4 = 0; idx4 < desc->descriptorCount; ++idx4) {
            if (desc->pBufferInfo[idx4].buffer) {
                skip |= ValidateObject(disp, desc->pBufferInfo[idx4].buffer, kVulkanObjectTypeBuffer, false,
                                       "VUID-VkDescriptorBufferInfo-buffer-parameter", kVUIDUndefined);
            }
        }
    }

    return skip;
}

bool ObjectLifetimes::PreCallValidateCmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                             VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                             const VkWriteDescriptorSet *pDescriptorWrites) {
    bool skip = false;
    skip |= ValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false,
                           "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-parameter", "VUID-vkCmdPushDescriptorSetKHR-commonparent");
    skip |= ValidateObject(commandBuffer, layout, kVulkanObjectTypePipelineLayout, false,
                           "VUID-vkCmdPushDescriptorSetKHR-layout-parameter", "VUID-vkCmdPushDescriptorSetKHR-commonparent");
    if (pDescriptorWrites) {
        for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
            skip |= ValidateDescriptorWrite(commandBuffer, &pDescriptorWrites[index0], true);
        }
    }
    return skip;
}

void ObjectLifetimes::CreateQueue(VkDevice device, VkQueue vkObj) {
    std::shared_ptr<ObjTrackState> p_obj_node = NULL;
    auto queue_item = object_map[kVulkanObjectTypeQueue].find(HandleToUint64(vkObj));
    if (queue_item == object_map[kVulkanObjectTypeQueue].end()) {
        p_obj_node = std::make_shared<ObjTrackState>();
        InsertObject(object_map[kVulkanObjectTypeQueue], HandleToUint64(vkObj), kVulkanObjectTypeQueue, p_obj_node);
        num_objects[kVulkanObjectTypeQueue]++;
        num_total_objects++;
    } else {
        p_obj_node = queue_item->second;
    }
    p_obj_node->object_type = kVulkanObjectTypeQueue;
    p_obj_node->status = OBJSTATUS_NONE;
    p_obj_node->handle = HandleToUint64(vkObj);
}

void ObjectLifetimes::CreateSwapchainImageObject(VkDevice dispatchable_object, VkImage swapchain_image, VkSwapchainKHR swapchain) {
    if (!swapchainImageMap.contains(HandleToUint64(swapchain_image))) {
        auto pNewObjNode = std::make_shared<ObjTrackState>();
        pNewObjNode->object_type = kVulkanObjectTypeImage;
        pNewObjNode->status = OBJSTATUS_NONE;
        pNewObjNode->handle = HandleToUint64(swapchain_image);
        pNewObjNode->parent_object = HandleToUint64(swapchain);
        InsertObject(swapchainImageMap, HandleToUint64(swapchain_image), kVulkanObjectTypeImage, pNewObjNode);
    }
}

bool ObjectLifetimes::DeviceReportUndestroyedObjects(VkDevice device, VulkanObjectType object_type, const std::string &error_code) {
    bool skip = false;

    auto snapshot = object_map[object_type].snapshot();
    for (const auto &item : snapshot) {
        const auto object_info = item.second;
        skip |= log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, get_debug_report_enum[object_type], object_info->handle,
                        error_code, "OBJ ERROR : For %s, %s has not been destroyed.", report_data->FormatHandle(device).c_str(),
                        report_data->FormatHandle(ObjTrackStateTypedHandle(*object_info)).c_str());
    }
    return skip;
}

void ObjectLifetimes::DeviceDestroyUndestroyedObjects(VkDevice device, VulkanObjectType object_type) {
    auto snapshot = object_map[object_type].snapshot();
    for (const auto &item : snapshot) {
        auto object_info = item.second;
        DestroyObjectSilently(object_info->handle, object_type);
    }
}

bool ObjectLifetimes::PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    bool skip = false;

    // We validate here for coverage, though we'd not have made it this for with a bad instance.
    skip |= ValidateObject(instance, instance, kVulkanObjectTypeInstance, true, "VUID-vkDestroyInstance-instance-parameter",
                           kVUIDUndefined);

    // Validate that child devices have been destroyed
    auto snapshot = object_map[kVulkanObjectTypeDevice].snapshot();
    for (const auto &iit : snapshot) {
        auto pNode = iit.second;

        VkDevice device = reinterpret_cast<VkDevice>(pNode->handle);
        VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[pNode->object_type];

        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, pNode->handle, kVUID_ObjectTracker_ObjectLeak,
                    "OBJ ERROR : %s object %s has not been destroyed.", string_VkDebugReportObjectTypeEXT(debug_object_type),
                    report_data->FormatHandle(ObjTrackStateTypedHandle(*pNode)).c_str());

        // Report any remaining objects in LL
        skip |= ReportUndestroyedObjects(device, "VUID-vkDestroyInstance-instance-00629");

        skip |= ValidateDestroyObject(instance, device, kVulkanObjectTypeDevice, pAllocator,
                                      "VUID-vkDestroyInstance-instance-00630", "VUID-vkDestroyInstance-instance-00631");
    }

    ValidateDestroyObject(instance, instance, kVulkanObjectTypeInstance, pAllocator, "VUID-vkDestroyInstance-instance-00630",
                          "VUID-vkDestroyInstance-instance-00631");

    return skip;
}

bool ObjectLifetimes::PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                              VkPhysicalDevice *pPhysicalDevices) {
    bool skip = ValidateObject(instance, instance, kVulkanObjectTypeInstance, false,
                               "VUID-vkEnumeratePhysicalDevices-instance-parameter", kVUIDUndefined);
    return skip;
}

void ObjectLifetimes::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                             VkPhysicalDevice *pPhysicalDevices, VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pPhysicalDevices) {
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
            CreateObject(instance, pPhysicalDevices[i], kVulkanObjectTypePhysicalDevice, nullptr);
        }
    }
}

void ObjectLifetimes::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    // Destroy physical devices
    auto snapshot = object_map[kVulkanObjectTypePhysicalDevice].snapshot();
    for (const auto &iit : snapshot) {
        auto pNode = iit.second;
        VkPhysicalDevice physical_device = reinterpret_cast<VkPhysicalDevice>(pNode->handle);
        RecordDestroyObject(instance, physical_device, kVulkanObjectTypePhysicalDevice);
    }

    // Destroy child devices
    auto snapshot2 = object_map[kVulkanObjectTypeDevice].snapshot();
    for (const auto &iit : snapshot2) {
        auto pNode = iit.second;
        VkDevice device = reinterpret_cast<VkDevice>(pNode->handle);
        DestroyUndestroyedObjects(device);

        RecordDestroyObject(instance, device, kVulkanObjectTypeDevice);
    }
}

void ObjectLifetimes::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    RecordDestroyObject(instance, instance, kVulkanObjectTypeInstance);
}

bool ObjectLifetimes::PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, true, "VUID-vkDestroyDevice-device-parameter", kVUIDUndefined);
    skip |= ValidateDestroyObject(physical_device, device, kVulkanObjectTypeDevice, pAllocator, "VUID-vkDestroyDevice-device-00379",
                                  "VUID-vkDestroyDevice-device-00380");
    // Report any remaining objects associated with this VkDevice object in LL
    skip |= ReportUndestroyedObjects(device, "VUID-vkDestroyDevice-device-00378");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physical_device), layer_data_map);
    ValidationObject *validation_data = GetValidationObject(instance_data->object_dispatch, LayerObjectTypeObjectTracker);
    ObjectLifetimes *object_lifetimes = static_cast<ObjectLifetimes *>(validation_data);
    object_lifetimes->RecordDestroyObject(physical_device, device, kVulkanObjectTypeDevice);
    DestroyUndestroyedObjects(device);

    // Clean up Queue's MemRef Linked Lists
    DestroyQueueDataStructures(device);
}

bool ObjectLifetimes::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                                    VkQueue *pQueue) {
    bool skip = false;
    skip |=
        ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceQueue-device-parameter", kVUIDUndefined);
    return skip;
}

void ObjectLifetimes::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                                   VkQueue *pQueue) {
    auto lock = write_shared_lock();
    CreateQueue(device, *pQueue);
}

bool ObjectLifetimes::PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    return ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceQueue2-device-parameter",
                          kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    auto lock = write_shared_lock();
    CreateQueue(device, *pQueue);
}

bool ObjectLifetimes::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                          const VkWriteDescriptorSet *pDescriptorWrites,
                                                          uint32_t descriptorCopyCount,
                                                          const VkCopyDescriptorSet *pDescriptorCopies) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkUpdateDescriptorSets-device-parameter",
                           kVUIDUndefined);
    if (pDescriptorCopies) {
        for (uint32_t idx0 = 0; idx0 < descriptorCopyCount; ++idx0) {
            if (pDescriptorCopies[idx0].dstSet) {
                skip |= ValidateObject(device, pDescriptorCopies[idx0].dstSet, kVulkanObjectTypeDescriptorSet, false,
                                       "VUID-VkCopyDescriptorSet-dstSet-parameter", "VUID-VkCopyDescriptorSet-commonparent");
            }
            if (pDescriptorCopies[idx0].srcSet) {
                skip |= ValidateObject(device, pDescriptorCopies[idx0].srcSet, kVulkanObjectTypeDescriptorSet, false,
                                       "VUID-VkCopyDescriptorSet-srcSet-parameter", "VUID-VkCopyDescriptorSet-commonparent");
            }
        }
    }
    if (pDescriptorWrites) {
        for (uint32_t idx1 = 0; idx1 < descriptorWriteCount; ++idx1) {
            skip |= ValidateDescriptorWrite(device, &pDescriptorWrites[idx1], false);
        }
    }
    return skip;
}

bool ObjectLifetimes::PreCallValidateResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                         VkDescriptorPoolResetFlags flags) {
    bool skip = false;
    auto lock = read_shared_lock();

    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkResetDescriptorPool-device-parameter",
                           kVUIDUndefined);
    skip |=
        ValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                       "VUID-vkResetDescriptorPool-descriptorPool-parameter", "VUID-vkResetDescriptorPool-descriptorPool-parent");

    auto itr = object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pPoolNode = itr->second;
        for (auto set : *pPoolNode->child_objects) {
            skip |= ValidateDestroyObject(device, (VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                                          kVUIDUndefined);
        }
    }
    return skip;
}

void ObjectLifetimes::PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                       VkDescriptorPoolResetFlags flags) {
    auto lock = write_shared_lock();
    // A DescriptorPool's descriptor sets are implicitly deleted when the pool is reset. Remove this pool's descriptor sets from
    // our descriptorSet map.
    auto itr = object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pPoolNode = itr->second;
        for (auto set : *pPoolNode->child_objects) {
            RecordDestroyObject(device, (VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet);
        }
        pPoolNode->child_objects->clear();
    }
}

bool ObjectLifetimes::PreCallValidateBeginCommandBuffer(VkCommandBuffer command_buffer,
                                                        const VkCommandBufferBeginInfo *begin_info) {
    bool skip = false;
    skip |= ValidateObject(command_buffer, command_buffer, kVulkanObjectTypeCommandBuffer, false,
                           "VUID-vkBeginCommandBuffer-commandBuffer-parameter", kVUIDUndefined);
    if (begin_info) {
        auto iter = object_map[kVulkanObjectTypeCommandBuffer].find(HandleToUint64(command_buffer));
        if (iter != object_map[kVulkanObjectTypeCommandBuffer].end()) {
            auto pNode = iter->second;
            if ((begin_info->pInheritanceInfo) && (pNode->status & OBJSTATUS_COMMAND_BUFFER_SECONDARY) &&
                (begin_info->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
                skip |=
                    ValidateObject(command_buffer, begin_info->pInheritanceInfo->framebuffer, kVulkanObjectTypeFramebuffer, true,
                                   "VUID-VkCommandBufferBeginInfo-flags-00055", "VUID-VkCommandBufferInheritanceInfo-commonparent");
                skip |=
                    ValidateObject(command_buffer, begin_info->pInheritanceInfo->renderPass, kVulkanObjectTypeRenderPass, false,
                                   "VUID-VkCommandBufferBeginInfo-flags-00053", "VUID-VkCommandBufferInheritanceInfo-commonparent");
            }
        }
    }
    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                           uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetSwapchainImagesKHR-device-parameter",
                           "VUID-vkGetSwapchainImagesKHR-commonparent");
    skip |= ValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false,
                           "VUID-vkGetSwapchainImagesKHR-swapchain-parameter", "VUID-vkGetSwapchainImagesKHR-commonparent");
    return skip;
}

void ObjectLifetimes::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                          VkImage *pSwapchainImages, VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    auto lock = write_shared_lock();
    if (pSwapchainImages != NULL) {
        for (uint32_t i = 0; i < *pSwapchainImageCount; i++) {
            CreateSwapchainImageObject(device, pSwapchainImages[i], swapchain);
        }
    }
}

bool ObjectLifetimes::PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator,
                                                               VkDescriptorSetLayout *pSetLayout) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateDescriptorSetLayout-device-parameter",
                           kVUIDUndefined);
    if (pCreateInfo) {
        if (pCreateInfo->pBindings) {
            for (uint32_t binding_index = 0; binding_index < pCreateInfo->bindingCount; ++binding_index) {
                const VkDescriptorSetLayoutBinding &binding = pCreateInfo->pBindings[binding_index];
                const bool is_sampler_type = binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                                             binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                if (binding.pImmutableSamplers && is_sampler_type) {
                    for (uint32_t index2 = 0; index2 < binding.descriptorCount; ++index2) {
                        const VkSampler sampler = binding.pImmutableSamplers[index2];
                        skip |= ValidateObject(device, sampler, kVulkanObjectTypeSampler, false,
                                               "VUID-VkDescriptorSetLayoutBinding-descriptorType-00282", kVUIDUndefined);
                    }
                }
            }
        }
    }
    return skip;
}

void ObjectLifetimes::PostCallRecordCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDescriptorSetLayout *pSetLayout, VkResult result) {
    if (result != VK_SUCCESS) return;
    CreateObject(device, *pSetLayout, kVulkanObjectTypeDescriptorSetLayout, pAllocator);
}

bool ObjectLifetimes::ValidateSamplerObjects(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo) {
    bool skip = false;
    if (pCreateInfo->pBindings) {
        for (uint32_t index1 = 0; index1 < pCreateInfo->bindingCount; ++index1) {
            for (uint32_t index2 = 0; index2 < pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                if (pCreateInfo->pBindings[index1].pImmutableSamplers) {
                    skip |=
                        ValidateObject(device, pCreateInfo->pBindings[index1].pImmutableSamplers[index2], kVulkanObjectTypeSampler,
                                       true, "VUID-VkDescriptorSetLayoutBinding-descriptorType-00282", kVUIDUndefined);
                }
            }
        }
    }
    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device,
                                                                   const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                   VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = ValidateObject(device, device, kVulkanObjectTypeDevice, false,
                               "VUID-vkGetDescriptorSetLayoutSupport-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= ValidateSamplerObjects(device, pCreateInfo);
    }
    return skip;
}
bool ObjectLifetimes::PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                      const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                      VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = ValidateObject(device, device, kVulkanObjectTypeDevice, false,
                               "VUID-vkGetDescriptorSetLayoutSupportKHR-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= ValidateSamplerObjects(device, pCreateInfo);
    }
    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                            uint32_t *pQueueFamilyPropertyCount,
                                                                            VkQueueFamilyProperties *pQueueFamilyProperties) {
    return ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                          "VUID-vkGetPhysicalDeviceQueueFamilyProperties-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pQueueFamilyPropertyCount,
                                                                           VkQueueFamilyProperties *pQueueFamilyProperties) {}

void ObjectLifetimes::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                                   VkInstance *pInstance, VkResult result) {
    if (result != VK_SUCCESS) return;
    CreateObject(*pInstance, *pInstance, kVulkanObjectTypeInstance, pAllocator);
}

bool ObjectLifetimes::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                            VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAllocateCommandBuffers-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, pAllocateInfo->commandPool, kVulkanObjectTypeCommandPool, false,
                           "VUID-VkCommandBufferAllocateInfo-commandPool-parameter", kVUIDUndefined);
    return skip;
}

void ObjectLifetimes::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                           VkCommandBuffer *pCommandBuffers, VkResult result) {
    if (result != VK_SUCCESS) return;
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
        AllocateCommandBuffer(device, pAllocateInfo->commandPool, pCommandBuffers[i], pAllocateInfo->level);
    }
}

bool ObjectLifetimes::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                            VkDescriptorSet *pDescriptorSets) {
    bool skip = false;
    auto lock = read_shared_lock();
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAllocateDescriptorSets-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, pAllocateInfo->descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                           "VUID-VkDescriptorSetAllocateInfo-descriptorPool-parameter",
                           "VUID-VkDescriptorSetAllocateInfo-commonparent");
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        skip |= ValidateObject(device, pAllocateInfo->pSetLayouts[i], kVulkanObjectTypeDescriptorSetLayout, false,
                               "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-parameter",
                               "VUID-VkDescriptorSetAllocateInfo-commonparent");
    }
    return skip;
}

void ObjectLifetimes::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                           VkDescriptorSet *pDescriptorSets, VkResult result) {
    if (result != VK_SUCCESS) return;
    auto lock = write_shared_lock();
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        AllocateDescriptorSet(device, pAllocateInfo->descriptorPool, pDescriptorSets[i]);
    }
}

bool ObjectLifetimes::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                        const VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFreeCommandBuffers-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, false,
                           "VUID-vkFreeCommandBuffers-commandPool-parameter", "VUID-vkFreeCommandBuffers-commandPool-parent");
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        if (pCommandBuffers[i] != VK_NULL_HANDLE) {
            skip |= ValidateCommandBuffer(device, commandPool, pCommandBuffers[i]);
            skip |= ValidateDestroyObject(device, pCommandBuffers[i], kVulkanObjectTypeCommandBuffer, nullptr, kVUIDUndefined,
                                          kVUIDUndefined);
        }
    }
    return skip;
}

void ObjectLifetimes::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers) {
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        RecordDestroyObject(device, pCommandBuffers[i], kVulkanObjectTypeCommandBuffer);
    }
}

bool ObjectLifetimes::PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                         const VkAllocationCallbacks *pAllocator) {
    return ValidateDestroyObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, pAllocator,
                                 "VUID-vkDestroySwapchainKHR-swapchain-01283", "VUID-vkDestroySwapchainKHR-swapchain-01284");
}

void ObjectLifetimes::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                       const VkAllocationCallbacks *pAllocator) {
    RecordDestroyObject(device, swapchain, kVulkanObjectTypeSwapchainKHR);

    auto snapshot = swapchainImageMap.snapshot(
        [swapchain](std::shared_ptr<ObjTrackState> pNode) { return pNode->parent_object == HandleToUint64(swapchain); });
    for (const auto &itr : snapshot) {
        swapchainImageMap.erase(itr.first);
    }
}

bool ObjectLifetimes::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                                        uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets) {
    auto lock = read_shared_lock();
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFreeDescriptorSets-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                           "VUID-vkFreeDescriptorSets-descriptorPool-parameter", "VUID-vkFreeDescriptorSets-descriptorPool-parent");
    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateDescriptorSet(device, descriptorPool, pDescriptorSets[i]);
            skip |= ValidateDestroyObject(device, pDescriptorSets[i], kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                                          kVUIDUndefined);
        }
    }
    return skip;
}
void ObjectLifetimes::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                      const VkDescriptorSet *pDescriptorSets) {
    auto lock = write_shared_lock();
    std::shared_ptr<ObjTrackState> pPoolNode = nullptr;
    auto itr = object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != object_map[kVulkanObjectTypeDescriptorPool].end()) {
        pPoolNode = itr->second;
    }
    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        RecordDestroyObject(device, pDescriptorSets[i], kVulkanObjectTypeDescriptorSet);
        if (pPoolNode) {
            pPoolNode->child_objects->erase(HandleToUint64(pDescriptorSets[i]));
        }
    }
}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                           const VkAllocationCallbacks *pAllocator) {
    auto lock = read_shared_lock();
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyDescriptorPool-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, true,
                           "VUID-vkDestroyDescriptorPool-descriptorPool-parameter",
                           "VUID-vkDestroyDescriptorPool-descriptorPool-parent");

    auto itr = object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pPoolNode = itr->second;
        for (auto set : *pPoolNode->child_objects) {
            skip |= ValidateDestroyObject(device, (VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                                          kVUIDUndefined);
        }
    }
    skip |= ValidateDestroyObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, pAllocator,
                                  "VUID-vkDestroyDescriptorPool-descriptorPool-00304",
                                  "VUID-vkDestroyDescriptorPool-descriptorPool-00305");
    return skip;
}
void ObjectLifetimes::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                         const VkAllocationCallbacks *pAllocator) {
    auto lock = write_shared_lock();
    auto itr = object_map[kVulkanObjectTypeDescriptorPool].find(HandleToUint64(descriptorPool));
    if (itr != object_map[kVulkanObjectTypeDescriptorPool].end()) {
        auto pPoolNode = itr->second;
        for (auto set : *pPoolNode->child_objects) {
            RecordDestroyObject(device, (VkDescriptorSet)set, kVulkanObjectTypeDescriptorSet);
        }
        pPoolNode->child_objects->clear();
    }
    RecordDestroyObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool);
}

bool ObjectLifetimes::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                        const VkAllocationCallbacks *pAllocator) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyCommandPool-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, true,
                           "VUID-vkDestroyCommandPool-commandPool-parameter", "VUID-vkDestroyCommandPool-commandPool-parent");

    auto snapshot = object_map[kVulkanObjectTypeCommandBuffer].snapshot(
        [commandPool](std::shared_ptr<ObjTrackState> pNode) { return pNode->parent_object == HandleToUint64(commandPool); });
    for (const auto &itr : snapshot) {
        auto pNode = itr.second;
        skip |= ValidateCommandBuffer(device, commandPool, reinterpret_cast<VkCommandBuffer>(itr.first));
        skip |= ValidateDestroyObject(device, reinterpret_cast<VkCommandBuffer>(itr.first), kVulkanObjectTypeCommandBuffer, nullptr,
                                      kVUIDUndefined, kVUIDUndefined);
    }
    skip |= ValidateDestroyObject(device, commandPool, kVulkanObjectTypeCommandPool, pAllocator,
                                  "VUID-vkDestroyCommandPool-commandPool-00042", "VUID-vkDestroyCommandPool-commandPool-00043");
    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                      const VkAllocationCallbacks *pAllocator) {
    auto snapshot = object_map[kVulkanObjectTypeCommandBuffer].snapshot(
        [commandPool](std::shared_ptr<ObjTrackState> pNode) { return pNode->parent_object == HandleToUint64(commandPool); });
    // A CommandPool's cmd buffers are implicitly deleted when pool is deleted. Remove this pool's cmdBuffers from cmd buffer map.
    for (const auto &itr : snapshot) {
        RecordDestroyObject(device, reinterpret_cast<VkCommandBuffer>(itr.first), kVulkanObjectTypeCommandBuffer);
    }
    RecordDestroyObject(device, commandPool, kVulkanObjectTypeCommandPool);
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                             uint32_t *pQueueFamilyPropertyCount,
                                                                             VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    return ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                          "VUID-vkGetPhysicalDeviceQueueFamilyProperties2-physicalDevice-parameter", kVUIDUndefined);
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                uint32_t *pQueueFamilyPropertyCount,
                                                                                VkQueueFamilyProperties2 *pQueueFamilyProperties) {
    return ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                          "VUID-vkGetPhysicalDeviceQueueFamilyProperties2-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                            uint32_t *pQueueFamilyPropertyCount,
                                                                            VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pPropertyCount,
                                                                           VkDisplayPropertiesKHR *pProperties) {
    return ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                          "VUID-vkGetPhysicalDeviceDisplayPropertiesKHR-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                          VkDisplayPropertiesKHR *pProperties, VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObject(physicalDevice, pProperties[i].display, kVulkanObjectTypeDisplayKHR, nullptr);
        }
    }
}

bool ObjectLifetimes::PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                 uint32_t *pPropertyCount,
                                                                 VkDisplayModePropertiesKHR *pProperties) {
    bool skip = false;
    skip |= ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                           "VUID-vkGetDisplayModePropertiesKHR-physicalDevice-parameter", kVUIDUndefined);
    skip |= ValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false,
                           "VUID-vkGetDisplayModePropertiesKHR-display-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties,
                                                                VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObject(physicalDevice, pProperties[i].displayMode, kVulkanObjectTypeDisplayModeKHR, nullptr);
        }
    }
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t *pPropertyCount,
                                                                            VkDisplayProperties2KHR *pProperties) {
    return ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                          "VUID-vkGetPhysicalDeviceDisplayProperties2KHR-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pPropertyCount,
                                                                           VkDisplayProperties2KHR *pProperties, VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    for (uint32_t index = 0; index < *pPropertyCount; ++index) {
        CreateObject(physicalDevice, pProperties[index].displayProperties.display, kVulkanObjectTypeDisplayKHR, nullptr);
    }
}

bool ObjectLifetimes::PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                  uint32_t *pPropertyCount,
                                                                  VkDisplayModeProperties2KHR *pProperties) {
    bool skip = false;
    skip |= ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                           "VUID-vkGetDisplayModeProperties2KHR-physicalDevice-parameter", kVUIDUndefined);
    skip |= ValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false,
                           "VUID-vkGetDisplayModeProperties2KHR-display-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                 uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties,
                                                                 VkResult result) {
    if ((result != VK_SUCCESS) && (result != VK_INCOMPLETE)) return;
    for (uint32_t index = 0; index < *pPropertyCount; ++index) {
        CreateObject(physicalDevice, pProperties[index].displayModeProperties.displayMode, kVulkanObjectTypeDisplayModeKHR,
                     nullptr);
    }
}

bool ObjectLifetimes::PreCallValidateAcquirePerformanceConfigurationINTEL(
    VkDevice device, const VkPerformanceConfigurationAcquireInfoINTEL *pAcquireInfo,
    VkPerformanceConfigurationINTEL *pConfiguration) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false,
                           "VUID-vkAcquirePerformanceConfigurationINTEL-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateReleasePerformanceConfigurationINTEL(VkDevice device,
                                                                          VkPerformanceConfigurationINTEL configuration) {
    bool skip = false;
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false,
                           "VUID-vkReleasePerformanceConfigurationINTEL-device-parameter", kVUIDUndefined);

    return skip;
}

bool ObjectLifetimes::PreCallValidateQueueSetPerformanceConfigurationINTEL(VkQueue queue,
                                                                           VkPerformanceConfigurationINTEL configuration) {
    bool skip = false;
    skip |=
        ValidateObject(queue, queue, kVulkanObjectTypeQueue, false, "VUID-vkQueueSetPerformanceConfigurationINTEL-queue-parameter",
                       "VUID-vkQueueSetPerformanceConfigurationINTEL-commonparent");

    return skip;
}

bool ObjectLifetimes::PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer) {
    bool skip = false;
    skip |=
        ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateFramebuffer-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= ValidateObject(device, pCreateInfo->renderPass, kVulkanObjectTypeRenderPass, false,
                               "VUID-VkFramebufferCreateInfo-renderPass-parameter", "VUID-VkFramebufferCreateInfo-commonparent");
        if ((pCreateInfo->flags & VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR) == 0) {
            for (uint32_t index1 = 0; index1 < pCreateInfo->attachmentCount; ++index1) {
                skip |= ValidateObject(device, pCreateInfo->pAttachments[index1], kVulkanObjectTypeImageView, true, kVUIDUndefined,
                                       "VUID-VkFramebufferCreateInfo-commonparent");
            }
        }
    }

    return skip;
}

void ObjectLifetimes::PostCallRecordCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer,
                                                      VkResult result) {
    if (result != VK_SUCCESS) return;
    CreateObject(device, *pFramebuffer, kVulkanObjectTypeFramebuffer, pAllocator);
}
