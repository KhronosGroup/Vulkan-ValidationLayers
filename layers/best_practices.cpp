/* Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
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
 * Author: Camden Stocker <camden@lunarg.com>
 */

#include "best_practices.h"
#include "layer_chassis_dispatch.h"
#include "best_practices_error_enums.h"

#include <string>
#include <iomanip>

// get the API name is proper format
std::string BestPractices::GetAPIVersionName(uint32_t version) const {
    std::stringstream version_name;
    uint32_t major = VK_VERSION_MAJOR(version);
    uint32_t minor = VK_VERSION_MINOR(version);
    uint32_t patch = VK_VERSION_PATCH(version);

    version_name << major << "." << minor << "." << patch << " (0x" << std::setfill('0') << std::setw(8) << std::hex << version
                 << ")";

    return version_name.str();
}

bool BestPractices::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                  VkInstance* pInstance) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceExtensionNames)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_BestPractices_CreateInstance_ExtensionMismatch,
                            "vkCreateInstance(): Attempting to enable Device Extension %s at CreateInstance time.",
                            pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }

    return skip;
}

void BestPractices::PreCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                VkInstance* pInstance) {
    instance_api_version = pCreateInfo->pApplicationInfo->apiVersion;
}

bool BestPractices::PreCallValidateCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) const {
    bool skip = false;

    // get API version of physical device passed when creating device.
    VkPhysicalDeviceProperties physical_device_properties{};
    DispatchGetPhysicalDeviceProperties(physicalDevice, &physical_device_properties);
    auto device_api_version = physical_device_properties.apiVersion;

    // check api versions and warn if instance api Version is higher than version on device.
    if (instance_api_version > device_api_version) {
        std::string inst_api_name = GetAPIVersionName(instance_api_version);
        std::string dev_api_name = GetAPIVersionName(device_api_version);

        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CreateDevice_API_Mismatch,
                        "vkCreateDevice(): API Version of current instance, %s is higher than API Version on device, %s",
                        inst_api_name.c_str(), dev_api_name.c_str());
    }

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kInstanceExtensionNames)) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                            kVUID_BestPractices_CreateInstance_ExtensionMismatch,
                            "vkCreateDevice(): Attempting to enable Instance Extension %s at CreateDevice time.",
                            pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }

    auto pd_state = GetPhysicalDeviceState(physicalDevice);
    if ((pd_state->vkGetPhysicalDeviceFeaturesState == UNCALLED) && (pCreateInfo->pEnabledFeatures != NULL)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CreateDevice_PDFeaturesNotCalled,
                        "vkCreateDevice() called before getting physical device features from vkGetPhysicalDeviceFeatures().");
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream bufferHex;
        bufferHex << "0x" << std::hex << HandleToUint64(pBuffer);

        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_BestPractices_SharingModeExclusive,
                    "Warning: Buffer (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                    "(queueFamilyIndexCount of %" PRIu32 ").",
                    bufferHex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImage* pImage) const {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream imageHex;
        imageHex << "0x" << std::hex << HandleToUint64(pImage);

        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_BestPractices_SharingModeExclusive,
                    "Warning: Image (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                    "(queueFamilyIndexCount of %" PRIu32 ").",
                    imageHex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) const {
    bool skip = false;

    auto physical_device_state = GetPhysicalDeviceState();

    if (physical_device_state->vkGetPhysicalDeviceSurfaceCapabilitiesKHRState == UNCALLED) {
        skip |= log_msg(
            report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
            kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
            "vkCreateSwapchainKHR() called before getting surface capabilities from vkGetPhysicalDeviceSurfaceCapabilitiesKHR().");
    }

    if (physical_device_state->vkGetPhysicalDeviceSurfacePresentModesKHRState != QUERY_DETAILS) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                        "vkCreateSwapchainKHR() called before getting surface present mode(s) from "
                        "vkGetPhysicalDeviceSurfacePresentModesKHR().");
    }

    if (physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState != QUERY_DETAILS) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_BestPractices_Swapchain_GetSurfaceNotCalled,
                    "vkCreateSwapchainKHR() called before getting surface format(s) from vkGetPhysicalDeviceSurfaceFormatsKHR().");
    }

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_SharingModeExclusive,
                        "Warning: A Swapchain is being created which specifies a sharing mode of VK_SHARING_MODE_EXCULSIVE while "
                        "specifying multiple queues (queueFamilyIndexCount of %" PRIu32 ").",
                        pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                             const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkSwapchainKHR* pSwapchains) const {
    bool skip = false;

    for (uint32_t i = 0; i < swapchainCount; i++) {
        if ((pCreateInfos[i].queueFamilyIndexCount > 1) && (pCreateInfos[i].imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_SharingModeExclusive,
                        "Warning: A shared swapchain (index %" PRIu32
                        ") is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple "
                        "queues (queueFamilyIndexCount of %" PRIu32 ").",
                        i, pCreateInfos[i].queueFamilyIndexCount);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) const {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        VkFormat format = pCreateInfo->pAttachments[i].format;
        if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            if ((FormatIsColor(format) || FormatHasDepth(format)) &&
                pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_BestPractices_RenderPass_Attatchment,
                                "Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and "
                                "initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                "image truely is undefined at the start of the render pass.");
            }
            if (FormatHasStencil(format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                kVUID_BestPractices_RenderPass_Attatchment,
                                "Render pass has an attachment with stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD "
                                "and initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                "image truely is undefined at the start of the render pass.");
            }
        }
    }

    for (uint32_t dependency = 0; dependency < pCreateInfo->dependencyCount; dependency++) {
        skip |= CheckPipelineStageFlags("vkCreateRenderPass", pCreateInfo->pDependencies[dependency].srcStageMask);
        skip |= CheckPipelineStageFlags("vkCreateRenderPass", pCreateInfo->pDependencies[dependency].dstStageMask);
    }

    return skip;
}

bool BestPractices::PreCallValidateAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) const {
    bool skip = false;

    if (num_mem_objects + 1 > kMemoryObjectWarningLimit) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_AllocateMemory_TooManyObjects,
                        "Performance Warning: This app has > %" PRIu32 " memory objects.", kMemoryObjectWarningLimit);
    }

    // TODO: Insert get check for GetPhysicalDeviceMemoryProperties once the state is tracked in the StateTracker

    return skip;
}

void BestPractices::PostCallRecordAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory,
                                                 VkResult result) {
    ValidationStateTracker::PostCallRecordAllocateMemory(device, pAllocateInfo, pAllocator, pMemory, result);

    if (VK_SUCCESS == result) {
        num_mem_objects++;
    }
}

bool BestPractices::PreCallValidateFreeMemory(VkDevice device, VkDeviceMemory memory,
                                              const VkAllocationCallbacks* pAllocator) const {
    bool skip = false;

    const DEVICE_MEMORY_STATE* mem_info = ValidationStateTracker::GetDevMemState(memory);

    for (auto& obj : mem_info->obj_bindings) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_INFORMATION_BIT_EXT, get_debug_report_enum[obj.type], 0, layer_name.c_str(),
                        "VK Object %s still has a reference to mem obj %s.", report_data->FormatHandle(obj).c_str(),
                        report_data->FormatHandle(mem_info->mem).c_str());
    }

    return skip;
}

void BestPractices::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    if (memory != VK_NULL_HANDLE) {
        num_mem_objects--;
    }
}

bool BestPractices::ValidateBindBufferMemory(VkBuffer buffer, const char* api_name) const {
    bool skip = false;
    const BUFFER_STATE* buffer_state = GetBufferState(buffer);

    if (!buffer_state->memory_requirements_checked) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_BufferMemReqNotCalled,
                        "%s: Binding memory to %s but vkGetBufferMemoryRequirements() has not been called on that buffer.",
                        api_name, report_data->FormatHandle(buffer).c_str());
    }

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory,
                                                    VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "BindBufferMemory()";

    skip |= ValidateBindBufferMemory(buffer, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2(VkDevice device, uint32_t bindInfoCount,
                                                     const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, api_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateBindBufferMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                        const VkBindBufferMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindBufferMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindBufferMemory(pBindInfos[i].buffer, api_name);
    }

    return skip;
}

bool BestPractices::ValidateBindImageMemory(VkImage image, const char* api_name) const {
    bool skip = false;
    const IMAGE_STATE* image_state = GetImageState(image);

    if (!image_state->memory_requirements_checked) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_ImageMemReqNotCalled,
                        "%s: Binding memory to %s but vkGetImageMemoryRequirements() has not been called on that image.", api_name,
                        report_data->FormatHandle(image).c_str());
    }

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory,
                                                   VkDeviceSize memoryOffset) const {
    bool skip = false;
    const char* api_name = "vkBindImageMemory()";

    skip |= ValidateBindImageMemory(image, api_name);

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2(VkDevice device, uint32_t bindInfoCount,
                                                    const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i].image, api_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount,
                                                       const VkBindImageMemoryInfo* pBindInfos) const {
    char api_name[64];
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        sprintf(api_name, "vkBindImageMemory2KHR() pBindInfos[%u]", i);
        skip |= ValidateBindImageMemory(pBindInfos[i].image, api_name);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                           void* cgpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                     pAllocator, pPipelines, cgpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    kVUID_BestPractices_CreatePipelines_MultiplePipelines,
                    "Performance Warning: This vkCreateGraphicsPipelines call is creating multiple pipelines but is not using a "
                    "pipeline cache, which may help with performance");
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines,
                                                          void* ccpl_state_data) const {
    bool skip = StateTracker::PreCallValidateCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos,
                                                                    pAllocator, pPipelines, ccpl_state_data);

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CreatePipelines_MultiplePipelines,
                        "Performance Warning: This vkCreateComputePipelines call is creating multiple pipelines but is not using a "
                        "pipeline cache, which may help with performance");
    }

    return skip;
}

bool BestPractices::CheckPipelineStageFlags(std::string api_name, const VkPipelineStageFlags flags) const {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_PipelineStageFlags,
                        "You are using VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT when %s is called\n", api_name.c_str());
    } else if (flags & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_PipelineStageFlags,
                        "You are using VK_PIPELINE_STAGE_ALL_COMMANDS_BIT when %s is called\n", api_name.c_str());
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits,
                                               VkFence fence) const {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreCount; semaphore++) {
            skip |= CheckPipelineStageFlags("vkQueueSubmit", pSubmits[submit].pWaitDstStageMask[semaphore]);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdSetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event,
                                                 VkPipelineStageFlags stageMask) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdResetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
                                                 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                                 uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                 uint32_t bufferMemoryBarrierCount,
                                                 const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                 uint32_t imageMemoryBarrierCount,
                                                 const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdWaitEvents", dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
                                                      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags,
                                                      uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
                                                      uint32_t bufferMemoryBarrierCount,
                                                      const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                                      uint32_t imageMemoryBarrierCount,
                                                      const VkImageMemoryBarrier* pImageMemoryBarriers) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                     VkQueryPool queryPool, uint32_t query) const {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWriteTimestamp", pipelineStage);

    return skip;
}

bool BestPractices::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CmdDraw_InstanceCountZero,
                        "Warning: You are calling vkCmdDraw() with an instanceCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CmdDraw_InstanceCountZero,
                        "Warning: You are calling vkCmdDrawIndexed() with an instanceCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CmdDraw_DrawCountZero,
                        "Warning: You are calling vkCmdDrawIndirect() with a drawCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) const {
    bool skip = false;

    if (drawCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CmdDraw_DrawCountZero,
                        "Warning: You are calling vkCmdDrawIndexedIndirect() with a drawCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ) const {
    bool skip = false;

    if ((groupCountX == 0) || (groupCountY == 0) || (groupCountZ == 0)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_CmdDispatch_GroupCountZero,
                        "Warning: You are calling vkCmdDispatch() while one or more groupCounts are zero (groupCountX = %" PRIu32
                        ", groupCountY = %" PRIu32 ", groupCountZ = %" PRIu32 ").",
                        groupCountX, groupCountY, groupCountZ);
    }

    return skip;
}

bool BestPractices::PreCallValidateGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex,
                                                                       uint32_t* pDisplayCount, VkDisplayKHR* pDisplays) const {
    bool skip = false;

    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);

    if (physical_device_state->vkGetPhysicalDeviceDisplayPlanePropertiesKHRState != QUERY_DETAILS) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        kVUID_BestPractices_DisplayPlane_PropertiesNotCalled,
                        "vkGetDisplayPlaneSupportedDisplaysKHR() called before getting diplay plane properties from "
                        "vkGetPhysicalDeviceDisplayPlanePropertiesKHR().");
    }

    return skip;
}

bool BestPractices::PreCallValidateGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount,
                                                         VkImage* pSwapchainImages) const {
    bool skip = false;

    auto swapchain_state = GetSwapchainState(swapchain);

    if (swapchain_state && pSwapchainImages) {
        // Compare the preliminary value of *pSwapchainImageCount with the value this time:
        if (swapchain_state->vkGetSwapchainImagesKHRState == UNCALLED) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,
                            HandleToUint64(device), kVUID_Core_Swapchain_PriorCount,
                            "vkGetSwapchainImagesKHR() called with non-NULL pSwapchainImageCount; but no prior positive value has "
                            "been seen for pSwapchainImages.");
        }
    }

    return skip;
}

// Common function to handle validation for GetPhysicalDeviceQueueFamilyProperties & 2KHR version
static bool ValidateCommonGetPhysicalDeviceQueueFamilyProperties(debug_report_data* report_data,
                                                                 const PHYSICAL_DEVICE_STATE* pd_state,
                                                                 uint32_t requested_queue_family_property_count, bool qfp_null,
                                                                 const char* caller_name) {
    bool skip = false;
    if (!qfp_null) {
        // Verify that for each physical device, this command is called first with NULL pQueueFamilyProperties in order to get count
        if (UNCALLED == pd_state->vkGetPhysicalDeviceQueueFamilyPropertiesState) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                HandleToUint64(pd_state->phys_device), kVUID_Core_DevLimit_MissingQueryCount,
                "%s is called with non-NULL pQueueFamilyProperties before obtaining pQueueFamilyPropertyCount. It is recommended "
                "to first call %s with NULL pQueueFamilyProperties in order to obtain the maximal pQueueFamilyPropertyCount.",
                caller_name, caller_name);
            // Then verify that pCount that is passed in on second call matches what was returned
        } else if (pd_state->queue_family_known_count != requested_queue_family_property_count) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                HandleToUint64(pd_state->phys_device), kVUID_Core_DevLimit_CountMismatch,
                "%s is called with non-NULL pQueueFamilyProperties and pQueueFamilyPropertyCount value %" PRIu32
                ", but the largest previously returned pQueueFamilyPropertyCount for this physicalDevice is %" PRIu32
                ". It is recommended to instead receive all the properties by calling %s with pQueueFamilyPropertyCount that was "
                "previously obtained by calling %s with NULL pQueueFamilyProperties.",
                caller_name, requested_queue_family_property_count, pd_state->queue_family_known_count, caller_name, caller_name);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateBindAccelerationStructureMemoryNV(
    VkDevice device, uint32_t bindInfoCount, const VkBindAccelerationStructureMemoryInfoNV* pBindInfos) const {
    bool skip = false;

    for (uint32_t i = 0; i < bindInfoCount; i++) {
        const ACCELERATION_STRUCTURE_STATE* as_state = GetAccelerationStructureState(pBindInfos[i].accelerationStructure);
        if (!as_state->memory_requirements_checked) {
            // There's not an explicit requirement in the spec to call vkGetImageMemoryRequirements() prior to calling
            // BindAccelerationStructureMemoryNV but it's implied in that memory being bound must conform with
            // VkAccelerationStructureMemoryRequirementsInfoNV from vkGetAccelerationStructureMemoryRequirementsNV
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, 0,
                kVUID_BestPractices_BindAccelNV_NoMemReqQuery,
                "vkBindAccelerationStructureMemoryNV(): "
                "Binding memory to %s but vkGetAccelerationStructureMemoryRequirementsNV() has not been called on that structure.",
                report_data->FormatHandle(pBindInfos[i].accelerationStructure).c_str());
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                          uint32_t* pQueueFamilyPropertyCount,
                                                                          VkQueueFamilyProperties* pQueueFamilyProperties) const {
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(report_data, physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties()");
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2(
    VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2KHR* pQueueFamilyProperties) const {
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(report_data, physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties2()");
}

bool BestPractices::PreCallValidateGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2KHR* pQueueFamilyProperties) const {
    auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    assert(physical_device_state);
    return ValidateCommonGetPhysicalDeviceQueueFamilyProperties(report_data, physical_device_state, *pQueueFamilyPropertyCount,
                                                                (nullptr == pQueueFamilyProperties),
                                                                "vkGetPhysicalDeviceQueueFamilyProperties2KHR()");
}

bool BestPractices::PreCallValidateGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                                                                      uint32_t* pSurfaceFormatCount,
                                                                      VkSurfaceFormatKHR* pSurfaceFormats) const {
    if (!pSurfaceFormats) return false;
    const auto physical_device_state = GetPhysicalDeviceState(physicalDevice);
    const auto& call_state = physical_device_state->vkGetPhysicalDeviceSurfaceFormatsKHRState;
    bool skip = false;
    if (call_state == UNCALLED) {
        // Since we haven't recorded a preliminary value of *pSurfaceFormatCount, that likely means that the application didn't
        // previously call this function with a NULL value of pSurfaceFormats:
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                        HandleToUint64(physicalDevice), kVUID_Core_DevLimit_MustQueryCount,
                        "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount; but no prior "
                        "positive value has been seen for pSurfaceFormats.");
    } else {
        auto prev_format_count = (uint32_t)physical_device_state->surface_formats.size();
        if (*pSurfaceFormatCount > prev_format_count) {
            skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,
                            HandleToUint64(physicalDevice), kVUID_Core_DevLimit_CountMismatch,
                            "vkGetPhysicalDeviceSurfaceFormatsKHR() called with non-NULL pSurfaceFormatCount, and with "
                            "pSurfaceFormats set to a value (%u) that is greater than the value (%u) that was returned "
                            "when pSurfaceFormatCount was NULL.",
                            *pSurfaceFormatCount, prev_format_count);
        }
    }
    return skip;
}

bool BestPractices::PreCallValidateQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                   VkFence fence) const {
    bool skip = false;

    for (uint32_t bindIdx = 0; bindIdx < bindInfoCount; bindIdx++) {
        const VkBindSparseInfo& bindInfo = pBindInfo[bindIdx];
        // Store sparse binding image_state and after binding is complete make sure that any requiring metadata have it bound
        std::unordered_set<const IMAGE_STATE*> sparse_images;
        // Track images getting metadata bound by this call in a set, it'll be recorded into the image_state
        // in RecordQueueBindSparse.
        std::unordered_set<const IMAGE_STATE*> sparse_images_with_metadata;
        // If we're binding sparse image memory make sure reqs were queried and note if metadata is required and bound
        for (uint32_t i = 0; i < bindInfo.imageBindCount; ++i) {
            const auto& image_bind = bindInfo.pImageBinds[i];
            auto image_state = GetImageState(image_bind.image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            sparse_images.insert(image_state);
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                                    "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                                    "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                    report_data->FormatHandle(image_state->image).c_str());
                }
            }
            if (!image_state->memory_requirements_checked) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                                "vkQueueBindSparse(): Binding sparse memory to %s without first calling "
                                "vkGetImageMemoryRequirements() to retrieve requirements.",
                                report_data->FormatHandle(image_state->image).c_str());
            }
        }
        for (uint32_t i = 0; i < bindInfo.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bindInfo.pImageOpaqueBinds[i];
            auto image_state = GetImageState(bindInfo.pImageOpaqueBinds[i].image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            sparse_images.insert(image_state);
            if (image_state->createInfo.flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) {
                if (!image_state->get_sparse_reqs_called || image_state->sparse_requirements.empty()) {
                    // For now just warning if sparse image binding occurs without calling to get reqs first
                    skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                    HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                                    "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                                    "vkGetImageSparseMemoryRequirements[2KHR]() to retrieve requirements.",
                                    report_data->FormatHandle(image_state->image).c_str());
                }
            }
            if (!image_state->memory_requirements_checked) {
                // For now just warning if sparse image binding occurs without calling to get reqs first
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(image_state->image), kVUID_Core_MemTrack_InvalidState,
                                "vkQueueBindSparse(): Binding opaque sparse memory to %s without first calling "
                                "vkGetImageMemoryRequirements() to retrieve requirements.",
                                report_data->FormatHandle(image_state->image).c_str());
            }
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    sparse_images_with_metadata.insert(image_state);
                }
            }
        }
        for (const auto& sparse_image_state : sparse_images) {
            if (sparse_image_state->sparse_metadata_required && !sparse_image_state->sparse_metadata_bound &&
                sparse_images_with_metadata.find(sparse_image_state) == sparse_images_with_metadata.end()) {
                // Warn if sparse image binding metadata required for image with sparse binding, but metadata not bound
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,
                                HandleToUint64(sparse_image_state->image), kVUID_Core_MemTrack_InvalidState,
                                "vkQueueBindSparse(): Binding sparse memory to %s which requires a metadata aspect but no "
                                "binding with VK_SPARSE_MEMORY_BIND_METADATA_BIT set was made.",
                                report_data->FormatHandle(sparse_image_state->image).c_str());
            }
        }
    }

    return skip;
}

void BestPractices::PostCallRecordQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo,
                                                  VkFence fence, VkResult result) {
    if (result != VK_SUCCESS) return;

    for (uint32_t bindIdx = 0; bindIdx < bindInfoCount; bindIdx++) {
        const VkBindSparseInfo& bindInfo = pBindInfo[bindIdx];
        for (uint32_t i = 0; i < bindInfo.imageOpaqueBindCount; ++i) {
            const auto& image_opaque_bind = bindInfo.pImageOpaqueBinds[i];
            auto image_state = GetImageState(bindInfo.pImageOpaqueBinds[i].image);
            if (!image_state)
                continue;  // Param/Object validation should report image_bind.image handles being invalid, so just skip here.
            for (uint32_t j = 0; j < image_opaque_bind.bindCount; ++j) {
                if (image_opaque_bind.pBinds[j].flags & VK_SPARSE_MEMORY_BIND_METADATA_BIT) {
                    image_state->sparse_metadata_bound = true;
                }
            }
        }
    }
}
