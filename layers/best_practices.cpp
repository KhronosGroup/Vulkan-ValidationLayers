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

#include <string>
#include <iomanip>

// get the API name is proper format
std::string BestPractices::GetAPIVersionName(uint32_t version) {
    std::stringstream version_name;
    uint32_t major = VK_VERSION_MAJOR(version);
    uint32_t minor = VK_VERSION_MINOR(version);
    uint32_t patch = VK_VERSION_PATCH(version);

    version_name << major << "." << minor << "." << patch << " (0x" << std::setfill('0') << std::setw(8) << std::hex << version
                 << ")";

    return version_name.str();
}

bool BestPractices::PreCallValidateCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                  VkInstance* pInstance) {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kDeviceExtensionNames)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(), "vkCreateInstance(): Attempting to enable Device Extension %s at CreateInstance time.",
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
                                                const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    bool skip = false;

    // get API version of physical device passed when creating device.
    VkPhysicalDeviceProperties physical_device_properties{};
    DispatchGetPhysicalDeviceProperties(physicalDevice, &physical_device_properties);
    device_api_version = physical_device_properties.apiVersion;

    // check api versions and warn if instance api Version is higher than version on device.
    if (instance_api_version > device_api_version) {
        std::string inst_api_name = GetAPIVersionName(instance_api_version);
        std::string dev_api_name = GetAPIVersionName(device_api_version);

        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                    "vkCreateDevice(): API Version of current instance, %s is higher than API Version on device, %s",
                    inst_api_name.c_str(), dev_api_name.c_str());
    }

    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {
        if (white_list(pCreateInfo->ppEnabledExtensionNames[i], kInstanceExtensionNames)) {
            skip |=
                log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(), "vkCreateDevice(): Attempting to enable Instance Extension %s at CreateDevice time.",
                        pCreateInfo->ppEnabledExtensionNames[i]);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo,
                                                const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream bufferHex;
        bufferHex << "0x" << std::hex << HandleToUint64(pBuffer);

        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                    "Warning: Buffer (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                    "(queueFamilyIndexCount of %" PRIu32 ").",
                    bufferHex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->sharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        std::stringstream imageHex;
        imageHex << "0x" << std::hex << HandleToUint64(pImage);

        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                    "Warning: Image (%s) specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple queues "
                    "(queueFamilyIndexCount of %" PRIu32 ").",
                    imageHex.str().c_str(), pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo,
                                                      const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    bool skip = false;

    if ((pCreateInfo->queueFamilyIndexCount > 1) && (pCreateInfo->imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                    "Warning: A Swapchain is being created which specifies a sharing mode of VK_SHARING_MODE_EXCULSIVE while "
                    "specifying multiple queues (queueFamilyIndexCount of %" PRIu32 ").",
                    pCreateInfo->queueFamilyIndexCount);
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount,
                                                             const VkSwapchainCreateInfoKHR* pCreateInfos,
                                                             const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains) {
    bool skip = false;

    for (uint32_t i = 0; i < swapchainCount; i++) {
        if ((pCreateInfos[i].queueFamilyIndexCount > 1) && (pCreateInfos[i].imageSharingMode == VK_SHARING_MODE_EXCLUSIVE)) {
            skip |= log_msg(
                report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                "Warning: A shared swapchain (index %" PRIu32
                ") is being created which specifies a sharing mode of VK_SHARING_MODE_EXCLUSIVE while specifying multiple "
                "queues (queueFamilyIndexCount of %" PRIu32 ").",
                i, pCreateInfos[i].queueFamilyIndexCount);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    bool skip = false;

    for (uint32_t i = 0; i < pCreateInfo->attachmentCount; ++i) {
        VkFormat format = pCreateInfo->pAttachments[i].format;
        if (pCreateInfo->pAttachments[i].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            if ((FormatIsColor(format) || FormatHasDepth(format)) &&
                pCreateInfo->pAttachments[i].loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                layer_name.c_str(),
                                "Render pass has an attachment with loadOp == VK_ATTACHMENT_LOAD_OP_LOAD and "
                                "initialLayout == VK_IMAGE_LAYOUT_UNDEFINED.  This is probably not what you "
                                "intended.  Consider using VK_ATTACHMENT_LOAD_OP_DONT_CARE instead if the "
                                "image truely is undefined at the start of the render pass.");
            }
            if (FormatHasStencil(format) && pCreateInfo->pAttachments[i].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
                skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                                layer_name.c_str(),
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
                                                  const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    bool skip = false;

    num_mem_objects++;

    if (num_mem_objects > kMemoryObjectWarningLimit) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(), "Performance Warning: This app has > %" PRIu32 " memory objects.",
                        kMemoryObjectWarningLimit);
    }

    return skip;
}

void BestPractices::PreCallRecordFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    if (memory != VK_NULL_HANDLE) {
        num_mem_objects--;
    }
}

bool BestPractices::PreCallValidateCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                           const VkGraphicsPipelineCreateInfo* pCreateInfos,
                                                           const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    bool skip = false;

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                    layer_name.c_str(),
                    "Performance Warning: This vkCreateGraphicsPipelines call is creating multiple pipelines but is not using a "
                    "pipeline cache, which may help with performance");
    }

    return skip;
}

bool BestPractices::PreCallValidateCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
                                                          const VkComputePipelineCreateInfo* pCreateInfos,
                                                          const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    bool skip = false;

    if ((createInfoCount > 1) && (!pipelineCache)) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(),
                        "Performance Warning: This vkCreateComputePipelines call is creating multiple pipelines but is not using a "
                        "pipeline cache, which may help with performance");
    }

    return skip;
}

bool BestPractices::CheckPipelineStageFlags(std::string api_name, const VkPipelineStageFlags flags) {
    bool skip = false;

    if (flags & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                    "You are using VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT when %s is called\n", api_name.c_str());
    } else if (flags & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                    "You are using VK_PIPELINE_STAGE_ALL_COMMANDS_BIT when %s is called\n", api_name.c_str());
    }

    return skip;
}

bool BestPractices::PreCallValidateQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    bool skip = false;

    for (uint32_t submit = 0; submit < submitCount; submit++) {
        for (uint32_t semaphore = 0; semaphore < pSubmits[submit].waitSemaphoreCount; semaphore++) {
            skip |= CheckPipelineStageFlags("vkQueueSubmit", pSubmits[submit].pWaitDstStageMask[semaphore]);
        }
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdSetEvent", stageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
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
                                                 const VkImageMemoryBarrier* pImageMemoryBarriers) {
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
                                                      const VkImageMemoryBarrier* pImageMemoryBarriers) {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", srcStageMask);
    skip |= CheckPipelineStageFlags("vkCmdPipelineBarrier", dstStageMask);

    return skip;
}

bool BestPractices::PreCallValidateCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
                                                     VkQueryPool queryPool, uint32_t query) {
    bool skip = false;

    skip |= CheckPipelineStageFlags("vkCmdWriteTimestamp", pipelineStage);

    return skip;
}

bool BestPractices::PreCallValidateCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                           uint32_t firstVertex, uint32_t firstInstance) {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(), "Warning: You are calling vkCmdDraw() with an instanceCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    bool skip = false;

    if (instanceCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(), "Warning: You are calling vkCmdDrawIndexed() with an instanceCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                   uint32_t drawCount, uint32_t stride) {
    bool skip = false;

    if (drawCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(), "Warning: You are calling vkCmdDrawIndirect() with a drawCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
                                                          uint32_t drawCount, uint32_t stride) {
    bool skip = false;

    if (drawCount == 0) {
        skip |= log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0,
                        layer_name.c_str(), "Warning: You are calling vkCmdDrawIndexedIndirect() with a drawCount of Zero.");
    }

    return skip;
}

bool BestPractices::PreCallValidateCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
                                               uint32_t groupCountZ) {
    bool skip = false;

    if ((groupCountX == 0) || (groupCountY == 0) || (groupCountZ == 0)) {
        skip |=
            log_msg(report_data, VK_DEBUG_REPORT_WARNING_BIT_EXT, VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, 0, layer_name.c_str(),
                    "Warning: You are calling vkCmdDispatch() while one or more groupCounts are zero (groupCountX = %" PRIu32
                    ", groupCountY = %" PRIu32 ", groupCountZ = %" PRIu32 ").",
                    groupCountX, groupCountY, groupCountZ);
    }

    return skip;
}
