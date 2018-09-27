/* Copyright (c) 2015-2018 The Khronos Group Inc.
 * Copyright (c) 2015-2018 Valve Corporation
 * Copyright (c) 2015-2018 LunarG, Inc.
 * Copyright (C) 2015-2018 Google Inc.
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

// Add new queue to head of global queue list
void ObjectLifetimes::AddQueueInfo(VkDevice device, uint32_t queue_node_index, VkQueue queue) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    auto queueItem = device_data->objdata.queue_info_map.find(queue);
    if (queueItem == device_data->objdata.queue_info_map.end()) {
        ObjTrackQueueInfo *p_queue_info = new ObjTrackQueueInfo;
        if (p_queue_info != NULL) {
            memset(p_queue_info, 0, sizeof(ObjTrackQueueInfo));
            p_queue_info->queue = queue;
            p_queue_info->queue_node_index = queue_node_index;
            device_data->objdata.queue_info_map[queue] = p_queue_info;
        } else {
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                    HandleToUint64(queue), kVUID_ObjectTracker_InternalError,
                    "ERROR:  VK_ERROR_OUT_OF_HOST_MEMORY -- could not allocate memory for Queue Information");
        }
    }
}

// Destroy memRef lists and free all memory
void ObjectLifetimes::DestroyQueueDataStructures(VkDevice device) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    for (auto queue_item : device_data->objdata.queue_info_map) {
        delete queue_item.second;
    }
    device_data->objdata.queue_info_map.clear();

    // Destroy the items in the queue map
    auto queue = device_data->objdata.object_map[kVulkanObjectTypeQueue].begin();
    while (queue != device_data->objdata.object_map[kVulkanObjectTypeQueue].end()) {
        uint32_t obj_index = queue->second->object_type;
        assert(device_data->objdata.num_total_objects > 0);
        device_data->objdata.num_total_objects--;
        assert(device_data->objdata.num_objects[obj_index] > 0);
        device_data->objdata.num_objects[obj_index]--;
        log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                queue->second->handle, kVUID_ObjectTracker_Info,
                "OBJ_STAT Destroy Queue obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " Queue objs).",
                queue->second->handle, device_data->objdata.num_total_objects, device_data->objdata.num_objects[obj_index]);
        delete queue->second;
        queue = device_data->objdata.object_map[kVulkanObjectTypeQueue].erase(queue);
    }
}

// Check Queue type flags for selected queue operations
void ObjectLifetimes::ValidateQueueFlags(VkQueue queue, const char *function) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    auto queue_item = device_data->objdata.queue_info_map.find(queue);
    if (queue_item != device_data->objdata.queue_info_map.end()) {
        ObjTrackQueueInfo *pQueueInfo = queue_item->second;
        if (pQueueInfo != NULL) {
            instance_layer_data *instance_data =
                GetLayerDataPtr(get_dispatch_key(device_data->physical_device), instance_layer_data_map);
            if ((instance_data->objdata.queue_family_properties[pQueueInfo->queue_node_index].queueFlags &
                 VK_QUEUE_SPARSE_BINDING_BIT) == 0) {
                log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                        HandleToUint64(queue), "VUID-vkQueueBindSparse-queuetype",
                        "Attempting %s on a non-memory-management capable queue -- VK_QUEUE_SPARSE_BINDING_BIT not set.", function);
            }
        }
    }
}

// Look for this device object in any of the instance child devices lists.
// NOTE: This is of dubious value. In most circumstances Vulkan will die a flaming death if a dispatchable object is invalid.
// However, if this layer is loaded first and GetProcAddress is used to make API calls, it will detect bad DOs.
bool ObjectLifetimes::ValidateDeviceObject(uint64_t device_handle, const std::string &invalid_handle_code,
                                           const std::string &wrong_device_code) {
    VkInstance last_instance = nullptr;
    for (auto instance_data : instance_layer_data_map) {
        for (auto object : instance_data.second->objdata.object_map[kVulkanObjectTypeDevice]) {
            // Grab last instance to use for possible error message
            last_instance = instance_data.second->instance;
            if (object.second->handle == device_handle) return false;
        }
    }

    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(last_instance), instance_layer_data_map);
    return log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, device_handle,
                   invalid_handle_code, "Invalid Device Object 0x%" PRIxLEAST64 ".", device_handle);
}

void ObjectLifetimes::AllocateCommandBuffer(VkDevice device, const VkCommandPool command_pool, const VkCommandBuffer command_buffer,
                                            VkCommandBufferLevel level) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
            HandleToUint64(command_buffer), kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64,
            object_track_index++, "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT", HandleToUint64(command_buffer));

    ObjTrackState *pNewObjNode = new ObjTrackState;
    pNewObjNode->object_type = kVulkanObjectTypeCommandBuffer;
    pNewObjNode->handle = HandleToUint64(command_buffer);
    pNewObjNode->parent_object = HandleToUint64(command_pool);
    if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        pNewObjNode->status = OBJSTATUS_COMMAND_BUFFER_SECONDARY;
    } else {
        pNewObjNode->status = OBJSTATUS_NONE;
    }
    device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer][HandleToUint64(command_buffer)] = pNewObjNode;
    device_data->objdata.num_objects[kVulkanObjectTypeCommandBuffer]++;
    device_data->objdata.num_total_objects++;
}

bool ObjectLifetimes::ValidateCommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    uint64_t object_handle = HandleToUint64(command_buffer);
    if (device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer].find(object_handle) !=
        device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer].end()) {
        ObjTrackState *pNode = device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer][HandleToUint64(command_buffer)];

        if (pNode->parent_object != HandleToUint64(command_pool)) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                            object_handle, "VUID-vkFreeCommandBuffers-pCommandBuffers-parent",
                            "FreeCommandBuffers is attempting to free Command Buffer 0x%" PRIxLEAST64
                            " belonging to Command Pool 0x%" PRIxLEAST64 " from pool 0x%" PRIxLEAST64 ").",
                            HandleToUint64(command_buffer), pNode->parent_object, HandleToUint64(command_pool));
        }
    } else {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                        object_handle, "VUID-vkFreeCommandBuffers-pCommandBuffers-00048", "Invalid %s Object 0x%" PRIxLEAST64 ".",
                        object_string[kVulkanObjectTypeCommandBuffer], object_handle);
    }
    return skip;
}

void ObjectLifetimes::AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
            HandleToUint64(descriptor_set), kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64,
            object_track_index++, "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT", HandleToUint64(descriptor_set));

    ObjTrackState *pNewObjNode = new ObjTrackState;
    pNewObjNode->object_type = kVulkanObjectTypeDescriptorSet;
    pNewObjNode->status = OBJSTATUS_NONE;
    pNewObjNode->handle = HandleToUint64(descriptor_set);
    pNewObjNode->parent_object = HandleToUint64(descriptor_pool);
    device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet][HandleToUint64(descriptor_set)] = pNewObjNode;
    device_data->objdata.num_objects[kVulkanObjectTypeDescriptorSet]++;
    device_data->objdata.num_total_objects++;
}

bool ObjectLifetimes::ValidateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    uint64_t object_handle = HandleToUint64(descriptor_set);
    auto dsItem = device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].find(object_handle);
    if (dsItem != device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].end()) {
        ObjTrackState *pNode = dsItem->second;

        if (pNode->parent_object != HandleToUint64(descriptor_pool)) {
            skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                            object_handle, "VUID-vkFreeDescriptorSets-pDescriptorSets-parent",
                            "FreeDescriptorSets is attempting to free descriptorSet 0x%" PRIxLEAST64
                            " belonging to Descriptor Pool 0x%" PRIxLEAST64 " from pool 0x%" PRIxLEAST64 ").",
                            HandleToUint64(descriptor_set), pNode->parent_object, HandleToUint64(descriptor_pool));
        }
    } else {
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                        object_handle, "VUID-vkFreeDescriptorSets-pDescriptorSets-00310", "Invalid %s Object 0x%" PRIxLEAST64 ".",
                        object_string[kVulkanObjectTypeDescriptorSet], object_handle);
    }
    return skip;
}

template <typename DispObj>
bool ObjectLifetimes::ValidateDescriptorWrite(DispObj disp, VkWriteDescriptorSet const *desc, bool isPush) {
    bool skip = false;

    if (!isPush && desc->dstSet) {
        skip |= DeviceValidateObject(disp, desc->dstSet, kVulkanObjectTypeDescriptorSet, false,
                                     "VUID-VkWriteDescriptorSet-dstSet-00320", "VUID-VkWriteDescriptorSet-commonparent");
    }

    if ((desc->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)) {
        for (uint32_t idx2 = 0; idx2 < desc->descriptorCount; ++idx2) {
            skip |=
                DeviceValidateObject(disp, desc->pTexelBufferView[idx2], kVulkanObjectTypeBufferView, false,
                                     "VUID-VkWriteDescriptorSet-descriptorType-00323", "VUID-VkWriteDescriptorSet-commonparent");
        }
    }

    if ((desc->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
        for (uint32_t idx3 = 0; idx3 < desc->descriptorCount; ++idx3) {
            skip |=
                DeviceValidateObject(disp, desc->pImageInfo[idx3].imageView, kVulkanObjectTypeImageView, false,
                                     "VUID-VkWriteDescriptorSet-descriptorType-00326", "VUID-VkDescriptorImageInfo-commonparent");
        }
    }

    if ((desc->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
        (desc->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)) {
        for (uint32_t idx4 = 0; idx4 < desc->descriptorCount; ++idx4) {
            if (desc->pBufferInfo[idx4].buffer) {
                skip |= DeviceValidateObject(disp, desc->pBufferInfo[idx4].buffer, kVulkanObjectTypeBuffer, false,
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
    skip |= DeviceValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false,
                                 "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-parameter",
                                 "VUID-vkCmdPushDescriptorSetKHR-commonparent");
    skip |= DeviceValidateObject(commandBuffer, layout, kVulkanObjectTypePipelineLayout, false,
                                 "VUID-vkCmdPushDescriptorSetKHR-layout-parameter", "VUID-vkCmdPushDescriptorSetKHR-commonparent");
    if (pDescriptorWrites) {
        for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
            skip |= ValidateDescriptorWrite(commandBuffer, &pDescriptorWrites[index0], true);
        }
    }
    return skip;
}

void ObjectLifetimes::CreateQueue(VkDevice device, VkQueue vkObj) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
            HandleToUint64(vkObj), kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64,
            object_track_index++, "VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT", HandleToUint64(vkObj));

    ObjTrackState *p_obj_node = NULL;
    auto queue_item = device_data->objdata.object_map[kVulkanObjectTypeQueue].find(HandleToUint64(vkObj));
    if (queue_item == device_data->objdata.object_map[kVulkanObjectTypeQueue].end()) {
        p_obj_node = new ObjTrackState;
        device_data->objdata.object_map[kVulkanObjectTypeQueue][HandleToUint64(vkObj)] = p_obj_node;
        device_data->objdata.num_objects[kVulkanObjectTypeQueue]++;
        device_data->objdata.num_total_objects++;
    } else {
        p_obj_node = queue_item->second;
    }
    p_obj_node->object_type = kVulkanObjectTypeQueue;
    p_obj_node->status = OBJSTATUS_NONE;
    p_obj_node->handle = HandleToUint64(vkObj);
}

void ObjectLifetimes::CreateSwapchainImageObject(VkDevice dispatchable_object, VkImage swapchain_image, VkSwapchainKHR swapchain) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(dispatchable_object), layer_data_map);
    log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
            HandleToUint64(swapchain_image), kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64,
            object_track_index++, "SwapchainImage", HandleToUint64(swapchain_image));

    ObjTrackState *pNewObjNode = new ObjTrackState;
    pNewObjNode->object_type = kVulkanObjectTypeImage;
    pNewObjNode->status = OBJSTATUS_NONE;
    pNewObjNode->handle = HandleToUint64(swapchain_image);
    pNewObjNode->parent_object = HandleToUint64(swapchain);
    device_data->objdata.swapchainImageMap[HandleToUint64(swapchain_image)] = pNewObjNode;
}

bool ObjectLifetimes::DeviceReportUndestroyedObjects(VkDevice device, VulkanObjectType object_type, const std::string &error_code) {
    bool skip = false;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    for (const auto &item : device_data->objdata.object_map[object_type]) {
        const ObjTrackState *object_info = item.second;
        skip |= log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, get_debug_report_enum[object_type],
                        object_info->handle, error_code,
                        "OBJ ERROR : For device 0x%" PRIxLEAST64 ", %s object 0x%" PRIxLEAST64 " has not been destroyed.",
                        HandleToUint64(device), object_string[object_type], object_info->handle);
    }
    return skip;
}

void ObjectLifetimes::DeviceDestroyUndestroyedObjects(VkDevice device, VulkanObjectType object_type) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    while (!device_data->objdata.object_map[object_type].empty()) {
        auto item = device_data->objdata.object_map[object_type].begin();

        ObjTrackState *object_info = item->second;
        DestroyObjectSilently(&device_data->objdata, object_info->handle, object_type);
    }
}

bool ObjectLifetimes::PreCallValidateDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    instance_layer_data *instance_data = GetLayerDataPtr(key, instance_layer_data_map);
    bool skip = false;

    // We validate here for coverage, though we'd not have made it this for with a bad instance.
    skip |= InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, true, "VUID-vkDestroyInstance-instance-parameter",
                                   kVUIDUndefined);

    // Validate that child devices have been destroyed
    for (const auto &iit : instance_data->objdata.object_map[kVulkanObjectTypeDevice]) {
        ObjTrackState *pNode = iit.second;

        VkDevice device = reinterpret_cast<VkDevice>(pNode->handle);
        VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[pNode->object_type];

        skip |= log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, pNode->handle,
                        kVUID_ObjectTracker_ObjectLeak, "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.",
                        string_VkDebugReportObjectTypeEXT(debug_object_type), pNode->handle);

        // Report any remaining objects in LL
        skip |= ReportUndestroyedObjects(device, "VUID-vkDestroyInstance-instance-00629");

        skip |= InstanceValidateDestroyObject(instance, device, kVulkanObjectTypeDevice, pAllocator,
                                              "VUID-vkDestroyInstance-instance-00630", "VUID-vkDestroyInstance-instance-00631");
    }

    InstanceValidateDestroyObject(instance, instance, kVulkanObjectTypeInstance, pAllocator,
                                  "VUID-vkDestroyInstance-instance-00630", "VUID-vkDestroyInstance-instance-00631");

    return skip;
}

bool ObjectLifetimes::PreCallValidateEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                              VkPhysicalDevice *pPhysicalDevices) {
    bool skip = InstanceValidateObject(instance, instance, kVulkanObjectTypeInstance, false,
                                       "VUID-vkEnumeratePhysicalDevices-instance-parameter", kVUIDUndefined);
    return skip;
}

void ObjectLifetimes::PostCallRecordEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                             VkPhysicalDevice *pPhysicalDevices) {
    if (pPhysicalDevices) {
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
            InstanceCreateObject(instance, pPhysicalDevices[i], kVulkanObjectTypePhysicalDevice, nullptr);
        }
    }
}

void ObjectLifetimes::PreCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    dispatch_key key = get_dispatch_key(instance);
    instance_layer_data *instance_data = GetLayerDataPtr(key, instance_layer_data_map);

    // Destroy physical devices
    for (auto iit = instance_data->objdata.object_map[kVulkanObjectTypePhysicalDevice].begin();
         iit != instance_data->objdata.object_map[kVulkanObjectTypePhysicalDevice].end();) {
        ObjTrackState *pNode = iit->second;
        VkPhysicalDevice physical_device = reinterpret_cast<VkPhysicalDevice>(pNode->handle);
        InstanceRecordDestroyObject(instance, physical_device, kVulkanObjectTypePhysicalDevice);
        iit = instance_data->objdata.object_map[kVulkanObjectTypePhysicalDevice].begin();
    }

    // Destroy child devices
    for (auto iit = instance_data->objdata.object_map[kVulkanObjectTypeDevice].begin();
         iit != instance_data->objdata.object_map[kVulkanObjectTypeDevice].end();) {
        ObjTrackState *pNode = iit->second;
        VkDevice device = reinterpret_cast<VkDevice>(pNode->handle);
        DestroyUndestroyedObjects(device);

        InstanceRecordDestroyObject(instance, device, kVulkanObjectTypeDevice);
        iit = instance_data->objdata.object_map[kVulkanObjectTypeDevice].begin();
    }

    instance_data->objdata.object_map[kVulkanObjectTypeDevice].clear();
}

void ObjectLifetimes::PostCallRecordDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    InstanceRecordDestroyObject(instance, instance, kVulkanObjectTypeInstance);
}

bool ObjectLifetimes::PreCallValidateDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, true, "VUID-vkDestroyDevice-device-parameter",
                                 kVUIDUndefined);
    skip |= InstanceValidateDestroyObject(device_data->physical_device, device, kVulkanObjectTypeDevice, pAllocator,
                                          "VUID-vkDestroyDevice-device-00379", "VUID-vkDestroyDevice-device-00380");
    // Report any remaining objects associated with this VkDevice object in LL
    skip |= ReportUndestroyedObjects(device, "VUID-vkDestroyDevice-device-00378");

    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    InstanceRecordDestroyObject(device_data->physical_device, device, kVulkanObjectTypeDevice);
    DestroyUndestroyedObjects(device);

    // Clean up Queue's MemRef Linked Lists
    DestroyQueueDataStructures(device);
}

bool ObjectLifetimes::PreCallValidateGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                                    VkQueue *pQueue) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceQueue-device-parameter",
                                 kVUIDUndefined);
    return skip;
}

void ObjectLifetimes::PostCallRecordGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex,
                                                   VkQueue *pQueue) {
    CreateQueue(device, *pQueue);
    AddQueueInfo(device, queueFamilyIndex, *pQueue);
}

bool ObjectLifetimes::PreCallValidateGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    return DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceQueue2-device-parameter",
                                kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    CreateQueue(device, *pQueue);
    AddQueueInfo(device, pQueueInfo->queueFamilyIndex, *pQueue);
}

bool ObjectLifetimes::PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                          const VkWriteDescriptorSet *pDescriptorWrites,
                                                          uint32_t descriptorCopyCount,
                                                          const VkCopyDescriptorSet *pDescriptorCopies) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkUpdateDescriptorSets-device-parameter",
                                 kVUIDUndefined);
    if (pDescriptorCopies) {
        for (uint32_t idx0 = 0; idx0 < descriptorCopyCount; ++idx0) {
            if (pDescriptorCopies[idx0].dstSet) {
                skip |= DeviceValidateObject(device, pDescriptorCopies[idx0].dstSet, kVulkanObjectTypeDescriptorSet, false,
                                             "VUID-VkCopyDescriptorSet-dstSet-parameter", "VUID-VkCopyDescriptorSet-commonparent");
            }
            if (pDescriptorCopies[idx0].srcSet) {
                skip |= DeviceValidateObject(device, pDescriptorCopies[idx0].srcSet, kVulkanObjectTypeDescriptorSet, false,
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
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkResetDescriptorPool-device-parameter",
                                 kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                                 "VUID-vkResetDescriptorPool-descriptorPool-parameter",
                                 "VUID-vkResetDescriptorPool-descriptorPool-parent");
    for (const auto &itr : device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet]) {
        if (itr.second->parent_object == HandleToUint64(descriptorPool)) {
            skip |= DeviceValidateDestroyObject(device, (VkDescriptorSet)(itr.first), kVulkanObjectTypeDescriptorSet, nullptr,
                                                kVUIDUndefined, kVUIDUndefined);
        }
    }
    return skip;
}

void ObjectLifetimes::PreCallRecordResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                       VkDescriptorPoolResetFlags flags) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    // A DescriptorPool's descriptor sets are implicitly deleted when the pool is reset. Remove this pool's descriptor sets from
    // our descriptorSet map.
    auto itr = device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].begin();
    while (itr != device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].end()) {
        ObjTrackState *pNode = (*itr).second;
        auto del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(descriptorPool)) {
            DeviceRecordDestroyObject(device, (VkDescriptorSet)((*del_itr).first), kVulkanObjectTypeDescriptorSet);
        }
    }
}

bool ObjectLifetimes::PreCallValidateBeginCommandBuffer(VkCommandBuffer command_buffer,
                                                        const VkCommandBufferBeginInfo *begin_info) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(command_buffer), layer_data_map);
    bool skip = false;
    skip |= DeviceValidateObject(command_buffer, command_buffer, kVulkanObjectTypeCommandBuffer, false,
                                 "VUID-vkBeginCommandBuffer-commandBuffer-parameter", kVUIDUndefined);
    if (begin_info) {
        ObjTrackState *pNode = device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer][HandleToUint64(command_buffer)];
        if ((begin_info->pInheritanceInfo) && (pNode->status & OBJSTATUS_COMMAND_BUFFER_SECONDARY) &&
            (begin_info->flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)) {
            skip |= DeviceValidateObject(command_buffer, begin_info->pInheritanceInfo->framebuffer, kVulkanObjectTypeFramebuffer,
                                         true, "VUID-VkCommandBufferBeginInfo-flags-00055",
                                         "VUID-VkCommandBufferInheritanceInfo-commonparent");
            skip |= DeviceValidateObject(command_buffer, begin_info->pInheritanceInfo->renderPass, kVulkanObjectTypeRenderPass,
                                         false, "VUID-VkCommandBufferBeginInfo-flags-00053",
                                         "VUID-VkCommandBufferInheritanceInfo-commonparent");
        }
    }
    return skip;
}

bool ObjectLifetimes::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                           uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetSwapchainImagesKHR-device-parameter",
                                 "VUID-vkGetSwapchainImagesKHR-commonparent");
    skip |= DeviceValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false,
                                 "VUID-vkGetSwapchainImagesKHR-swapchain-parameter", "VUID-vkGetSwapchainImagesKHR-commonparent");
    return skip;
}

void ObjectLifetimes::PostCallRecordGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                          VkImage *pSwapchainImages) {
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
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false,
                                 "VUID-vkCreateDescriptorSetLayout-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        if (pCreateInfo->pBindings) {
            for (uint32_t binding_index = 0; binding_index < pCreateInfo->bindingCount; ++binding_index) {
                const VkDescriptorSetLayoutBinding &binding = pCreateInfo->pBindings[binding_index];
                const bool is_sampler_type = binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
                                             binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                if (binding.pImmutableSamplers && is_sampler_type) {
                    for (uint32_t index2 = 0; index2 < binding.descriptorCount; ++index2) {
                        const VkSampler sampler = binding.pImmutableSamplers[index2];
                        skip |= DeviceValidateObject(device, sampler, kVulkanObjectTypeSampler, false,
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
                                                              VkDescriptorSetLayout *pSetLayout) {
    DeviceCreateObject(device, *pSetLayout, kVulkanObjectTypeDescriptorSetLayout, pAllocator);
}

bool ObjectLifetimes::ValidateSamplerObjects(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo) {
    bool skip = false;
    if (pCreateInfo->pBindings) {
        for (uint32_t index1 = 0; index1 < pCreateInfo->bindingCount; ++index1) {
            for (uint32_t index2 = 0; index2 < pCreateInfo->pBindings[index1].descriptorCount; ++index2) {
                if (pCreateInfo->pBindings[index1].pImmutableSamplers) {
                    skip |= DeviceValidateObject(device, pCreateInfo->pBindings[index1].pImmutableSamplers[index2],
                                                 kVulkanObjectTypeSampler, true,
                                                 "VUID-VkDescriptorSetLayoutBinding-descriptorType-00282", kVUIDUndefined);
                }
            }
        }
    }
    return skip;
}

bool ObjectLifetimes::PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device,
                                                                   const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                   VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false,
                                     "VUID-vkGetDescriptorSetLayoutSupport-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= ValidateSamplerObjects(device, pCreateInfo);
    }
    return skip;
}
bool ObjectLifetimes::PreCallValidateGetDescriptorSetLayoutSupportKHR(VkDevice device,
                                                                      const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                      VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false,
                                     "VUID-vkGetDescriptorSetLayoutSupportKHR-device-parameter", kVUIDUndefined);
    if (pCreateInfo) {
        skip |= ValidateSamplerObjects(device, pCreateInfo);
    }
    return skip;
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                            uint32_t *pQueueFamilyPropertyCount,
                                                                            VkQueueFamilyProperties *pQueueFamilyProperties) {
    return InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                                  "VUID-vkGetPhysicalDeviceQueueFamilyProperties-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pQueueFamilyPropertyCount,
                                                                           VkQueueFamilyProperties *pQueueFamilyProperties) {
    if (pQueueFamilyProperties != NULL) {
        auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
        if (instance_data->objdata.queue_family_properties.size() < *pQueueFamilyPropertyCount) {
            instance_data->objdata.queue_family_properties.resize(*pQueueFamilyPropertyCount);
        }
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; i++) {
            instance_data->objdata.queue_family_properties[i] = pQueueFamilyProperties[i];
        }
    }
}

void ObjectLifetimes::PostCallRecordCreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                                   VkInstance *pInstance) {
    InstanceCreateObject(*pInstance, *pInstance, kVulkanObjectTypeInstance, pAllocator);
}

bool ObjectLifetimes::PreCallValidateAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                            VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAllocateCommandBuffers-device-parameter",
                                 kVUIDUndefined);
    skip |= DeviceValidateObject(device, pAllocateInfo->commandPool, kVulkanObjectTypeCommandPool, false,
                                 "VUID-VkCommandBufferAllocateInfo-commandPool-parameter", kVUIDUndefined);
    return skip;
}

void ObjectLifetimes::PostCallRecordAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                           VkCommandBuffer *pCommandBuffers) {
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
        AllocateCommandBuffer(device, pAllocateInfo->commandPool, pCommandBuffers[i], pAllocateInfo->level);
    }
}

bool ObjectLifetimes::PreCallValidateAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                            VkDescriptorSet *pDescriptorSets) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAllocateDescriptorSets-device-parameter",
                                 kVUIDUndefined);
    skip |= DeviceValidateObject(device, pAllocateInfo->descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                                 "VUID-VkDescriptorSetAllocateInfo-descriptorPool-parameter",
                                 "VUID-VkDescriptorSetAllocateInfo-commonparent");
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        skip |= DeviceValidateObject(device, pAllocateInfo->pSetLayouts[i], kVulkanObjectTypeDescriptorSetLayout, false,
                                     "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-parameter",
                                     "VUID-VkDescriptorSetAllocateInfo-commonparent");
    }
    return skip;
}

void ObjectLifetimes::PostCallRecordAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                           VkDescriptorSet *pDescriptorSets) {
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
        AllocateDescriptorSet(device, pAllocateInfo->descriptorPool, pDescriptorSets[i]);
    }
}

bool ObjectLifetimes::PreCallValidateFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                        const VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFreeCommandBuffers-device-parameter",
                                 kVUIDUndefined);
    skip |= DeviceValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, false,
                                 "VUID-vkFreeCommandBuffers-commandPool-parameter", "VUID-vkFreeCommandBuffers-commandPool-parent");
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        if (pCommandBuffers[i] != VK_NULL_HANDLE) {
            skip |= ValidateCommandBuffer(device, commandPool, pCommandBuffers[i]);
            skip |= DeviceValidateDestroyObject(device, pCommandBuffers[i], kVulkanObjectTypeCommandBuffer, nullptr, kVUIDUndefined,
                                                kVUIDUndefined);
        }
    }
    return skip;
}

void ObjectLifetimes::PreCallRecordFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                                      const VkCommandBuffer *pCommandBuffers) {
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        DeviceRecordDestroyObject(device, pCommandBuffers[i], kVulkanObjectTypeCommandBuffer);
    }
}

bool ObjectLifetimes::PreCallValidateDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                         const VkAllocationCallbacks *pAllocator) {
    return DeviceValidateDestroyObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, pAllocator,
                                       "VUID-vkDestroySwapchainKHR-swapchain-01283", "VUID-vkDestroySwapchainKHR-swapchain-01284");
}

void ObjectLifetimes::PreCallRecordDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain,
                                                       const VkAllocationCallbacks *pAllocator) {
    DeviceRecordDestroyObject(device, swapchain, kVulkanObjectTypeSwapchainKHR);
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    std::unordered_map<uint64_t, ObjTrackState *>::iterator itr = device_data->objdata.swapchainImageMap.begin();
    while (itr != device_data->objdata.swapchainImageMap.end()) {
        ObjTrackState *pNode = (*itr).second;
        if (pNode->parent_object == HandleToUint64(swapchain)) {
            delete pNode;
            auto delete_item = itr++;
            device_data->objdata.swapchainImageMap.erase(delete_item);
        } else {
            ++itr;
        }
    }
}

bool ObjectLifetimes::PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool,
                                                        uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets) {
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFreeDescriptorSets-device-parameter",
                                 kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                                 "VUID-vkFreeDescriptorSets-descriptorPool-parameter",
                                 "VUID-vkFreeDescriptorSets-descriptorPool-parent");
    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateDescriptorSet(device, descriptorPool, pDescriptorSets[i]);
            skip |= DeviceValidateDestroyObject(device, pDescriptorSets[i], kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                                                kVUIDUndefined);
        }
    }
    return skip;
}
void ObjectLifetimes::PreCallRecordFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                      const VkDescriptorSet *pDescriptorSets) {
    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        DeviceRecordDestroyObject(device, pDescriptorSets[i], kVulkanObjectTypeDescriptorSet);
    }
}

bool ObjectLifetimes::PreCallValidateDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                           const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyDescriptorPool-device-parameter",
                                 kVUIDUndefined);
    skip |= DeviceValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, true,
                                 "VUID-vkDestroyDescriptorPool-descriptorPool-parameter",
                                 "VUID-vkDestroyDescriptorPool-descriptorPool-parent");
    std::unordered_map<uint64_t, ObjTrackState *>::iterator itr =
        device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].begin();
    while (itr != device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].end()) {
        ObjTrackState *pNode = (*itr).second;
        auto del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(descriptorPool)) {
            skip |= DeviceValidateDestroyObject(device, (VkDescriptorSet)((*del_itr).first), kVulkanObjectTypeDescriptorSet,
                                                nullptr, kVUIDUndefined, kVUIDUndefined);
        }
    }
    skip |= DeviceValidateDestroyObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, pAllocator,
                                        "VUID-vkDestroyDescriptorPool-descriptorPool-00304",
                                        "VUID-vkDestroyDescriptorPool-descriptorPool-00305");
    return skip;
}
void ObjectLifetimes::PreCallRecordDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                         const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    std::unordered_map<uint64_t, ObjTrackState *>::iterator itr =
        device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].begin();
    while (itr != device_data->objdata.object_map[kVulkanObjectTypeDescriptorSet].end()) {
        ObjTrackState *pNode = (*itr).second;
        auto del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(descriptorPool)) {
            DeviceRecordDestroyObject(device, (VkDescriptorSet)((*del_itr).first), kVulkanObjectTypeDescriptorSet);
        }
    }
    DeviceRecordDestroyObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool);
}

bool ObjectLifetimes::PreCallValidateDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                        const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    skip |= DeviceValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyCommandPool-device-parameter",
                                 kVUIDUndefined);
    skip |= DeviceValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, true,
                                 "VUID-vkDestroyCommandPool-commandPool-parameter", "VUID-vkDestroyCommandPool-commandPool-parent");
    auto itr = device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer].begin();
    auto del_itr = itr;
    while (itr != device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer].end()) {
        ObjTrackState *pNode = (*itr).second;
        del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(commandPool)) {
            skip |= ValidateCommandBuffer(device, commandPool, reinterpret_cast<VkCommandBuffer>((*del_itr).first));
            skip |= DeviceValidateDestroyObject(device, reinterpret_cast<VkCommandBuffer>((*del_itr).first),
                                                kVulkanObjectTypeCommandBuffer, nullptr, kVUIDUndefined, kVUIDUndefined);
        }
    }
    skip |=
        DeviceValidateDestroyObject(device, commandPool, kVulkanObjectTypeCommandPool, pAllocator,
                                    "VUID-vkDestroyCommandPool-commandPool-00042", "VUID-vkDestroyCommandPool-commandPool-00043");
    return skip;
}

void ObjectLifetimes::PreCallRecordDestroyCommandPool(VkDevice device, VkCommandPool commandPool,
                                                      const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    auto itr = device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer].begin();
    auto del_itr = itr;
    // A CommandPool's cmd buffers are implicitly deleted when pool is deleted. Remove this pool's cmdBuffers from cmd buffer map.
    while (itr != device_data->objdata.object_map[kVulkanObjectTypeCommandBuffer].end()) {
        ObjTrackState *pNode = (*itr).second;
        del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(commandPool)) {
            DeviceRecordDestroyObject(device, reinterpret_cast<VkCommandBuffer>((*del_itr).first), kVulkanObjectTypeCommandBuffer);
        }
    }
    DeviceRecordDestroyObject(device, commandPool, kVulkanObjectTypeCommandPool);
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                             uint32_t *pQueueFamilyPropertyCount,
                                                                             VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    return InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                                  "VUID-vkGetPhysicalDeviceQueueFamilyProperties2-physicalDevice-parameter", kVUIDUndefined);
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                                uint32_t *pQueueFamilyPropertyCount,
                                                                                VkQueueFamilyProperties2 *pQueueFamilyProperties) {
    return InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                                  "VUID-vkGetPhysicalDeviceQueueFamilyProperties2KHR-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                            uint32_t *pQueueFamilyPropertyCount,
                                                                            VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    if (pQueueFamilyProperties != NULL) {
        if (instance_data->objdata.queue_family_properties.size() < *pQueueFamilyPropertyCount) {
            instance_data->objdata.queue_family_properties.resize(*pQueueFamilyPropertyCount);
        }
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; i++) {
            instance_data->objdata.queue_family_properties[i] = pQueueFamilyProperties[i].queueFamilyProperties;
        }
    }
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    instance_layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), instance_layer_data_map);
    if (pQueueFamilyProperties != NULL) {
        if (instance_data->objdata.queue_family_properties.size() < *pQueueFamilyPropertyCount) {
            instance_data->objdata.queue_family_properties.resize(*pQueueFamilyPropertyCount);
        }
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; i++) {
            instance_data->objdata.queue_family_properties[i] = pQueueFamilyProperties[i].queueFamilyProperties;
        }
    }
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pPropertyCount,
                                                                           VkDisplayPropertiesKHR *pProperties) {
    return InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                                  "VUID-vkGetPhysicalDeviceDisplayPropertiesKHR-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                          VkDisplayPropertiesKHR *pProperties) {
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            InstanceCreateObject(physicalDevice, pProperties[i].display, kVulkanObjectTypeDisplayKHR, nullptr);
        }
    }
}

bool ObjectLifetimes::PreCallValidateGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                 uint32_t *pPropertyCount,
                                                                 VkDisplayModePropertiesKHR *pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                                   "VUID-vkGetDisplayModePropertiesKHR-physicalDevice-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false,
                                   "VUID-vkGetDisplayModePropertiesKHR-display-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties) {
    if (pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            InstanceCreateObject(physicalDevice, pProperties[i].displayMode, kVulkanObjectTypeDisplayModeKHR, nullptr);
        }
    }
}

bool ObjectLifetimes::PreCallValidateGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                            uint32_t *pPropertyCount,
                                                                            VkDisplayProperties2KHR *pProperties) {
    return InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                                  "VUID-vkGetPhysicalDeviceDisplayProperties2KHR-physicalDevice-parameter", kVUIDUndefined);
}

void ObjectLifetimes::PostCallRecordGetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                           uint32_t *pPropertyCount,
                                                                           VkDisplayProperties2KHR *pProperties) {
    for (uint32_t index = 0; index < *pPropertyCount; ++index) {
        InstanceCreateObject(physicalDevice, pProperties[index].displayProperties.display, kVulkanObjectTypeDisplayKHR, nullptr);
    }
}

bool ObjectLifetimes::PreCallValidateGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                  uint32_t *pPropertyCount,
                                                                  VkDisplayModeProperties2KHR *pProperties) {
    bool skip = false;
    skip |= InstanceValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                                   "VUID-vkGetDisplayModeProperties2KHR-physicalDevice-parameter", kVUIDUndefined);
    skip |= InstanceValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false,
                                   "VUID-vkGetDisplayModeProperties2KHR-display-parameter", kVUIDUndefined);

    return skip;
}

void ObjectLifetimes::PostCallRecordGetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                                 uint32_t *pPropertyCount,
                                                                 VkDisplayModeProperties2KHR *pProperties) {
    for (uint32_t index = 0; index < *pPropertyCount; ++index) {
        InstanceCreateObject(physicalDevice, pProperties[index].displayModeProperties.displayMode, kVulkanObjectTypeDisplayModeKHR,
                             nullptr);
    }
}
