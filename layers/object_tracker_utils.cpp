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

#include "object_tracker.h"

namespace object_tracker {

std::unordered_map<void *, layer_data *> layer_data_map;
std::mutex global_lock;
uint64_t object_track_index = 0;
uint32_t loader_layer_if_version = CURRENT_LOADER_LAYER_INTERFACE_VERSION;

void InitObjectTracker(layer_data *my_data, const VkAllocationCallbacks *pAllocator) {
    layer_debug_report_actions(my_data->report_data, my_data->logging_callback, pAllocator, "lunarg_object_tracker");
    layer_debug_messenger_actions(my_data->report_data, my_data->logging_messenger, pAllocator, "lunarg_object_tracker");
}

// Add new queue to head of global queue list
void AddQueueInfo(VkDevice device, uint32_t queue_node_index, VkQueue queue) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    auto queueItem = device_data->queue_info_map.find(queue);
    if (queueItem == device_data->queue_info_map.end()) {
        ObjTrackQueueInfo *p_queue_info = new ObjTrackQueueInfo;
        if (p_queue_info != NULL) {
            memset(p_queue_info, 0, sizeof(ObjTrackQueueInfo));
            p_queue_info->queue = queue;
            p_queue_info->queue_node_index = queue_node_index;
            device_data->queue_info_map[queue] = p_queue_info;
        } else {
            log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                    HandleToUint64(queue), kVUID_ObjectTracker_InternalError,
                    "ERROR:  VK_ERROR_OUT_OF_HOST_MEMORY -- could not allocate memory for Queue Information");
        }
    }
}

// Destroy memRef lists and free all memory
void DestroyQueueDataStructures(VkDevice device) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    for (auto queue_item : device_data->queue_info_map) {
        delete queue_item.second;
    }
    device_data->queue_info_map.clear();

    // Destroy the items in the queue map
    auto queue = device_data->object_map[kVulkanObjectTypeQueue].begin();
    while (queue != device_data->object_map[kVulkanObjectTypeQueue].end()) {
        uint32_t obj_index = queue->second->object_type;
        assert(device_data->num_total_objects > 0);
        device_data->num_total_objects--;
        assert(device_data->num_objects[obj_index] > 0);
        device_data->num_objects[obj_index]--;
        log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
                queue->second->handle, kVUID_ObjectTracker_Info,
                "OBJ_STAT Destroy Queue obj 0x%" PRIxLEAST64 " (%" PRIu64 " total objs remain & %" PRIu64 " Queue objs).",
                queue->second->handle, device_data->num_total_objects, device_data->num_objects[obj_index]);
        delete queue->second;
        queue = device_data->object_map[kVulkanObjectTypeQueue].erase(queue);
    }
}

// Check Queue type flags for selected queue operations
void ValidateQueueFlags(VkQueue queue, const char *function) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    auto queue_item = device_data->queue_info_map.find(queue);
    if (queue_item != device_data->queue_info_map.end()) {
        ObjTrackQueueInfo *pQueueInfo = queue_item->second;
        if (pQueueInfo != NULL) {
            layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(device_data->physical_device), layer_data_map);
            if ((instance_data->queue_family_properties[pQueueInfo->queue_node_index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ==
                0) {
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
bool ValidateDeviceObject(uint64_t device_handle, const std::string &invalid_handle_code, const std::string &wrong_device_code) {
    VkInstance last_instance = nullptr;
    for (auto layer_data : layer_data_map) {
        for (auto object : layer_data.second->object_map[kVulkanObjectTypeDevice]) {
            // Grab last instance to use for possible error message
            last_instance = layer_data.second->instance;
            if (object.second->handle == device_handle) return false;
        }
    }

    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(last_instance), layer_data_map);
    return log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, device_handle,
                   invalid_handle_code, "Invalid Device Object 0x%" PRIxLEAST64 ".", device_handle);
}

void AllocateCommandBuffer(VkDevice device, const VkCommandPool command_pool, const VkCommandBuffer command_buffer,
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
    device_data->object_map[kVulkanObjectTypeCommandBuffer][HandleToUint64(command_buffer)] = pNewObjNode;
    device_data->num_objects[kVulkanObjectTypeCommandBuffer]++;
    device_data->num_total_objects++;
}

bool ValidateCommandBuffer(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    uint64_t object_handle = HandleToUint64(command_buffer);
    if (device_data->object_map[kVulkanObjectTypeCommandBuffer].find(object_handle) !=
        device_data->object_map[kVulkanObjectTypeCommandBuffer].end()) {
        ObjTrackState *pNode = device_data->object_map[kVulkanObjectTypeCommandBuffer][HandleToUint64(command_buffer)];

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

void AllocateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
            HandleToUint64(descriptor_set), kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64,
            object_track_index++, "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT", HandleToUint64(descriptor_set));

    ObjTrackState *pNewObjNode = new ObjTrackState;
    pNewObjNode->object_type = kVulkanObjectTypeDescriptorSet;
    pNewObjNode->status = OBJSTATUS_NONE;
    pNewObjNode->handle = HandleToUint64(descriptor_set);
    pNewObjNode->parent_object = HandleToUint64(descriptor_pool);
    device_data->object_map[kVulkanObjectTypeDescriptorSet][HandleToUint64(descriptor_set)] = pNewObjNode;
    device_data->num_objects[kVulkanObjectTypeDescriptorSet]++;
    device_data->num_total_objects++;
}

bool ValidateDescriptorSet(VkDevice device, VkDescriptorPool descriptor_pool, VkDescriptorSet descriptor_set) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    uint64_t object_handle = HandleToUint64(descriptor_set);
    auto dsItem = device_data->object_map[kVulkanObjectTypeDescriptorSet].find(object_handle);
    if (dsItem != device_data->object_map[kVulkanObjectTypeDescriptorSet].end()) {
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
static bool ValidateDescriptorWrite(DispObj disp, VkWriteDescriptorSet const *desc, bool isPush) {
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

VKAPI_ATTR void VKAPI_CALL CmdPushDescriptorSetKHR(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
                                                   VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount,
                                                   const VkWriteDescriptorSet *pDescriptorWrites) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |=
            ValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false,
                           "VUID-vkCmdPushDescriptorSetKHR-commandBuffer-parameter", "VUID-vkCmdPushDescriptorSetKHR-commonparent");
        skip |= ValidateObject(commandBuffer, layout, kVulkanObjectTypePipelineLayout, false,
                               "VUID-vkCmdPushDescriptorSetKHR-layout-parameter", "VUID-vkCmdPushDescriptorSetKHR-commonparent");
        if (pDescriptorWrites) {
            for (uint32_t index0 = 0; index0 < descriptorWriteCount; ++index0) {
                skip |= ValidateDescriptorWrite(commandBuffer, &pDescriptorWrites[index0], true);
            }
        }
    }
    if (skip) return;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    device_data->device_dispatch_table.CmdPushDescriptorSetKHR(commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount,
                                                               pDescriptorWrites);
}

void CreateQueue(VkDevice device, VkQueue vkObj) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);

    log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,
            HandleToUint64(vkObj), kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64,
            object_track_index++, "VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT", HandleToUint64(vkObj));

    ObjTrackState *p_obj_node = NULL;
    auto queue_item = device_data->object_map[kVulkanObjectTypeQueue].find(HandleToUint64(vkObj));
    if (queue_item == device_data->object_map[kVulkanObjectTypeQueue].end()) {
        p_obj_node = new ObjTrackState;
        device_data->object_map[kVulkanObjectTypeQueue][HandleToUint64(vkObj)] = p_obj_node;
        device_data->num_objects[kVulkanObjectTypeQueue]++;
        device_data->num_total_objects++;
    } else {
        p_obj_node = queue_item->second;
    }
    p_obj_node->object_type = kVulkanObjectTypeQueue;
    p_obj_node->status = OBJSTATUS_NONE;
    p_obj_node->handle = HandleToUint64(vkObj);
}

void CreateSwapchainImageObject(VkDevice dispatchable_object, VkImage swapchain_image, VkSwapchainKHR swapchain) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(dispatchable_object), layer_data_map);
    log_msg(device_data->report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
            HandleToUint64(swapchain_image), kVUID_ObjectTracker_Info, "OBJ[0x%" PRIxLEAST64 "] : CREATE %s object 0x%" PRIxLEAST64,
            object_track_index++, "SwapchainImage", HandleToUint64(swapchain_image));

    ObjTrackState *pNewObjNode = new ObjTrackState;
    pNewObjNode->object_type = kVulkanObjectTypeImage;
    pNewObjNode->status = OBJSTATUS_NONE;
    pNewObjNode->handle = HandleToUint64(swapchain_image);
    pNewObjNode->parent_object = HandleToUint64(swapchain);
    device_data->swapchainImageMap[HandleToUint64(swapchain_image)] = pNewObjNode;
}

void DeviceReportUndestroyedObjects(VkDevice device, VulkanObjectType object_type, const std::string &error_code) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    for (const auto &item : device_data->object_map[object_type]) {
        const ObjTrackState *object_info = item.second;
        log_msg(device_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, get_debug_report_enum[object_type], object_info->handle,
                error_code, "OBJ ERROR : For device 0x%" PRIxLEAST64 ", %s object 0x%" PRIxLEAST64 " has not been destroyed.",
                HandleToUint64(device), object_string[object_type], object_info->handle);
    }
}

void DeviceDestroyUndestroyedObjects(VkDevice device, VulkanObjectType object_type) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    while (!device_data->object_map[object_type].empty()) {
        auto item = device_data->object_map[object_type].begin();

        ObjTrackState *object_info = item->second;
        DestroyObjectSilently(device, object_info->handle, object_type);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    std::unique_lock<std::mutex> lock(global_lock);

    dispatch_key key = get_dispatch_key(instance);
    layer_data *instance_data = GetLayerDataPtr(key, layer_data_map);

    // Enable the temporary callback(s) here to catch cleanup issues:
    if (instance_data->num_tmp_debug_messengers > 0) {
        layer_enable_tmp_debug_messengers(instance_data->report_data, instance_data->num_tmp_debug_messengers,
                                          instance_data->tmp_messenger_create_infos, instance_data->tmp_debug_messengers);
    }
    if (instance_data->num_tmp_report_callbacks > 0) {
        layer_enable_tmp_report_callbacks(instance_data->report_data, instance_data->num_tmp_report_callbacks,
                                          instance_data->tmp_report_create_infos, instance_data->tmp_report_callbacks);
    }

    // TODO: The instance handle can not be validated here. The loader will likely have to validate it.
    ValidateObject(instance, instance, kVulkanObjectTypeInstance, true, "VUID-vkDestroyInstance-instance-parameter",
                   kVUIDUndefined);

    // Destroy physical devices
    for (auto iit = instance_data->object_map[kVulkanObjectTypePhysicalDevice].begin();
         iit != instance_data->object_map[kVulkanObjectTypePhysicalDevice].end();) {
        ObjTrackState *pNode = iit->second;
        VkPhysicalDevice physical_device = reinterpret_cast<VkPhysicalDevice>(pNode->handle);

        DestroyObject(instance, physical_device, kVulkanObjectTypePhysicalDevice, nullptr, kVUIDUndefined, kVUIDUndefined);
        iit = instance_data->object_map[kVulkanObjectTypePhysicalDevice].begin();
    }

    // Destroy child devices
    for (auto iit = instance_data->object_map[kVulkanObjectTypeDevice].begin();
         iit != instance_data->object_map[kVulkanObjectTypeDevice].end();) {
        ObjTrackState *pNode = iit->second;

        VkDevice device = reinterpret_cast<VkDevice>(pNode->handle);
        VkDebugReportObjectTypeEXT debug_object_type = get_debug_report_enum[pNode->object_type];

        log_msg(instance_data->report_data, VK_DEBUG_REPORT_ERROR_BIT_EXT, debug_object_type, pNode->handle,
                kVUID_ObjectTracker_ObjectLeak, "OBJ ERROR : %s object 0x%" PRIxLEAST64 " has not been destroyed.",
                string_VkDebugReportObjectTypeEXT(debug_object_type), pNode->handle);

        // Report any remaining objects in LL
        ReportUndestroyedObjects(device, "VUID-vkDestroyInstance-instance-00629");
        DestroyUndestroyedObjects(device);

        DestroyObject(instance, device, kVulkanObjectTypeDevice, pAllocator, "VUID-vkDestroyInstance-instance-00630",
                      "VUID-vkDestroyInstance-instance-00631");
        iit = instance_data->object_map[kVulkanObjectTypeDevice].begin();
    }

    instance_data->object_map[kVulkanObjectTypeDevice].clear();
    instance_data->instance_dispatch_table.DestroyInstance(instance, pAllocator);

    // Disable and cleanup the temporary callback(s):
    layer_disable_tmp_debug_messengers(instance_data->report_data, instance_data->num_tmp_debug_messengers,
                                       instance_data->tmp_debug_messengers);
    layer_disable_tmp_report_callbacks(instance_data->report_data, instance_data->num_tmp_report_callbacks,
                                       instance_data->tmp_report_callbacks);
    if (instance_data->num_tmp_debug_messengers > 0) {
        layer_free_tmp_debug_messengers(instance_data->tmp_messenger_create_infos, instance_data->tmp_debug_messengers);
        instance_data->num_tmp_debug_messengers = 0;
    }
    if (instance_data->num_tmp_report_callbacks > 0) {
        layer_free_tmp_report_callbacks(instance_data->tmp_report_create_infos, instance_data->tmp_report_callbacks);
        instance_data->num_tmp_report_callbacks = 0;
    }

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

    DestroyObject(instance, instance, kVulkanObjectTypeInstance, pAllocator, "VUID-vkDestroyInstance-instance-00630",
                  "VUID-vkDestroyInstance-instance-00631");

    layer_debug_utils_destroy_instance(instance_data->report_data);
    FreeLayerDataPtr(key, layer_data_map);

    lock.unlock();
}

VKAPI_ATTR void VKAPI_CALL DestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    std::unique_lock<std::mutex> lock(global_lock);
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    ValidateObject(device, device, kVulkanObjectTypeDevice, true, "VUID-vkDestroyDevice-device-parameter", kVUIDUndefined);
    DestroyObject(device_data->instance, device, kVulkanObjectTypeDevice, pAllocator, "VUID-vkDestroyDevice-device-00379",
                  "VUID-vkDestroyDevice-device-00380");

    // Report any remaining objects associated with this VkDevice object in LL
    ReportUndestroyedObjects(device, "VUID-vkDestroyDevice-device-00378");
    DestroyUndestroyedObjects(device);

    // Clean up Queue's MemRef Linked Lists
    DestroyQueueDataStructures(device);

    lock.unlock();
    dispatch_key key = get_dispatch_key(device);
    device_data->device_dispatch_table.DestroyDevice(device, pAllocator);
    FreeLayerDataPtr(key, layer_data_map);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue) {
    std::unique_lock<std::mutex> lock(global_lock);
    ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceQueue-device-parameter", kVUIDUndefined);
    lock.unlock();

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);

    lock.lock();
    CreateQueue(device, *pQueue);
    AddQueueInfo(device, queueFamilyIndex, *pQueue);
}

VKAPI_ATTR void VKAPI_CALL GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2 *pQueueInfo, VkQueue *pQueue) {
    std::unique_lock<std::mutex> lock(global_lock);
    ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetDeviceQueue2-device-parameter", kVUIDUndefined);
    lock.unlock();

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.GetDeviceQueue2(device, pQueueInfo, pQueue);

    lock.lock();
    if (*pQueue != VK_NULL_HANDLE) {
        CreateQueue(device, *pQueue);
        AddQueueInfo(device, pQueueInfo->queueFamilyIndex, *pQueue);
    }
}

VKAPI_ATTR void VKAPI_CALL UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                const VkCopyDescriptorSet *pDescriptorCopies) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
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
    }
    if (skip) {
        return;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    device_data->device_dispatch_table.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount,
                                                            pDescriptorCopies);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                      const VkComputePipelineCreateInfo *pCreateInfos,
                                                      const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkCreateComputePipelines-device-parameter",
                           kVUIDUndefined);
    if (pCreateInfos) {
        for (uint32_t idx0 = 0; idx0 < createInfoCount; ++idx0) {
            if (pCreateInfos[idx0].basePipelineHandle) {
                skip |=
                    ValidateObject(device, pCreateInfos[idx0].basePipelineHandle, kVulkanObjectTypePipeline, true,
                                   "VUID-VkComputePipelineCreateInfo-flags-00697", "VUID-VkComputePipelineCreateInfo-commonparent");
            }
            if (pCreateInfos[idx0].layout) {
                skip |= ValidateObject(device, pCreateInfos[idx0].layout, kVulkanObjectTypePipelineLayout, false,
                                       "VUID-VkComputePipelineCreateInfo-layout-parameter",
                                       "VUID-VkComputePipelineCreateInfo-commonparent");
            }
            if (pCreateInfos[idx0].stage.module) {
                skip |= ValidateObject(device, pCreateInfos[idx0].stage.module, kVulkanObjectTypeShaderModule, false,
                                       "VUID-VkPipelineShaderStageCreateInfo-module-parameter", kVUIDUndefined);
            }
        }
    }
    if (pipelineCache) {
        skip |= ValidateObject(device, pipelineCache, kVulkanObjectTypePipelineCache, true,
                               "VUID-vkCreateComputePipelines-pipelineCache-parameter",
                               "VUID-vkCreateComputePipelines-pipelineCache-parent");
    }
    lock.unlock();
    if (skip) {
        for (uint32_t i = 0; i < createInfoCount; i++) {
            pPipelines[i] = VK_NULL_HANDLE;
        }
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.CreateComputePipelines(device, pipelineCache, createInfoCount,
                                                                                pCreateInfos, pAllocator, pPipelines);

    lock.lock();
    for (uint32_t idx1 = 0; idx1 < createInfoCount; ++idx1) {
        if (pPipelines[idx1] != VK_NULL_HANDLE) {
            CreateObject(device, pPipelines[idx1], kVulkanObjectTypePipeline, pAllocator);
        }
    }
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                   VkDescriptorPoolResetFlags flags) {
    bool skip = false;
    std::unique_lock<std::mutex> lock(global_lock);
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkResetDescriptorPool-device-parameter",
                           kVUIDUndefined);
    skip |=
        ValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                       "VUID-vkResetDescriptorPool-descriptorPool-parameter", "VUID-vkResetDescriptorPool-descriptorPool-parent");
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    // A DescriptorPool's descriptor sets are implicitly deleted when the pool is reset.
    // Remove this pool's descriptor sets from our descriptorSet map.
    auto itr = device_data->object_map[kVulkanObjectTypeDescriptorSet].begin();
    while (itr != device_data->object_map[kVulkanObjectTypeDescriptorSet].end()) {
        ObjTrackState *pNode = (*itr).second;
        auto del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(descriptorPool)) {
            DestroyObject(device, (VkDescriptorSet)((*del_itr).first), kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                          kVUIDUndefined);
        }
    }
    lock.unlock();
    VkResult result = device_data->device_dispatch_table.ResetDescriptorPool(device, descriptorPool, flags);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL BeginCommandBuffer(VkCommandBuffer command_buffer, const VkCommandBufferBeginInfo *begin_info) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(command_buffer), layer_data_map);
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= ValidateObject(command_buffer, command_buffer, kVulkanObjectTypeCommandBuffer, false,
                               "VUID-vkBeginCommandBuffer-commandBuffer-parameter", kVUIDUndefined);
        if (begin_info) {
            ObjTrackState *pNode = device_data->object_map[kVulkanObjectTypeCommandBuffer][HandleToUint64(command_buffer)];
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
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    VkResult result = device_data->device_dispatch_table.BeginCommandBuffer(command_buffer, begin_info);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugReportCallbackEXT(VkInstance instance,
                                                            const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugReportCallbackEXT *pCallback) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
    if (VK_SUCCESS == result) {
        result = layer_create_report_callback(instance_data->report_data, false, pCreateInfo, pAllocator, pCallback);
        CreateObject(instance, *pCallback, kVulkanObjectTypeDebugReportCallbackEXT, pAllocator);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT msgCallback,
                                                         const VkAllocationCallbacks *pAllocator) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    instance_data->instance_dispatch_table.DestroyDebugReportCallbackEXT(instance, msgCallback, pAllocator);
    layer_destroy_report_callback(instance_data->report_data, msgCallback, pAllocator);
    DestroyObject(instance, msgCallback, kVulkanObjectTypeDebugReportCallbackEXT, pAllocator,
                  "VUID-vkDestroyDebugReportCallbackEXT-instance-01242", "VUID-vkDestroyDebugReportCallbackEXT-instance-01243");
}

VKAPI_ATTR void VKAPI_CALL DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                 VkDebugReportObjectTypeEXT objType, uint64_t object, size_t location,
                                                 int32_t msgCode, const char *pLayerPrefix, const char *pMsg) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    instance_data->instance_dispatch_table.DebugReportMessageEXT(instance, flags, objType, object, location, msgCode, pLayerPrefix,
                                                                 pMsg);
}

// VK_EXT_debug_utils commands
VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (pNameInfo->pObjectName) {
        lock.lock();
        dev_data->report_data->debugUtilsObjectNameMap->insert(
            std::make_pair<uint64_t, std::string>((uint64_t &&) pNameInfo->objectHandle, pNameInfo->pObjectName));
        lock.unlock();
    } else {
        lock.lock();
        dev_data->report_data->debugUtilsObjectNameMap->erase(pNameInfo->objectHandle);
        lock.unlock();
    }
    VkResult result = dev_data->device_dispatch_table.SetDebugUtilsObjectNameEXT(device, pNameInfo);
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL SetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = dev_data->device_dispatch_table.SetDebugUtilsObjectTagEXT(device, pTagInfo);
    return result;
}

VKAPI_ATTR void VKAPI_CALL QueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(queue, queue, kVulkanObjectTypeQueue, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!skip) {
        lock.lock();
        BeginQueueDebugUtilsLabel(dev_data->report_data, queue, pLabelInfo);
        lock.unlock();
        dev_data->device_dispatch_table.QueueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL QueueEndDebugUtilsLabelEXT(VkQueue queue) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(queue, queue, kVulkanObjectTypeQueue, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!skip) {
        dev_data->device_dispatch_table.QueueEndDebugUtilsLabelEXT(queue);
        lock.lock();
        EndQueueDebugUtilsLabel(dev_data->report_data, queue);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL QueueInsertDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT *pLabelInfo) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(queue, queue, kVulkanObjectTypeQueue, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(queue), layer_data_map);
    if (!skip) {
        lock.lock();
        InsertQueueDebugUtilsLabel(dev_data->report_data, queue, pLabelInfo);
        lock.unlock();
        dev_data->device_dispatch_table.QueueInsertDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdBeginDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!skip) {
        lock.lock();
        BeginCmdDebugUtilsLabel(dev_data->report_data, commandBuffer, pLabelInfo);
        lock.unlock();
        dev_data->device_dispatch_table.CmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL CmdEndDebugUtilsLabelEXT(VkCommandBuffer commandBuffer) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!skip) {
        dev_data->device_dispatch_table.CmdEndDebugUtilsLabelEXT(commandBuffer);
        lock.lock();
        EndCmdDebugUtilsLabel(dev_data->report_data, commandBuffer);
        lock.unlock();
    }
}

VKAPI_ATTR void VKAPI_CALL CmdInsertDebugUtilsLabelEXT(VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT *pLabelInfo) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(commandBuffer, commandBuffer, kVulkanObjectTypeCommandBuffer, false, kVUIDUndefined, kVUIDUndefined);
    lock.unlock();
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(commandBuffer), layer_data_map);
    if (!skip) {
        lock.lock();
        InsertCmdDebugUtilsLabel(dev_data->report_data, commandBuffer, pLabelInfo);
        lock.unlock();
        dev_data->device_dispatch_table.CmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                            const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator,
                                                            VkDebugUtilsMessengerEXT *pMessenger) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.CreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    if (VK_SUCCESS == result) {
        result = layer_create_messenger_callback(instance_data->report_data, false, pCreateInfo, pAllocator, pMessenger);
        CreateObject(instance, *pMessenger, kVulkanObjectTypeDebugUtilsMessengerEXT, pAllocator);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                         const VkAllocationCallbacks *pAllocator) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    instance_data->instance_dispatch_table.DestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    layer_destroy_messenger_callback(instance_data->report_data, messenger, pAllocator);
    DestroyObject(instance, messenger, kVulkanObjectTypeDebugUtilsMessengerEXT, pAllocator, kVUIDUndefined, kVUIDUndefined);
}

VKAPI_ATTR void VKAPI_CALL SubmitDebugUtilsMessageEXT(VkInstance instance, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    auto instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    instance_data->instance_dispatch_table.SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, pCallbackData);
}

static const VkExtensionProperties instance_extensions[] = {{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
                                                            {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION}};

static const VkLayerProperties globalLayerProps = {"VK_LAYER_LUNARG_object_tracker",
                                                   VK_LAYER_API_VERSION,  // specVersion
                                                   1,                     // implementationVersion
                                                   "LunarG Validation Layer"};

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &globalLayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                              VkLayerProperties *pProperties) {
    return util_GetLayerProperties(1, &globalLayerProps, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                    VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, globalLayerProps.layerName))
        return util_GetExtensionProperties(1, instance_extensions, pCount, pProperties);

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                  uint32_t *pCount, VkExtensionProperties *pProperties) {
    if (pLayerName && !strcmp(pLayerName, globalLayerProps.layerName))
        return util_GetExtensionProperties(0, nullptr, pCount, pProperties);

    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    return instance_data->instance_dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, NULL, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                            const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    std::lock_guard<std::mutex> lock(global_lock);
    bool skip = ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                               "VUID-vkCreateDevice-physicalDevice-parameter", kVUIDUndefined);
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    layer_data *phy_dev_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(phy_dev_data->instance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(*pDevice), layer_data_map);
    device_data->report_data = layer_debug_utils_create_device(phy_dev_data->report_data, *pDevice);
    layer_init_device_dispatch_table(*pDevice, &device_data->device_dispatch_table, fpGetDeviceProcAddr);

    // Save pCreateInfo device extension list for GetDeviceProcAddr()
    for (uint32_t extn = 0; extn < pCreateInfo->enabledExtensionCount; extn++) {
        device_data->device_extension_set.insert(pCreateInfo->ppEnabledExtensionNames[extn]);
    }

    // Add link back to physDev
    device_data->physical_device = physicalDevice;
    device_data->instance = phy_dev_data->instance;

    CreateObject(phy_dev_data->instance, *pDevice, kVulkanObjectTypeDevice, pAllocator);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount,
                                                     VkImage *pSwapchainImages) {
    bool skip = false;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkGetSwapchainImagesKHR-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, false,
                           "VUID-vkGetSwapchainImagesKHR-swapchain-parameter", kVUIDUndefined);
    lock.unlock();
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result =
        device_data->device_dispatch_table.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    if (pSwapchainImages != NULL) {
        lock.lock();
        for (uint32_t i = 0; i < *pSwapchainImageCount; i++) {
            CreateSwapchainImageObject(device, pSwapchainImages[i], swapchain);
        }
        lock.unlock();
    }
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator,
                                                         VkDescriptorSetLayout *pSetLayout) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
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
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
    if (VK_SUCCESS == result) {
        std::lock_guard<std::mutex> lock(global_lock);
        CreateObject(device, *pSetLayout, kVulkanObjectTypeDescriptorSetLayout, pAllocator);
    }
    return result;
}

static inline bool ValidateSamplerObjects(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo) {
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

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                         VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false,
                               "VUID-vkGetDescriptorSetLayoutSupport-device-parameter", kVUIDUndefined);
        if (pCreateInfo) {
            skip |= ValidateSamplerObjects(device, pCreateInfo);
        }
    }
    if (skip) return;
    GetLayerDataPtr(get_dispatch_key(device), layer_data_map)
        ->device_dispatch_table.GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

VKAPI_ATTR void VKAPI_CALL GetDescriptorSetLayoutSupportKHR(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                            VkDescriptorSetLayoutSupport *pSupport) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false,
                               "VUID-vkGetDescriptorSetLayoutSupportKHR-device-parameter", kVUIDUndefined);
        if (pCreateInfo) {
            skip |= ValidateSamplerObjects(device, pCreateInfo);
        }
    }
    if (skip) return;
    GetLayerDataPtr(get_dispatch_key(device), layer_data_map)
        ->device_dispatch_table.GetDescriptorSetLayoutSupportKHR(device, pCreateInfo, pSupport);
}

VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                  uint32_t *pQueueFamilyPropertyCount,
                                                                  VkQueueFamilyProperties *pQueueFamilyProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                               "VUID-vkGetPhysicalDeviceQueueFamilyProperties-physicalDevice-parameter", kVUIDUndefined);
    }
    if (skip) {
        return;
    }
    auto instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    instance_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount,
                                                                                  pQueueFamilyProperties);
    std::lock_guard<std::mutex> lock(global_lock);
    if (pQueueFamilyProperties != NULL) {
        if (instance_data->queue_family_properties.size() < *pQueueFamilyPropertyCount) {
            instance_data->queue_family_properties.resize(*pQueueFamilyPropertyCount);
        }
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; i++) {
            instance_data->queue_family_properties[i] = pQueueFamilyProperties[i];
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
                                              VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);

    assert(chain_info->u.pLayerInfo);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }

    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(*pInstance), layer_data_map);
    instance_data->instance = *pInstance;
    layer_init_instance_dispatch_table(*pInstance, &instance_data->instance_dispatch_table, fpGetInstanceProcAddr);

    // Look for one or more debug report create info structures, and copy the
    // callback(s) for each one found (for use by vkDestroyInstance)
    layer_copy_tmp_debug_messengers(pCreateInfo->pNext, &instance_data->num_tmp_debug_messengers,
                                    &instance_data->tmp_messenger_create_infos, &instance_data->tmp_debug_messengers);
    layer_copy_tmp_report_callbacks(pCreateInfo->pNext, &instance_data->num_tmp_report_callbacks,
                                    &instance_data->tmp_report_create_infos, &instance_data->tmp_report_callbacks);

    instance_data->report_data =
        debug_utils_create_instance(&instance_data->instance_dispatch_table, *pInstance, pCreateInfo->enabledExtensionCount,
                                    pCreateInfo->ppEnabledExtensionNames);

    InitObjectTracker(instance_data, pAllocator);

    CreateObject(*pInstance, *pInstance, kVulkanObjectTypeInstance, pAllocator);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                        VkPhysicalDevice *pPhysicalDevices) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(instance, instance, kVulkanObjectTypeInstance, false,
                           "VUID-vkEnumeratePhysicalDevices-instance-parameter", kVUIDUndefined);
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
    lock.lock();
    if (result == VK_SUCCESS) {
        if (pPhysicalDevices) {
            for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
                CreateObject(instance, pPhysicalDevices[i], kVulkanObjectTypePhysicalDevice, nullptr);
            }
        }
    }
    lock.unlock();
    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
                                                      VkCommandBuffer *pCommandBuffers) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkAllocateCommandBuffers-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, pAllocateInfo->commandPool, kVulkanObjectTypeCommandPool, false,
                           "VUID-VkCommandBufferAllocateInfo-commandPool-parameter", kVUIDUndefined);
    lock.unlock();

    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);

    lock.lock();
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
        AllocateCommandBuffer(device, pAllocateInfo->commandPool, pCommandBuffers[i], pAllocateInfo->level);
    }
    lock.unlock();

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                                      VkDescriptorSet *pDescriptorSets) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
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
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }

    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    VkResult result = device_data->device_dispatch_table.AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);

    if (VK_SUCCESS == result) {
        lock.lock();
        for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; i++) {
            AllocateDescriptorSet(device, pAllocateInfo->descriptorPool, pDescriptorSets[i]);
        }
        lock.unlock();
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
                                              const VkCommandBuffer *pCommandBuffers) {
    bool skip = false;
    std::unique_lock<std::mutex> lock(global_lock);
    ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFreeCommandBuffers-device-parameter", kVUIDUndefined);
    ValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, false, "VUID-vkFreeCommandBuffers-commandPool-parameter",
                   "VUID-vkFreeCommandBuffers-commandPool-parent");
    for (uint32_t i = 0; i < commandBufferCount; i++) {
        if (pCommandBuffers[i] != VK_NULL_HANDLE) {
            skip |= ValidateCommandBuffer(device, commandPool, pCommandBuffers[i]);
        }
    }

    for (uint32_t i = 0; i < commandBufferCount; i++) {
        DestroyObject(device, pCommandBuffers[i], kVulkanObjectTypeCommandBuffer, nullptr, kVUIDUndefined, kVUIDUndefined);
    }

    lock.unlock();
    if (!skip) {
        layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
        device_data->device_dispatch_table.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
    }
}

VKAPI_ATTR void VKAPI_CALL DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    std::unique_lock<std::mutex> lock(global_lock);
    // A swapchain's images are implicitly deleted when the swapchain is deleted.
    // Remove this swapchain's images from our map of such images.
    std::unordered_map<uint64_t, ObjTrackState *>::iterator itr = device_data->swapchainImageMap.begin();
    while (itr != device_data->swapchainImageMap.end()) {
        ObjTrackState *pNode = (*itr).second;
        if (pNode->parent_object == HandleToUint64(swapchain)) {
            delete pNode;
            auto delete_item = itr++;
            device_data->swapchainImageMap.erase(delete_item);
        } else {
            ++itr;
        }
    }
    DestroyObject(device, swapchain, kVulkanObjectTypeSwapchainKHR, pAllocator, "VUID-vkDestroySwapchainKHR-swapchain-01283",
                  "VUID-vkDestroySwapchainKHR-swapchain-01284");
    lock.unlock();

    device_data->device_dispatch_table.DestroySwapchainKHR(device, swapchain, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                  const VkDescriptorSet *pDescriptorSets) {
    bool skip = false;
    VkResult result = VK_ERROR_VALIDATION_FAILED_EXT;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkFreeDescriptorSets-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, false,
                           "VUID-vkFreeDescriptorSets-descriptorPool-parameter", "VUID-vkFreeDescriptorSets-descriptorPool-parent");
    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        if (pDescriptorSets[i] != VK_NULL_HANDLE) {
            skip |= ValidateDescriptorSet(device, descriptorPool, pDescriptorSets[i]);
        }
    }

    for (uint32_t i = 0; i < descriptorSetCount; i++) {
        DestroyObject(device, pDescriptorSets[i], kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined, kVUIDUndefined);
    }

    lock.unlock();
    if (!skip) {
        layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
        result = device_data->device_dispatch_table.FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool,
                                                 const VkAllocationCallbacks *pAllocator) {
    bool skip = VK_FALSE;
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyDescriptorPool-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, true,
                           "VUID-vkDestroyDescriptorPool-descriptorPool-parameter",
                           "VUID-vkDestroyDescriptorPool-descriptorPool-parent");
    lock.unlock();
    if (skip) {
        return;
    }
    // A DescriptorPool's descriptor sets are implicitly deleted when the pool is deleted.
    // Remove this pool's descriptor sets from our descriptorSet map.
    lock.lock();
    std::unordered_map<uint64_t, ObjTrackState *>::iterator itr = device_data->object_map[kVulkanObjectTypeDescriptorSet].begin();
    while (itr != device_data->object_map[kVulkanObjectTypeDescriptorSet].end()) {
        ObjTrackState *pNode = (*itr).second;
        auto del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(descriptorPool)) {
            DestroyObject(device, (VkDescriptorSet)((*del_itr).first), kVulkanObjectTypeDescriptorSet, nullptr, kVUIDUndefined,
                          kVUIDUndefined);
        }
    }
    DestroyObject(device, descriptorPool, kVulkanObjectTypeDescriptorPool, pAllocator,
                  "VUID-vkDestroyDescriptorPool-descriptorPool-00304", "VUID-vkDestroyDescriptorPool-descriptorPool-00305");
    lock.unlock();
    device_data->device_dispatch_table.DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    bool skip = false;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDestroyCommandPool-device-parameter",
                           kVUIDUndefined);
    skip |= ValidateObject(device, commandPool, kVulkanObjectTypeCommandPool, true,
                           "VUID-vkDestroyCommandPool-commandPool-parameter", "VUID-vkDestroyCommandPool-commandPool-parent");
    lock.unlock();
    if (skip) {
        return;
    }
    lock.lock();
    // A CommandPool's command buffers are implicitly deleted when the pool is deleted.
    // Remove this pool's cmdBuffers from our cmd buffer map.
    auto itr = device_data->object_map[kVulkanObjectTypeCommandBuffer].begin();
    auto del_itr = itr;
    while (itr != device_data->object_map[kVulkanObjectTypeCommandBuffer].end()) {
        ObjTrackState *pNode = (*itr).second;
        del_itr = itr++;
        if (pNode->parent_object == HandleToUint64(commandPool)) {
            skip |= ValidateCommandBuffer(device, commandPool, reinterpret_cast<VkCommandBuffer>((*del_itr).first));
            DestroyObject(device, reinterpret_cast<VkCommandBuffer>((*del_itr).first), kVulkanObjectTypeCommandBuffer, nullptr,
                          kVUIDUndefined, kVUIDUndefined);
        }
    }
    DestroyObject(device, commandPool, kVulkanObjectTypeCommandPool, pAllocator, "VUID-vkDestroyCommandPool-commandPool-00042",
                  "VUID-vkDestroyCommandPool-commandPool-00043");
    lock.unlock();
    device_data->device_dispatch_table.DestroyCommandPool(device, commandPool, pAllocator);
}

// Note: This is the core version of this routine.  The extension version is below.
VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice,
                                                                   uint32_t *pQueueFamilyPropertyCount,
                                                                   VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |=
            ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, kVUIDUndefined, kVUIDUndefined);
    }
    if (skip) {
        return;
    }
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    instance_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount,
                                                                                   pQueueFamilyProperties);
    std::lock_guard<std::mutex> lock(global_lock);
    if (pQueueFamilyProperties != NULL) {
        if (instance_data->queue_family_properties.size() < *pQueueFamilyPropertyCount) {
            instance_data->queue_family_properties.resize(*pQueueFamilyPropertyCount);
        }
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; i++) {
            instance_data->queue_family_properties[i] = pQueueFamilyProperties[i].queueFamilyProperties;
        }
    }
}

// Note: This is the extension version of this routine.  The core version is above.
VKAPI_ATTR void VKAPI_CALL GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physicalDevice,
                                                                      uint32_t *pQueueFamilyPropertyCount,
                                                                      VkQueueFamilyProperties2KHR *pQueueFamilyProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |=
            ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, kVUIDUndefined, kVUIDUndefined);
    }
    if (skip) {
        return;
    }
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    instance_data->instance_dispatch_table.GetPhysicalDeviceQueueFamilyProperties2KHR(physicalDevice, pQueueFamilyPropertyCount,
                                                                                      pQueueFamilyProperties);
    std::lock_guard<std::mutex> lock(global_lock);
    if (pQueueFamilyProperties != NULL) {
        if (instance_data->queue_family_properties.size() < *pQueueFamilyPropertyCount) {
            instance_data->queue_family_properties.resize(*pQueueFamilyPropertyCount);
        }
        for (uint32_t i = 0; i < *pQueueFamilyPropertyCount; i++) {
            instance_data->queue_family_properties[i] = pQueueFamilyProperties[i].queueFamilyProperties;
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                     VkDisplayPropertiesKHR *pProperties) {
    bool skip = false;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                           "VUID-vkGetPhysicalDeviceDisplayPropertiesKHR-physicalDevice-parameter", kVUIDUndefined);
    lock.unlock();

    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);

    lock.lock();
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObject(physicalDevice, pProperties[i].display, kVulkanObjectTypeDisplayKHR, nullptr);
        }
    }
    lock.unlock();

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                           uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties) {
    bool skip = false;
    std::unique_lock<std::mutex> lock(global_lock);
    skip |= ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                           "VUID-vkGetDisplayModePropertiesKHR-physicalDevice-parameter", kVUIDUndefined);
    skip |= ValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false,
                           "VUID-vkGetDisplayModePropertiesKHR-display-parameter", kVUIDUndefined);
    lock.unlock();

    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);

    lock.lock();
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            CreateObject(physicalDevice, pProperties[i].displayMode, kVulkanObjectTypeDisplayModeKHR, nullptr);
        }
    }

    lock.unlock();

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL DebugMarkerSetObjectNameEXT(VkDevice device, const VkDebugMarkerObjectNameInfoEXT *pNameInfo) {
    bool skip = VK_FALSE;
    std::unique_lock<std::mutex> lock(global_lock);
    layer_data *dev_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (pNameInfo->pObjectName) {
        dev_data->report_data->debugObjectNameMap->insert(
            std::make_pair<uint64_t, std::string>((uint64_t &&) pNameInfo->object, pNameInfo->pObjectName));
    } else {
        dev_data->report_data->debugObjectNameMap->erase(pNameInfo->object);
    }
    skip |= ValidateObject(device, device, kVulkanObjectTypeDevice, false, "VUID-vkDebugMarkerSetObjectNameEXT-device-parameter",
                           kVUIDUndefined);
    lock.unlock();
    if (skip) {
        return VK_ERROR_VALIDATION_FAILED_EXT;
    }
    VkResult result = dev_data->device_dispatch_table.DebugMarkerSetObjectNameEXT(device, pNameInfo);
    return result;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    assert(instance);
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (instance_data->instance_dispatch_table.GetPhysicalDeviceProcAddr == NULL) {
        return NULL;
    }
    return instance_data->instance_dispatch_table.GetPhysicalDeviceProcAddr(instance, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetDeviceProcAddr(VkDevice device, const char *funcName) {
    layer_data *device_data = GetLayerDataPtr(get_dispatch_key(device), layer_data_map);
    if (!ApiParentExtensionEnabled(funcName, device_data->device_extension_set)) {
        return nullptr;
    }
    const auto item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    if (!device_data->device_dispatch_table.GetDeviceProcAddr) return NULL;
    return device_data->device_dispatch_table.GetDeviceProcAddr(device, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetInstanceProcAddr(VkInstance instance, const char *funcName) {
    const auto item = name_to_funcptr_map.find(funcName);
    if (item != name_to_funcptr_map.end()) {
        return reinterpret_cast<PFN_vkVoidFunction>(item->second);
    }
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(instance), layer_data_map);
    if (!instance_data->instance_dispatch_table.GetInstanceProcAddr) return nullptr;
    return instance_data->instance_dispatch_table.GetInstanceProcAddr(instance, funcName);
}

VKAPI_ATTR VkResult VKAPI_CALL GetPhysicalDeviceDisplayProperties2KHR(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
                                                                      VkDisplayProperties2KHR *pProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |=
            ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, kVUIDUndefined, kVUIDUndefined);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetPhysicalDeviceDisplayProperties2KHR(physicalDevice, pPropertyCount, pProperties);
    if (pProperties && (VK_SUCCESS == result || VK_INCOMPLETE == result)) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (uint32_t index = 0; index < *pPropertyCount; ++index) {
            CreateObject(physicalDevice, pProperties[index].displayProperties.display, kVulkanObjectTypeDisplayKHR, nullptr);
        }
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                   uint32_t *pDisplayCount, VkDisplayKHR *pDisplays) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |= ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false,
                               "VUID-vkGetDisplayPlaneSupportedDisplaysKHR-physicalDevice-parameter", kVUIDUndefined);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result = instance_data->instance_dispatch_table.GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex,
                                                                                                 pDisplayCount, pDisplays);
    if (pDisplays && (VK_SUCCESS == result || VK_INCOMPLETE == result)) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (uint32_t index = 0; index < *pDisplayCount; ++index) {
            CreateObject(physicalDevice, pDisplays[index], kVulkanObjectTypeDisplayKHR, nullptr);
        }
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL GetDisplayModeProperties2KHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display,
                                                            uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties) {
    bool skip = false;
    {
        std::lock_guard<std::mutex> lock(global_lock);
        skip |=
            ValidateObject(physicalDevice, physicalDevice, kVulkanObjectTypePhysicalDevice, false, kVUIDUndefined, kVUIDUndefined);
        skip |= ValidateObject(physicalDevice, display, kVulkanObjectTypeDisplayKHR, false, kVUIDUndefined, kVUIDUndefined);
    }
    if (skip) return VK_ERROR_VALIDATION_FAILED_EXT;
    layer_data *instance_data = GetLayerDataPtr(get_dispatch_key(physicalDevice), layer_data_map);
    VkResult result =
        instance_data->instance_dispatch_table.GetDisplayModeProperties2KHR(physicalDevice, display, pPropertyCount, pProperties);
    if (pProperties && (VK_SUCCESS == result || VK_INCOMPLETE == result)) {
        std::lock_guard<std::mutex> lock(global_lock);
        for (uint32_t index = 0; index < *pPropertyCount; ++index) {
            CreateObject(physicalDevice, pProperties[index].displayModeProperties.displayMode, kVulkanObjectTypeDisplayModeKHR,
                         nullptr);
        }
    }

    return result;
}

}  // namespace object_tracker

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount,
                                                                                      VkExtensionProperties *pProperties) {
    return object_tracker::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount,
                                                                                  VkLayerProperties *pProperties) {
    return object_tracker::EnumerateInstanceLayerProperties(pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount,
                                                                                VkLayerProperties *pProperties) {
    // The layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return object_tracker::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
    return object_tracker::GetDeviceProcAddr(dev, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return object_tracker::GetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice,
                                                                                    const char *pLayerName, uint32_t *pCount,
                                                                                    VkExtensionProperties *pProperties) {
    // The layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return object_tracker::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return object_tracker::GetPhysicalDeviceProcAddr(instance, funcName);
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

    if (pVersionStruct->loaderLayerInterfaceVersion < CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        object_tracker::loader_layer_if_version = pVersionStruct->loaderLayerInterfaceVersion;
    } else if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }

    return VK_SUCCESS;
}
