// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See deprecation_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#pragma once

#include <vulkan/vulkan.h>
#include "chassis/validation_object.h"

namespace deprecation {

class Instance : public vvl::base::Instance {
    using BaseClass = vvl::base::Instance;

  public:
    Instance(vvl::dispatch::Instance* dispatch) : BaseClass(dispatch, LayerObjectTypeDeprecation) {}

    void PostCallRecordCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                      VkInstance* pInstance, const RecordObject& record_obj) override;

    bool supported_vk_khr_get_physical_device_properties2 = false;
    bool PreCallValidateGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures,
                                                  const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                          VkFormatProperties* pFormatProperties,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type,
                                                               VkImageTiling tiling, VkImageUsageFlags usage,
                                                               VkImageCreateFlags flags,
                                                               VkImageFormatProperties* pImageFormatProperties,
                                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties,
                                                    const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount,
                                                               VkQueueFamilyProperties* pQueueFamilyProperties,
                                                               const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
                                                          VkPhysicalDeviceMemoryProperties* pMemoryProperties,
                                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount,
                                                       VkLayerProperties* pProperties, const ErrorObject& error_obj) const override;
    bool PreCallValidateGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
                                                                     VkImageType type, VkSampleCountFlagBits samples,
                                                                     VkImageUsageFlags usage, VkImageTiling tiling,
                                                                     uint32_t* pPropertyCount,
                                                                     VkSparseImageFormatProperties* pProperties,
                                                                     const ErrorObject& error_obj) const override;
};

class Device : public vvl::base::Device {
    using BaseClass = vvl::base::Device;

  public:
    Device(vvl::dispatch::Device* dev, Instance* instance_vo)
        : BaseClass(dev, instance_vo, LayerObjectTypeDeprecation), instance(instance_vo) {}
    ~Device() {}
    Instance* instance;

    void FinishDeviceSetup(const VkDeviceCreateInfo* pCreateInfo, const Location& loc) override;
    bool supported_vk_khr_create_renderpass2 = false;
    bool supported_vk_khr_dynamic_rendering_local_read = false;
    bool PreCallValidateCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo,
                                         const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                         const ErrorObject& error_obj) const override;
    bool PreCallValidateGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity,
                                                 const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                           VkSubpassContents contents, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents,
                                       const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass(VkCommandBuffer commandBuffer, const ErrorObject& error_obj) const override;
    bool PreCallValidateCreateRenderPass2(VkDevice device, const VkRenderPassCreateInfo2* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass,
                                          const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdBeginRenderPass2(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin,
                                            const VkSubpassBeginInfo* pSubpassBeginInfo,
                                            const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdNextSubpass2(VkCommandBuffer commandBuffer, const VkSubpassBeginInfo* pSubpassBeginInfo,
                                        const VkSubpassEndInfo* pSubpassEndInfo, const ErrorObject& error_obj) const override;
    bool PreCallValidateCmdEndRenderPass2(VkCommandBuffer commandBuffer, const VkSubpassEndInfo* pSubpassEndInfo,
                                          const ErrorObject& error_obj) const override;
};
}  // namespace deprecation
// NOLINTEND
