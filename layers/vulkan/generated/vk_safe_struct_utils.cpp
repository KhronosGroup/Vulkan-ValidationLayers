// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See safe_struct_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google Inc.
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

#include "vk_safe_struct.h"
#include "utils/vk_layer_utils.h"

#include <vector>

extern std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info;

char *SafeStringCopy(const char *in_string) {
    if (nullptr == in_string) return nullptr;
    char *dest = new char[std::strlen(in_string) + 1];
    return std::strcpy(dest, in_string);
}

// clang-format off
void *SafePnextCopy(const void *pNext, PNextCopyState* copy_state) {
    void *first_pNext{};
    VkBaseOutStructure *prev_pNext{};
    void *safe_pNext{};

    while (pNext) {
        const VkBaseOutStructure *header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

        switch (header->sType) {
            // Add special-case code to copy beloved secret loader structs
            // Special-case Loader Instance Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO: {
                VkLayerInstanceCreateInfo *struct_copy = new VkLayerInstanceCreateInfo;
                // TODO: Uses original VkLayerInstanceLink* chain, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerInstanceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }
            // Special-case Loader Device Struct passed to/from layer in pNext chain
            case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO: {
                VkLayerDeviceCreateInfo *struct_copy = new VkLayerDeviceCreateInfo;
                // TODO: Uses original VkLayerDeviceLink*, which should be okay for our uses
                memcpy(struct_copy, pNext, sizeof(VkLayerDeviceCreateInfo));
                safe_pNext = struct_copy;
                break;
            }
            case VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO:
                safe_pNext = new safe_VkShaderModuleCreateInfo(reinterpret_cast<const VkShaderModuleCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceSubgroupProperties(reinterpret_cast<const VkPhysicalDeviceSubgroupProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES:
                safe_pNext = new safe_VkPhysicalDevice16BitStorageFeatures(reinterpret_cast<const VkPhysicalDevice16BitStorageFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS:
                safe_pNext = new safe_VkMemoryDedicatedRequirements(reinterpret_cast<const VkMemoryDedicatedRequirements *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
                safe_pNext = new safe_VkMemoryDedicatedAllocateInfo(reinterpret_cast<const VkMemoryDedicatedAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
                safe_pNext = new safe_VkMemoryAllocateFlagsInfo(reinterpret_cast<const VkMemoryAllocateFlagsInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO:
                safe_pNext = new safe_VkDeviceGroupRenderPassBeginInfo(reinterpret_cast<const VkDeviceGroupRenderPassBeginInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO:
                safe_pNext = new safe_VkDeviceGroupCommandBufferBeginInfo(reinterpret_cast<const VkDeviceGroupCommandBufferBeginInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
                safe_pNext = new safe_VkDeviceGroupSubmitInfo(reinterpret_cast<const VkDeviceGroupSubmitInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO:
                safe_pNext = new safe_VkDeviceGroupBindSparseInfo(reinterpret_cast<const VkDeviceGroupBindSparseInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO:
                safe_pNext = new safe_VkBindBufferMemoryDeviceGroupInfo(reinterpret_cast<const VkBindBufferMemoryDeviceGroupInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO:
                safe_pNext = new safe_VkBindImageMemoryDeviceGroupInfo(reinterpret_cast<const VkBindImageMemoryDeviceGroupInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO:
                safe_pNext = new safe_VkDeviceGroupDeviceCreateInfo(reinterpret_cast<const VkDeviceGroupDeviceCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
                safe_pNext = new safe_VkPhysicalDeviceFeatures2(reinterpret_cast<const VkPhysicalDeviceFeatures2 *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDevicePointClippingProperties(reinterpret_cast<const VkPhysicalDevicePointClippingProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO:
                safe_pNext = new safe_VkRenderPassInputAttachmentAspectCreateInfo(reinterpret_cast<const VkRenderPassInputAttachmentAspectCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO:
                safe_pNext = new safe_VkImageViewUsageCreateInfo(reinterpret_cast<const VkImageViewUsageCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO:
                safe_pNext = new safe_VkPipelineTessellationDomainOriginStateCreateInfo(reinterpret_cast<const VkPipelineTessellationDomainOriginStateCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO:
                safe_pNext = new safe_VkRenderPassMultiviewCreateInfo(reinterpret_cast<const VkRenderPassMultiviewCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewFeatures(reinterpret_cast<const VkPhysicalDeviceMultiviewFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewProperties(reinterpret_cast<const VkPhysicalDeviceMultiviewProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVariablePointersFeatures(reinterpret_cast<const VkPhysicalDeviceVariablePointersFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceProtectedMemoryFeatures(reinterpret_cast<const VkPhysicalDeviceProtectedMemoryFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceProtectedMemoryProperties(reinterpret_cast<const VkPhysicalDeviceProtectedMemoryProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO:
                safe_pNext = new safe_VkProtectedSubmitInfo(reinterpret_cast<const VkProtectedSubmitInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO:
                safe_pNext = new safe_VkSamplerYcbcrConversionInfo(reinterpret_cast<const VkSamplerYcbcrConversionInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO:
                safe_pNext = new safe_VkBindImagePlaneMemoryInfo(reinterpret_cast<const VkBindImagePlaneMemoryInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO:
                safe_pNext = new safe_VkImagePlaneMemoryRequirementsInfo(reinterpret_cast<const VkImagePlaneMemoryRequirementsInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures(reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES:
                safe_pNext = new safe_VkSamplerYcbcrConversionImageFormatProperties(reinterpret_cast<const VkSamplerYcbcrConversionImageFormatProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
                safe_pNext = new safe_VkPhysicalDeviceExternalImageFormatInfo(reinterpret_cast<const VkPhysicalDeviceExternalImageFormatInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
                safe_pNext = new safe_VkExternalImageFormatProperties(reinterpret_cast<const VkExternalImageFormatProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceIDProperties(reinterpret_cast<const VkPhysicalDeviceIDProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
                safe_pNext = new safe_VkExternalMemoryImageCreateInfo(reinterpret_cast<const VkExternalMemoryImageCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO:
                safe_pNext = new safe_VkExternalMemoryBufferCreateInfo(reinterpret_cast<const VkExternalMemoryBufferCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
                safe_pNext = new safe_VkExportMemoryAllocateInfo(reinterpret_cast<const VkExportMemoryAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO:
                safe_pNext = new safe_VkExportFenceCreateInfo(reinterpret_cast<const VkExportFenceCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
                safe_pNext = new safe_VkExportSemaphoreCreateInfo(reinterpret_cast<const VkExportSemaphoreCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance3Properties(reinterpret_cast<const VkPhysicalDeviceMaintenance3Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderDrawParametersFeatures(reinterpret_cast<const VkPhysicalDeviceShaderDrawParametersFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan11Features(reinterpret_cast<const VkPhysicalDeviceVulkan11Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan11Properties(reinterpret_cast<const VkPhysicalDeviceVulkan11Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan12Features(reinterpret_cast<const VkPhysicalDeviceVulkan12Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan12Properties(reinterpret_cast<const VkPhysicalDeviceVulkan12Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
                safe_pNext = new safe_VkImageFormatListCreateInfo(reinterpret_cast<const VkImageFormatListCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES:
                safe_pNext = new safe_VkPhysicalDevice8BitStorageFeatures(reinterpret_cast<const VkPhysicalDevice8BitStorageFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceDriverProperties(reinterpret_cast<const VkPhysicalDeviceDriverProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderAtomicInt64Features(reinterpret_cast<const VkPhysicalDeviceShaderAtomicInt64Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderFloat16Int8Features(reinterpret_cast<const VkPhysicalDeviceShaderFloat16Int8Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceFloatControlsProperties(reinterpret_cast<const VkPhysicalDeviceFloatControlsProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO:
                safe_pNext = new safe_VkDescriptorSetLayoutBindingFlagsCreateInfo(reinterpret_cast<const VkDescriptorSetLayoutBindingFlagsCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorIndexingFeatures(reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorIndexingProperties(reinterpret_cast<const VkPhysicalDeviceDescriptorIndexingProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO:
                safe_pNext = new safe_VkDescriptorSetVariableDescriptorCountAllocateInfo(reinterpret_cast<const VkDescriptorSetVariableDescriptorCountAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT:
                safe_pNext = new safe_VkDescriptorSetVariableDescriptorCountLayoutSupport(reinterpret_cast<const VkDescriptorSetVariableDescriptorCountLayoutSupport *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE:
                safe_pNext = new safe_VkSubpassDescriptionDepthStencilResolve(reinterpret_cast<const VkSubpassDescriptionDepthStencilResolve *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceDepthStencilResolveProperties(reinterpret_cast<const VkPhysicalDeviceDepthStencilResolveProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceScalarBlockLayoutFeatures(reinterpret_cast<const VkPhysicalDeviceScalarBlockLayoutFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
                safe_pNext = new safe_VkImageStencilUsageCreateInfo(reinterpret_cast<const VkImageStencilUsageCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO:
                safe_pNext = new safe_VkSamplerReductionModeCreateInfo(reinterpret_cast<const VkSamplerReductionModeCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceSamplerFilterMinmaxProperties(reinterpret_cast<const VkPhysicalDeviceSamplerFilterMinmaxProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkanMemoryModelFeatures(reinterpret_cast<const VkPhysicalDeviceVulkanMemoryModelFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceImagelessFramebufferFeatures(reinterpret_cast<const VkPhysicalDeviceImagelessFramebufferFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO:
                safe_pNext = new safe_VkFramebufferAttachmentsCreateInfo(reinterpret_cast<const VkFramebufferAttachmentsCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO:
                safe_pNext = new safe_VkRenderPassAttachmentBeginInfo(reinterpret_cast<const VkRenderPassAttachmentBeginInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures(reinterpret_cast<const VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures(reinterpret_cast<const VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures(reinterpret_cast<const VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT:
                safe_pNext = new safe_VkAttachmentReferenceStencilLayout(reinterpret_cast<const VkAttachmentReferenceStencilLayout *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT:
                safe_pNext = new safe_VkAttachmentDescriptionStencilLayout(reinterpret_cast<const VkAttachmentDescriptionStencilLayout *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceHostQueryResetFeatures(reinterpret_cast<const VkPhysicalDeviceHostQueryResetFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceTimelineSemaphoreFeatures(reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceTimelineSemaphoreProperties(reinterpret_cast<const VkPhysicalDeviceTimelineSemaphoreProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
                safe_pNext = new safe_VkSemaphoreTypeCreateInfo(reinterpret_cast<const VkSemaphoreTypeCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO:
                safe_pNext = new safe_VkTimelineSemaphoreSubmitInfo(reinterpret_cast<const VkTimelineSemaphoreSubmitInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceBufferDeviceAddressFeatures(reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO:
                safe_pNext = new safe_VkBufferOpaqueCaptureAddressCreateInfo(reinterpret_cast<const VkBufferOpaqueCaptureAddressCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO:
                safe_pNext = new safe_VkMemoryOpaqueCaptureAddressAllocateInfo(reinterpret_cast<const VkMemoryOpaqueCaptureAddressAllocateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan13Features(reinterpret_cast<const VkPhysicalDeviceVulkan13Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceVulkan13Properties(reinterpret_cast<const VkPhysicalDeviceVulkan13Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO:
                safe_pNext = new safe_VkPipelineCreationFeedbackCreateInfo(reinterpret_cast<const VkPipelineCreationFeedbackCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderTerminateInvocationFeatures(reinterpret_cast<const VkPhysicalDeviceShaderTerminateInvocationFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures(reinterpret_cast<const VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES:
                safe_pNext = new safe_VkPhysicalDevicePrivateDataFeatures(reinterpret_cast<const VkPhysicalDevicePrivateDataFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO:
                safe_pNext = new safe_VkDevicePrivateDataCreateInfo(reinterpret_cast<const VkDevicePrivateDataCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES:
                safe_pNext = new safe_VkPhysicalDevicePipelineCreationCacheControlFeatures(reinterpret_cast<const VkPhysicalDevicePipelineCreationCacheControlFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_BARRIER_2:
                safe_pNext = new safe_VkMemoryBarrier2(reinterpret_cast<const VkMemoryBarrier2 *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSynchronization2Features(reinterpret_cast<const VkPhysicalDeviceSynchronization2Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures(reinterpret_cast<const VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceImageRobustnessFeatures(reinterpret_cast<const VkPhysicalDeviceImageRobustnessFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceSubgroupSizeControlFeatures(reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceSubgroupSizeControlProperties(reinterpret_cast<const VkPhysicalDeviceSubgroupSizeControlProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO:
                safe_pNext = new safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfo(reinterpret_cast<const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceInlineUniformBlockFeatures(reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceInlineUniformBlockProperties(reinterpret_cast<const VkPhysicalDeviceInlineUniformBlockProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK:
                safe_pNext = new safe_VkWriteDescriptorSetInlineUniformBlock(reinterpret_cast<const VkWriteDescriptorSetInlineUniformBlock *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO:
                safe_pNext = new safe_VkDescriptorPoolInlineUniformBlockCreateInfo(reinterpret_cast<const VkDescriptorPoolInlineUniformBlockCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceTextureCompressionASTCHDRFeatures(reinterpret_cast<const VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO:
                safe_pNext = new safe_VkPipelineRenderingCreateInfo(reinterpret_cast<const VkPipelineRenderingCreateInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceDynamicRenderingFeatures(reinterpret_cast<const VkPhysicalDeviceDynamicRenderingFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO:
                safe_pNext = new safe_VkCommandBufferInheritanceRenderingInfo(reinterpret_cast<const VkCommandBufferInheritanceRenderingInfo *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceShaderIntegerDotProductFeatures(reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductFeatures *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceShaderIntegerDotProductProperties(reinterpret_cast<const VkPhysicalDeviceShaderIntegerDotProductProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceTexelBufferAlignmentProperties(reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentProperties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3:
                safe_pNext = new safe_VkFormatProperties3(reinterpret_cast<const VkFormatProperties3 *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance4Features(reinterpret_cast<const VkPhysicalDeviceMaintenance4Features *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance4Properties(reinterpret_cast<const VkPhysicalDeviceMaintenance4Properties *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR:
                safe_pNext = new safe_VkImageSwapchainCreateInfoKHR(reinterpret_cast<const VkImageSwapchainCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR:
                safe_pNext = new safe_VkBindImageMemorySwapchainInfoKHR(reinterpret_cast<const VkBindImageMemorySwapchainInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR:
                safe_pNext = new safe_VkDeviceGroupPresentInfoKHR(reinterpret_cast<const VkDeviceGroupPresentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR:
                safe_pNext = new safe_VkDeviceGroupSwapchainCreateInfoKHR(reinterpret_cast<const VkDeviceGroupSwapchainCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR:
                safe_pNext = new safe_VkDisplayPresentInfoKHR(reinterpret_cast<const VkDisplayPresentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR:
                safe_pNext = new safe_VkQueueFamilyQueryResultStatusPropertiesKHR(reinterpret_cast<const VkQueueFamilyQueryResultStatusPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR:
                safe_pNext = new safe_VkQueueFamilyVideoPropertiesKHR(reinterpret_cast<const VkQueueFamilyVideoPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoProfileInfoKHR(reinterpret_cast<const VkVideoProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR:
                safe_pNext = new safe_VkVideoProfileListInfoKHR(reinterpret_cast<const VkVideoProfileListInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeCapabilitiesKHR(reinterpret_cast<const VkVideoDecodeCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeUsageInfoKHR(reinterpret_cast<const VkVideoDecodeUsageInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264ProfileInfoKHR(reinterpret_cast<const VkVideoDecodeH264ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeH264CapabilitiesKHR(reinterpret_cast<const VkVideoDecodeH264CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264SessionParametersAddInfoKHR(reinterpret_cast<const VkVideoDecodeH264SessionParametersAddInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoDecodeH264SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264PictureInfoKHR(reinterpret_cast<const VkVideoDecodeH264PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH264DpbSlotInfoKHR(reinterpret_cast<const VkVideoDecodeH264DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
                safe_pNext = new safe_VkRenderingFragmentShadingRateAttachmentInfoKHR(reinterpret_cast<const VkRenderingFragmentShadingRateAttachmentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT:
                safe_pNext = new safe_VkRenderingFragmentDensityMapAttachmentInfoEXT(reinterpret_cast<const VkRenderingFragmentDensityMapAttachmentInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD:
                safe_pNext = new safe_VkAttachmentSampleCountInfoAMD(reinterpret_cast<const VkAttachmentSampleCountInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX:
                safe_pNext = new safe_VkMultiviewPerViewAttributesInfoNVX(reinterpret_cast<const VkMultiviewPerViewAttributesInfoNVX *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkImportMemoryWin32HandleInfoKHR(reinterpret_cast<const VkImportMemoryWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkExportMemoryWin32HandleInfoKHR(reinterpret_cast<const VkExportMemoryWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
                safe_pNext = new safe_VkImportMemoryFdInfoKHR(reinterpret_cast<const VkImportMemoryFdInfoKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR:
                safe_pNext = new safe_VkWin32KeyedMutexAcquireReleaseInfoKHR(reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkExportSemaphoreWin32HandleInfoKHR(reinterpret_cast<const VkExportSemaphoreWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR:
                safe_pNext = new safe_VkD3D12FenceSubmitInfoKHR(reinterpret_cast<const VkD3D12FenceSubmitInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePushDescriptorPropertiesKHR(reinterpret_cast<const VkPhysicalDevicePushDescriptorPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR:
                safe_pNext = new safe_VkPresentRegionsKHR(reinterpret_cast<const VkPresentRegionsKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR:
                safe_pNext = new safe_VkSharedPresentSurfaceCapabilitiesKHR(reinterpret_cast<const VkSharedPresentSurfaceCapabilitiesKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR:
                safe_pNext = new safe_VkExportFenceWin32HandleInfoKHR(reinterpret_cast<const VkExportFenceWin32HandleInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePerformanceQueryFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePerformanceQueryFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePerformanceQueryPropertiesKHR(reinterpret_cast<const VkPhysicalDevicePerformanceQueryPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR:
                safe_pNext = new safe_VkQueryPoolPerformanceCreateInfoKHR(reinterpret_cast<const VkQueryPoolPerformanceCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR:
                safe_pNext = new safe_VkPerformanceQuerySubmitInfoKHR(reinterpret_cast<const VkPerformanceQuerySubmitInfoKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR(reinterpret_cast<const VkPhysicalDevicePortabilitySubsetPropertiesKHR *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderClockFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderClockFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265ProfileInfoKHR(reinterpret_cast<const VkVideoDecodeH265ProfileInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoDecodeH265CapabilitiesKHR(reinterpret_cast<const VkVideoDecodeH265CapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265SessionParametersAddInfoKHR(reinterpret_cast<const VkVideoDecodeH265SessionParametersAddInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265SessionParametersCreateInfoKHR(reinterpret_cast<const VkVideoDecodeH265SessionParametersCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265PictureInfoKHR(reinterpret_cast<const VkVideoDecodeH265PictureInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR:
                safe_pNext = new safe_VkVideoDecodeH265DpbSlotInfoKHR(reinterpret_cast<const VkVideoDecodeH265DpbSlotInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR:
                safe_pNext = new safe_VkDeviceQueueGlobalPriorityCreateInfoKHR(reinterpret_cast<const VkDeviceQueueGlobalPriorityCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES_KHR:
                safe_pNext = new safe_VkQueueFamilyGlobalPriorityPropertiesKHR(reinterpret_cast<const VkQueueFamilyGlobalPriorityPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
                safe_pNext = new safe_VkFragmentShadingRateAttachmentInfoKHR(reinterpret_cast<const VkFragmentShadingRateAttachmentInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR:
                safe_pNext = new safe_VkPipelineFragmentShadingRateStateCreateInfoKHR(reinterpret_cast<const VkPipelineFragmentShadingRateStateCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRatePropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR:
                safe_pNext = new safe_VkSurfaceProtectedCapabilitiesKHR(reinterpret_cast<const VkSurfaceProtectedCapabilitiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePresentWaitFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePresentWaitFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR:
                safe_pNext = new safe_VkPipelineLibraryCreateInfoKHR(reinterpret_cast<const VkPipelineLibraryCreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PRESENT_ID_KHR:
                safe_pNext = new safe_VkPresentIdKHR(reinterpret_cast<const VkPresentIdKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDevicePresentIdFeaturesKHR(reinterpret_cast<const VkPhysicalDevicePresentIdFeaturesKHR *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR:
                safe_pNext = new safe_VkVideoEncodeCapabilitiesKHR(reinterpret_cast<const VkVideoEncodeCapabilitiesKHR *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR:
                safe_pNext = new safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR(reinterpret_cast<const VkQueryPoolVideoEncodeFeedbackCreateInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeUsageInfoKHR(reinterpret_cast<const VkVideoEncodeUsageInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeRateControlInfoKHR(reinterpret_cast<const VkVideoEncodeRateControlInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR:
                safe_pNext = new safe_VkVideoEncodeQualityLevelInfoKHR(reinterpret_cast<const VkVideoEncodeQualityLevelInfoKHR *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV:
                safe_pNext = new safe_VkQueueFamilyCheckpointProperties2NV(reinterpret_cast<const VkQueueFamilyCheckpointProperties2NV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR(reinterpret_cast<const VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance5FeaturesKHR(reinterpret_cast<const VkPhysicalDeviceMaintenance5FeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceMaintenance5PropertiesKHR(reinterpret_cast<const VkPhysicalDeviceMaintenance5PropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR:
                safe_pNext = new safe_VkPipelineCreateFlags2CreateInfoKHR(reinterpret_cast<const VkPipelineCreateFlags2CreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR:
                safe_pNext = new safe_VkBufferUsageFlags2CreateInfoKHR(reinterpret_cast<const VkBufferUsageFlags2CreateInfoKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixPropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT:
                safe_pNext = new safe_VkDebugReportCallbackCreateInfoEXT(reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD:
                safe_pNext = new safe_VkPipelineRasterizationStateRasterizationOrderAMD(reinterpret_cast<const VkPipelineRasterizationStateRasterizationOrderAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV:
                safe_pNext = new safe_VkDedicatedAllocationImageCreateInfoNV(reinterpret_cast<const VkDedicatedAllocationImageCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV:
                safe_pNext = new safe_VkDedicatedAllocationBufferCreateInfoNV(reinterpret_cast<const VkDedicatedAllocationBufferCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV:
                safe_pNext = new safe_VkDedicatedAllocationMemoryAllocateInfoNV(reinterpret_cast<const VkDedicatedAllocationMemoryAllocateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceTransformFeedbackPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationStateStreamCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationStateStreamCreateInfoEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_EXT:
                safe_pNext = new safe_VkVideoEncodeH264CapabilitiesEXT(reinterpret_cast<const VkVideoEncodeH264CapabilitiesEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_EXT:
                safe_pNext = new safe_VkVideoEncodeH264QualityLevelPropertiesEXT(reinterpret_cast<const VkVideoEncodeH264QualityLevelPropertiesEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264SessionCreateInfoEXT(reinterpret_cast<const VkVideoEncodeH264SessionCreateInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersAddInfoEXT(reinterpret_cast<const VkVideoEncodeH264SessionParametersAddInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersCreateInfoEXT(reinterpret_cast<const VkVideoEncodeH264SessionParametersCreateInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersGetInfoEXT(reinterpret_cast<const VkVideoEncodeH264SessionParametersGetInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264SessionParametersFeedbackInfoEXT(reinterpret_cast<const VkVideoEncodeH264SessionParametersFeedbackInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264PictureInfoEXT(reinterpret_cast<const VkVideoEncodeH264PictureInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264DpbSlotInfoEXT(reinterpret_cast<const VkVideoEncodeH264DpbSlotInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264ProfileInfoEXT(reinterpret_cast<const VkVideoEncodeH264ProfileInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264RateControlInfoEXT(reinterpret_cast<const VkVideoEncodeH264RateControlInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264RateControlLayerInfoEXT(reinterpret_cast<const VkVideoEncodeH264RateControlLayerInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH264GopRemainingFrameInfoEXT(reinterpret_cast<const VkVideoEncodeH264GopRemainingFrameInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_EXT:
                safe_pNext = new safe_VkVideoEncodeH265CapabilitiesEXT(reinterpret_cast<const VkVideoEncodeH265CapabilitiesEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265SessionCreateInfoEXT(reinterpret_cast<const VkVideoEncodeH265SessionCreateInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_EXT:
                safe_pNext = new safe_VkVideoEncodeH265QualityLevelPropertiesEXT(reinterpret_cast<const VkVideoEncodeH265QualityLevelPropertiesEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersAddInfoEXT(reinterpret_cast<const VkVideoEncodeH265SessionParametersAddInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersCreateInfoEXT(reinterpret_cast<const VkVideoEncodeH265SessionParametersCreateInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersGetInfoEXT(reinterpret_cast<const VkVideoEncodeH265SessionParametersGetInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265SessionParametersFeedbackInfoEXT(reinterpret_cast<const VkVideoEncodeH265SessionParametersFeedbackInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265PictureInfoEXT(reinterpret_cast<const VkVideoEncodeH265PictureInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265DpbSlotInfoEXT(reinterpret_cast<const VkVideoEncodeH265DpbSlotInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265ProfileInfoEXT(reinterpret_cast<const VkVideoEncodeH265ProfileInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265RateControlInfoEXT(reinterpret_cast<const VkVideoEncodeH265RateControlInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265RateControlLayerInfoEXT(reinterpret_cast<const VkVideoEncodeH265RateControlLayerInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_EXT:
                safe_pNext = new safe_VkVideoEncodeH265GopRemainingFrameInfoEXT(reinterpret_cast<const VkVideoEncodeH265GopRemainingFrameInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD:
                safe_pNext = new safe_VkTextureLODGatherFormatPropertiesAMD(reinterpret_cast<const VkTextureLODGatherFormatPropertiesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCornerSampledImageFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV:
                safe_pNext = new safe_VkExternalMemoryImageCreateInfoNV(reinterpret_cast<const VkExternalMemoryImageCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV:
                safe_pNext = new safe_VkExportMemoryAllocateInfoNV(reinterpret_cast<const VkExportMemoryAllocateInfoNV *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV:
                safe_pNext = new safe_VkImportMemoryWin32HandleInfoNV(reinterpret_cast<const VkImportMemoryWin32HandleInfoNV *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV:
                safe_pNext = new safe_VkExportMemoryWin32HandleInfoNV(reinterpret_cast<const VkExportMemoryWin32HandleInfoNV *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV:
                safe_pNext = new safe_VkWin32KeyedMutexAcquireReleaseInfoNV(reinterpret_cast<const VkWin32KeyedMutexAcquireReleaseInfoNV *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT:
                safe_pNext = new safe_VkValidationFlagsEXT(reinterpret_cast<const VkValidationFlagsEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT:
                safe_pNext = new safe_VkImageViewASTCDecodeModeEXT(reinterpret_cast<const VkImageViewASTCDecodeModeEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceASTCDecodeFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceASTCDecodeFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePipelineRobustnessFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePipelineRobustnessFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePipelineRobustnessPropertiesEXT(reinterpret_cast<const VkPhysicalDevicePipelineRobustnessPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRobustnessCreateInfoEXT(reinterpret_cast<const VkPipelineRobustnessCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT:
                safe_pNext = new safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT(reinterpret_cast<const VkCommandBufferInheritanceConditionalRenderingInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportWScalingStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportWScalingStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSwapchainCounterCreateInfoEXT(reinterpret_cast<const VkSwapchainCounterCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE:
                safe_pNext = new safe_VkPresentTimesInfoGOOGLE(reinterpret_cast<const VkPresentTimesInfoGOOGLE *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportSwizzleStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportSwizzleStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDiscardRectanglePropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineDiscardRectangleStateCreateInfoEXT(reinterpret_cast<const VkPipelineDiscardRectangleStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceConservativeRasterizationPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationConservativeStateCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationConservativeStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT:
                safe_pNext = new safe_VkDebugUtilsObjectNameInfoEXT(reinterpret_cast<const VkDebugUtilsObjectNameInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
                safe_pNext = new safe_VkDebugUtilsMessengerCreateInfoEXT(reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferUsageANDROID(reinterpret_cast<const VkAndroidHardwareBufferUsageANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferFormatPropertiesANDROID(reinterpret_cast<const VkAndroidHardwareBufferFormatPropertiesANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
                safe_pNext = new safe_VkImportAndroidHardwareBufferInfoANDROID(reinterpret_cast<const VkImportAndroidHardwareBufferInfoANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID:
                safe_pNext = new safe_VkExternalFormatANDROID(reinterpret_cast<const VkExternalFormatANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferFormatProperties2ANDROID(reinterpret_cast<const VkAndroidHardwareBufferFormatProperties2ANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX:
                safe_pNext = new safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX(reinterpret_cast<const VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX:
                safe_pNext = new safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX(reinterpret_cast<const VkPhysicalDeviceShaderEnqueuePropertiesAMDX *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX:
                safe_pNext = new safe_VkPipelineShaderStageNodeCreateInfoAMDX(reinterpret_cast<const VkPipelineShaderStageNodeCreateInfoAMDX *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT:
                safe_pNext = new safe_VkSampleLocationsInfoEXT(reinterpret_cast<const VkSampleLocationsInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT:
                safe_pNext = new safe_VkRenderPassSampleLocationsBeginInfoEXT(reinterpret_cast<const VkRenderPassSampleLocationsBeginInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineSampleLocationsStateCreateInfoEXT(reinterpret_cast<const VkPipelineSampleLocationsStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceSampleLocationsPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceSampleLocationsPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT(reinterpret_cast<const VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineCoverageToColorStateCreateInfoNV(reinterpret_cast<const VkPipelineCoverageToColorStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineCoverageModulationStateCreateInfoNV(reinterpret_cast<const VkPipelineCoverageModulationStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(reinterpret_cast<const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT:
                safe_pNext = new safe_VkDrmFormatModifierPropertiesListEXT(reinterpret_cast<const VkDrmFormatModifierPropertiesListEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT(reinterpret_cast<const VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageDrmFormatModifierListCreateInfoEXT(reinterpret_cast<const VkImageDrmFormatModifierListCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageDrmFormatModifierExplicitCreateInfoEXT(reinterpret_cast<const VkImageDrmFormatModifierExplicitCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT:
                safe_pNext = new safe_VkDrmFormatModifierPropertiesList2EXT(reinterpret_cast<const VkDrmFormatModifierPropertiesList2EXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkShaderModuleValidationCacheCreateInfoEXT(reinterpret_cast<const VkShaderModuleValidationCacheCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportShadingRateImageStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShadingRateImageFeaturesNV(reinterpret_cast<const VkPhysicalDeviceShadingRateImageFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShadingRateImagePropertiesNV(reinterpret_cast<const VkPhysicalDeviceShadingRateImagePropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV:
                safe_pNext = new safe_VkWriteDescriptorSetAccelerationStructureNV(reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPropertiesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(reinterpret_cast<const VkPipelineRepresentativeFragmentTestStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageViewImageFormatInfoEXT(reinterpret_cast<const VkPhysicalDeviceImageViewImageFormatInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT:
                safe_pNext = new safe_VkFilterCubicImageViewImageFormatPropertiesEXT(reinterpret_cast<const VkFilterCubicImageViewImageFormatPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
                safe_pNext = new safe_VkImportMemoryHostPointerInfoEXT(reinterpret_cast<const VkImportMemoryHostPointerInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceExternalMemoryHostPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD:
                safe_pNext = new safe_VkPipelineCompilerControlCreateInfoAMD(reinterpret_cast<const VkPipelineCompilerControlCreateInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD:
                safe_pNext = new safe_VkPhysicalDeviceShaderCorePropertiesAMD(reinterpret_cast<const VkPhysicalDeviceShaderCorePropertiesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD:
                safe_pNext = new safe_VkDeviceMemoryOverallocationCreateInfoAMD(reinterpret_cast<const VkDeviceMemoryOverallocationCreateInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineVertexInputDivisorStateCreateInfoEXT(reinterpret_cast<const VkPipelineVertexInputDivisorStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_GGP
            case VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP:
                safe_pNext = new safe_VkPresentFrameTokenGGP(reinterpret_cast<const VkPresentFrameTokenGGP *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_GGP
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV(reinterpret_cast<const VkPhysicalDeviceComputeShaderDerivativesFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderFeaturesNV(reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderPropertiesNV(reinterpret_cast<const VkPhysicalDeviceMeshShaderPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(reinterpret_cast<const VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(reinterpret_cast<const VkPipelineViewportExclusiveScissorStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(reinterpret_cast<const VkPhysicalDeviceExclusiveScissorFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV:
                safe_pNext = new safe_VkQueueFamilyCheckpointPropertiesNV(reinterpret_cast<const VkQueueFamilyCheckpointPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL:
                safe_pNext = new safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(reinterpret_cast<const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL:
                safe_pNext = new safe_VkQueryPoolPerformanceQueryCreateInfoINTEL(reinterpret_cast<const VkQueryPoolPerformanceQueryCreateInfoINTEL *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePCIBusInfoPropertiesEXT(reinterpret_cast<const VkPhysicalDevicePCIBusInfoPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD:
                safe_pNext = new safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(reinterpret_cast<const VkDisplayNativeHdrSurfaceCapabilitiesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD:
                safe_pNext = new safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(reinterpret_cast<const VkSwapchainDisplayNativeHdrCreateInfoAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT:
                safe_pNext = new safe_VkRenderPassFragmentDensityMapCreateInfoEXT(reinterpret_cast<const VkRenderPassFragmentDensityMapCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD:
                safe_pNext = new safe_VkPhysicalDeviceShaderCoreProperties2AMD(reinterpret_cast<const VkPhysicalDeviceShaderCoreProperties2AMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD:
                safe_pNext = new safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD(reinterpret_cast<const VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT:
                safe_pNext = new safe_VkMemoryPriorityAllocateInfoEXT(reinterpret_cast<const VkMemoryPriorityAllocateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT:
                safe_pNext = new safe_VkBufferDeviceAddressCreateInfoEXT(reinterpret_cast<const VkBufferDeviceAddressCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT:
                safe_pNext = new safe_VkValidationFeaturesEXT(reinterpret_cast<const VkValidationFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(reinterpret_cast<const VkPhysicalDeviceCooperativeMatrixPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineCoverageReductionStateCreateInfoNV(reinterpret_cast<const VkPipelineCoverageReductionStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceProvokingVertexFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceProvokingVertexFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceProvokingVertexPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceProvokingVertexPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT:
                safe_pNext = new safe_VkSurfaceFullScreenExclusiveInfoEXT(reinterpret_cast<const VkSurfaceFullScreenExclusiveInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT:
                safe_pNext = new safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT(reinterpret_cast<const VkSurfaceCapabilitiesFullScreenExclusiveEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT:
                safe_pNext = new safe_VkSurfaceFullScreenExclusiveWin32InfoEXT(reinterpret_cast<const VkSurfaceFullScreenExclusiveWin32InfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_WIN32_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceLineRasterizationFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceLineRasterizationFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceLineRasterizationPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceLineRasterizationPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineRasterizationLineStateCreateInfoEXT(reinterpret_cast<const VkPipelineRasterizationLineStateCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceIndexTypeUint8FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceHostImageCopyFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceHostImageCopyPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceHostImageCopyPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE_EXT:
                safe_pNext = new safe_VkSubresourceHostMemcpySizeEXT(reinterpret_cast<const VkSubresourceHostMemcpySizeEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY_EXT:
                safe_pNext = new safe_VkHostImageCopyDevicePerformanceQueryEXT(reinterpret_cast<const VkHostImageCopyDevicePerformanceQueryEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT:
                safe_pNext = new safe_VkSurfacePresentModeEXT(reinterpret_cast<const VkSurfacePresentModeEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT:
                safe_pNext = new safe_VkSurfacePresentScalingCapabilitiesEXT(reinterpret_cast<const VkSurfacePresentScalingCapabilitiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT:
                safe_pNext = new safe_VkSurfacePresentModeCompatibilityEXT(reinterpret_cast<const VkSurfacePresentModeCompatibilityEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT:
                safe_pNext = new safe_VkSwapchainPresentFenceInfoEXT(reinterpret_cast<const VkSwapchainPresentFenceInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSwapchainPresentModesCreateInfoEXT(reinterpret_cast<const VkSwapchainPresentModesCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT:
                safe_pNext = new safe_VkSwapchainPresentModeInfoEXT(reinterpret_cast<const VkSwapchainPresentModeInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSwapchainPresentScalingCreateInfoEXT(reinterpret_cast<const VkSwapchainPresentScalingCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV:
                safe_pNext = new safe_VkGraphicsPipelineShaderGroupsCreateInfoNV(reinterpret_cast<const VkGraphicsPipelineShaderGroupsCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV(reinterpret_cast<const VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV:
                safe_pNext = new safe_VkCommandBufferInheritanceViewportScissorInfoNV(reinterpret_cast<const VkCommandBufferInheritanceViewportScissorInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM:
                safe_pNext = new safe_VkRenderPassTransformBeginInfoQCOM(reinterpret_cast<const VkRenderPassTransformBeginInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM:
                safe_pNext = new safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM(reinterpret_cast<const VkCommandBufferInheritanceRenderPassTransformInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT:
                safe_pNext = new safe_VkDepthBiasRepresentationInfoEXT(reinterpret_cast<const VkDepthBiasRepresentationInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT:
                safe_pNext = new safe_VkDeviceDeviceMemoryReportCreateInfoEXT(reinterpret_cast<const VkDeviceDeviceMemoryReportCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceRobustness2FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceRobustness2PropertiesEXT(reinterpret_cast<const VkPhysicalDeviceRobustness2PropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSamplerCustomBorderColorCreateInfoEXT(reinterpret_cast<const VkSamplerCustomBorderColorCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceCustomBorderColorPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDevicePresentBarrierFeaturesNV(reinterpret_cast<const VkPhysicalDevicePresentBarrierFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV:
                safe_pNext = new safe_VkSurfaceCapabilitiesPresentBarrierNV(reinterpret_cast<const VkSurfaceCapabilitiesPresentBarrierNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV:
                safe_pNext = new safe_VkSwapchainPresentBarrierCreateInfoNV(reinterpret_cast<const VkSwapchainPresentBarrierCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV:
                safe_pNext = new safe_VkDeviceDiagnosticsConfigCreateInfoNV(reinterpret_cast<const VkDeviceDiagnosticsConfigCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV:
                safe_pNext = new safe_VkQueryLowLatencySupportNV(reinterpret_cast<const VkQueryLowLatencySupportNV *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalObjectCreateInfoEXT(reinterpret_cast<const VkExportMetalObjectCreateInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalDeviceInfoEXT(reinterpret_cast<const VkExportMetalDeviceInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalCommandQueueInfoEXT(reinterpret_cast<const VkExportMetalCommandQueueInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT:
                safe_pNext = new safe_VkExportMetalBufferInfoEXT(reinterpret_cast<const VkExportMetalBufferInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT:
                safe_pNext = new safe_VkImportMetalBufferInfoEXT(reinterpret_cast<const VkImportMetalBufferInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalTextureInfoEXT(reinterpret_cast<const VkExportMetalTextureInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT:
                safe_pNext = new safe_VkImportMetalTextureInfoEXT(reinterpret_cast<const VkImportMetalTextureInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT:
                safe_pNext = new safe_VkExportMetalIOSurfaceInfoEXT(reinterpret_cast<const VkExportMetalIOSurfaceInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT:
                safe_pNext = new safe_VkImportMetalIOSurfaceInfoEXT(reinterpret_cast<const VkImportMetalIOSurfaceInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT:
                safe_pNext = new safe_VkExportMetalSharedEventInfoEXT(reinterpret_cast<const VkExportMetalSharedEventInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT:
                safe_pNext = new safe_VkImportMetalSharedEventInfoEXT(reinterpret_cast<const VkImportMetalSharedEventInfoEXT *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_METAL_EXT
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT:
                safe_pNext = new safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT(reinterpret_cast<const VkDescriptorBufferBindingPushDescriptorBufferHandleEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT:
                safe_pNext = new safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT(reinterpret_cast<const VkOpaqueCaptureDescriptorDataCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT:
                safe_pNext = new safe_VkGraphicsPipelineLibraryCreateInfoEXT(reinterpret_cast<const VkGraphicsPipelineLibraryCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD:
                safe_pNext = new safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(reinterpret_cast<const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV(reinterpret_cast<const VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV:
                safe_pNext = new safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV(reinterpret_cast<const VkPipelineFragmentShadingRateEnumStateCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV:
                safe_pNext = new safe_VkAccelerationStructureGeometryMotionTrianglesDataNV(reinterpret_cast<const VkAccelerationStructureGeometryMotionTrianglesDataNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV:
                safe_pNext = new safe_VkAccelerationStructureMotionInfoNV(reinterpret_cast<const VkAccelerationStructureMotionInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMap2PropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM:
                safe_pNext = new safe_VkCopyCommandTransformInfoQCOM(reinterpret_cast<const VkCopyCommandTransformInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT:
                safe_pNext = new safe_VkImageCompressionControlEXT(reinterpret_cast<const VkImageCompressionControlEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT:
                safe_pNext = new safe_VkImageCompressionPropertiesEXT(reinterpret_cast<const VkImageCompressionPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevice4444FormatsFeaturesEXT(reinterpret_cast<const VkPhysicalDevice4444FormatsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFaultFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkMutableDescriptorTypeCreateInfoEXT(reinterpret_cast<const VkMutableDescriptorTypeCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDrmPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceDrmPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT:
                safe_pNext = new safe_VkDeviceAddressBindingCallbackDataEXT(reinterpret_cast<const VkDeviceAddressBindingCallbackDataEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthClipControlFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthClipControlFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineViewportDepthClipControlCreateInfoEXT(reinterpret_cast<const VkPipelineViewportDepthClipControlCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA:
                safe_pNext = new safe_VkImportMemoryZirconHandleInfoFUCHSIA(reinterpret_cast<const VkImportMemoryZirconHandleInfoFUCHSIA *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA:
                safe_pNext = new safe_VkImportMemoryBufferCollectionFUCHSIA(reinterpret_cast<const VkImportMemoryBufferCollectionFUCHSIA *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA:
                safe_pNext = new safe_VkBufferCollectionImageCreateInfoFUCHSIA(reinterpret_cast<const VkBufferCollectionImageCreateInfoFUCHSIA *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA:
                safe_pNext = new safe_VkBufferCollectionBufferCreateInfoFUCHSIA(reinterpret_cast<const VkBufferCollectionBufferCreateInfoFUCHSIA *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_FUCHSIA
            case VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI:
                safe_pNext = new safe_VkSubpassShadingPipelineCreateInfoHUAWEI(reinterpret_cast<const VkSubpassShadingPipelineCreateInfoHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI(reinterpret_cast<const VkPhysicalDeviceSubpassShadingPropertiesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV(reinterpret_cast<const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT:
                safe_pNext = new safe_VkFrameBoundaryEXT(reinterpret_cast<const VkFrameBoundaryEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT:
                safe_pNext = new safe_VkSubpassResolvePerformanceQueryEXT(reinterpret_cast<const VkSubpassResolvePerformanceQueryEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT:
                safe_pNext = new safe_VkMultisampledRenderToSingleSampledInfoEXT(reinterpret_cast<const VkMultisampledRenderToSingleSampledInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineColorWriteCreateInfoEXT(reinterpret_cast<const VkPipelineColorWriteCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageViewMinLodCreateInfoEXT(reinterpret_cast<const VkImageViewMinLodCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMultiDrawFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMultiDrawFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMultiDrawPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceMultiDrawPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderTileImageFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderTileImageFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderTileImagePropertiesEXT(reinterpret_cast<const VkPhysicalDeviceShaderTileImagePropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceOpacityMicromapPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT:
                safe_pNext = new safe_VkAccelerationStructureTrianglesOpacityMicromapEXT(reinterpret_cast<const VkAccelerationStructureTrianglesOpacityMicromapEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV(reinterpret_cast<const VkPhysicalDeviceDisplacementMicromapPropertiesNV *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV:
                safe_pNext = new safe_VkAccelerationStructureTrianglesDisplacementMicromapNV(reinterpret_cast<const VkAccelerationStructureTrianglesDisplacementMicromapNV *>(pNext), copy_state, false);
                break;
#endif // VK_ENABLE_BETA_EXTENSIONS
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI(reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI:
                safe_pNext = new safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI(reinterpret_cast<const VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT:
                safe_pNext = new safe_VkSamplerBorderColorComponentMappingCreateInfoEXT(reinterpret_cast<const VkSamplerBorderColorComponentMappingCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceShaderCorePropertiesARM(reinterpret_cast<const VkPhysicalDeviceShaderCorePropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT:
                safe_pNext = new safe_VkImageViewSlicedCreateInfoEXT(reinterpret_cast<const VkImageViewSlicedCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(reinterpret_cast<const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDepthClampZeroOneFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDepthClampZeroOneFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM(reinterpret_cast<const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM:
                safe_pNext = new safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM(reinterpret_cast<const VkSubpassFragmentDensityMapOffsetEndInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV(reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV(reinterpret_cast<const VkPhysicalDeviceCopyMemoryIndirectPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV(reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV(reinterpret_cast<const VkPhysicalDeviceMemoryDecompressionPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV(reinterpret_cast<const VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkImageViewSampleWeightCreateInfoQCOM(reinterpret_cast<const VkImageViewSampleWeightCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessingFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessingFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessingPropertiesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessingPropertiesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceNestedCommandBufferPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT:
                safe_pNext = new safe_VkExternalMemoryAcquireUnmodifiedEXT(reinterpret_cast<const VkExternalMemoryAcquireUnmodifiedEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT(reinterpret_cast<const VkPhysicalDeviceExtendedDynamicState3PropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT:
                safe_pNext = new safe_VkRenderPassCreationControlEXT(reinterpret_cast<const VkRenderPassCreationControlEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT:
                safe_pNext = new safe_VkRenderPassCreationFeedbackCreateInfoEXT(reinterpret_cast<const VkRenderPassCreationFeedbackCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT:
                safe_pNext = new safe_VkRenderPassSubpassFeedbackCreateInfoEXT(reinterpret_cast<const VkRenderPassSubpassFeedbackCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG:
                safe_pNext = new safe_VkDirectDriverLoadingListLUNARG(reinterpret_cast<const VkDirectDriverLoadingListLUNARG *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT:
                safe_pNext = new safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT(reinterpret_cast<const VkPipelineShaderStageModuleIdentifierCreateInfoEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceOpticalFlowFeaturesNV(reinterpret_cast<const VkPhysicalDeviceOpticalFlowFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceOpticalFlowPropertiesNV(reinterpret_cast<const VkPhysicalDeviceOpticalFlowPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV:
                safe_pNext = new safe_VkOpticalFlowImageFormatInfoNV(reinterpret_cast<const VkOpticalFlowImageFormatInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV:
                safe_pNext = new safe_VkOpticalFlowSessionCreatePrivateDataInfoNV(reinterpret_cast<const VkOpticalFlowSessionCreatePrivateDataInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePipelineProtectedAccessFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePipelineProtectedAccessFeaturesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID:
                safe_pNext = new safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID(reinterpret_cast<const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID:
                safe_pNext = new safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID(reinterpret_cast<const VkPhysicalDeviceExternalFormatResolvePropertiesANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID:
                safe_pNext = new safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID(reinterpret_cast<const VkAndroidHardwareBufferFormatResolvePropertiesANDROID *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderObjectFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceShaderObjectFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceShaderObjectPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceShaderObjectPropertiesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC:
                safe_pNext = new safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC(reinterpret_cast<const VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC:
                safe_pNext = new safe_VkAmigoProfilingSubmitInfoSEC(reinterpret_cast<const VkAmigoProfilingSubmitInfoSEC *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV(reinterpret_cast<const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV(reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV:
                safe_pNext = new safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV(reinterpret_cast<const VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM(reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM:
                safe_pNext = new safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM(reinterpret_cast<const VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT(reinterpret_cast<const VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV:
                safe_pNext = new safe_VkLatencySubmissionPresentIdNV(reinterpret_cast<const VkLatencySubmissionPresentIdNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV:
                safe_pNext = new safe_VkSwapchainLatencyCreateInfoNV(reinterpret_cast<const VkSwapchainLatencyCreateInfoNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV:
                safe_pNext = new safe_VkLatencySurfaceCapabilitiesNV(reinterpret_cast<const VkLatencySurfaceCapabilitiesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM:
                safe_pNext = new safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(reinterpret_cast<const VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM(reinterpret_cast<const VkPhysicalDeviceImageProcessing2PropertiesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkSamplerBlockMatchWindowCreateInfoQCOM(reinterpret_cast<const VkSamplerBlockMatchWindowCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkSamplerCubicWeightsCreateInfoQCOM(reinterpret_cast<const VkSamplerCubicWeightsCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM:
                safe_pNext = new safe_VkBlitImageCubicWeightsInfoQCOM(reinterpret_cast<const VkBlitImageCubicWeightsInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM:
                safe_pNext = new safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM(reinterpret_cast<const VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM:
                safe_pNext = new safe_VkPhysicalDeviceCubicClampFeaturesQCOM(reinterpret_cast<const VkPhysicalDeviceCubicClampFeaturesQCOM *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(pNext), copy_state, false);
                break;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX:
                safe_pNext = new safe_VkScreenBufferFormatPropertiesQNX(reinterpret_cast<const VkScreenBufferFormatPropertiesQNX *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX:
                safe_pNext = new safe_VkImportScreenBufferInfoQNX(reinterpret_cast<const VkImportScreenBufferInfoQNX *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX:
                safe_pNext = new safe_VkExternalFormatQNX(reinterpret_cast<const VkExternalFormatQNX *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX:
                safe_pNext = new safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX(reinterpret_cast<const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(pNext), copy_state, false);
                break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT:
                safe_pNext = new safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT(reinterpret_cast<const VkPhysicalDeviceLayeredDriverPropertiesMSFT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV:
                safe_pNext = new safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV(reinterpret_cast<const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR:
                safe_pNext = new safe_VkWriteDescriptorSetAccelerationStructureKHR(reinterpret_cast<const VkWriteDescriptorSetAccelerationStructureKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR(reinterpret_cast<const VkPhysicalDeviceAccelerationStructurePropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR(reinterpret_cast<const VkPhysicalDeviceRayTracingPipelinePropertiesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR:
                safe_pNext = new safe_VkPhysicalDeviceRayQueryFeaturesKHR(reinterpret_cast<const VkPhysicalDeviceRayQueryFeaturesKHR *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderFeaturesEXT(reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT *>(pNext), copy_state, false);
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT:
                safe_pNext = new safe_VkPhysicalDeviceMeshShaderPropertiesEXT(reinterpret_cast<const VkPhysicalDeviceMeshShaderPropertiesEXT *>(pNext), copy_state, false);
                break;
            default: // Encountered an unknown sType -- skip (do not copy) this entry in the chain
                // If sType is in custom list, construct blind copy
                for (auto item : custom_stype_info) {
                    if (item.first == header->sType) {
                        safe_pNext = malloc(item.second);
                        memcpy(safe_pNext, header, item.second);
                    }
                }
                break;
        }
        if (!first_pNext) {
            first_pNext = safe_pNext;
        }
        pNext = header->pNext;
        if (prev_pNext && safe_pNext) {
            prev_pNext->pNext = (VkBaseOutStructure*)safe_pNext;
        }
        if (safe_pNext) {
            prev_pNext = (VkBaseOutStructure*)safe_pNext;
        }
        safe_pNext = nullptr;
    }

    return first_pNext;
}

void FreePnextChain(const void *pNext) {
    if (!pNext) return;

    auto header = reinterpret_cast<const VkBaseOutStructure *>(pNext);

    switch (header->sType) {
        // Special-case Loader Instance Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO:
            FreePnextChain(header->pNext);
            delete reinterpret_cast<const VkLayerInstanceCreateInfo *>(pNext);
            break;
        // Special-case Loader Device Struct passed to/from layer in pNext chain
        case VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO:
            FreePnextChain(header->pNext);
            delete reinterpret_cast<const VkLayerDeviceCreateInfo *>(pNext);
            break;

        case VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkShaderModuleCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSubgroupProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDevice16BitStorageFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS:
            delete reinterpret_cast<const safe_VkMemoryDedicatedRequirements *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
            delete reinterpret_cast<const safe_VkMemoryDedicatedAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
            delete reinterpret_cast<const safe_VkMemoryAllocateFlagsInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_RENDER_PASS_BEGIN_INFO:
            delete reinterpret_cast<const safe_VkDeviceGroupRenderPassBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO:
            delete reinterpret_cast<const safe_VkDeviceGroupCommandBufferBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO:
            delete reinterpret_cast<const safe_VkDeviceGroupSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO:
            delete reinterpret_cast<const safe_VkDeviceGroupBindSparseInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_DEVICE_GROUP_INFO:
            delete reinterpret_cast<const safe_VkBindBufferMemoryDeviceGroupInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_DEVICE_GROUP_INFO:
            delete reinterpret_cast<const safe_VkBindImageMemoryDeviceGroupInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkDeviceGroupDeviceCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFeatures2 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDevicePointClippingProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO:
            delete reinterpret_cast<const safe_VkRenderPassInputAttachmentAspectCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkImageViewUsageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkPipelineTessellationDomainOriginStateCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO:
            delete reinterpret_cast<const safe_VkRenderPassMultiviewCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultiviewFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultiviewProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVariablePointersFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceProtectedMemoryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceProtectedMemoryProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PROTECTED_SUBMIT_INFO:
            delete reinterpret_cast<const safe_VkProtectedSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO:
            delete reinterpret_cast<const safe_VkSamplerYcbcrConversionInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO:
            delete reinterpret_cast<const safe_VkBindImagePlaneMemoryInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO:
            delete reinterpret_cast<const safe_VkImagePlaneMemoryRequirementsInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_IMAGE_FORMAT_PROPERTIES:
            delete reinterpret_cast<const safe_VkSamplerYcbcrConversionImageFormatProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExternalImageFormatInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
            delete reinterpret_cast<const safe_VkExternalImageFormatProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceIDProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkExternalMemoryImageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO:
            delete reinterpret_cast<const safe_VkExternalMemoryBufferCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
            delete reinterpret_cast<const safe_VkExportMemoryAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkExportFenceCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkExportSemaphoreCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_3_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMaintenance3Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderDrawParametersFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVulkan11Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVulkan11Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVulkan12Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVulkan12Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
            delete reinterpret_cast<const safe_VkImageFormatListCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDevice8BitStorageFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDriverProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderAtomicInt64Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderFloat16Int8Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFloatControlsProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO:
            delete reinterpret_cast<const safe_VkDescriptorSetLayoutBindingFlagsCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDescriptorIndexingFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDescriptorIndexingProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO:
            delete reinterpret_cast<const safe_VkDescriptorSetVariableDescriptorCountAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_LAYOUT_SUPPORT:
            delete reinterpret_cast<const safe_VkDescriptorSetVariableDescriptorCountLayoutSupport *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE:
            delete reinterpret_cast<const safe_VkSubpassDescriptionDepthStencilResolve *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDepthStencilResolveProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceScalarBlockLayoutFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkImageStencilUsageCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkSamplerReductionModeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSamplerFilterMinmaxProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVulkanMemoryModelFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImagelessFramebufferFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO:
            delete reinterpret_cast<const safe_VkFramebufferAttachmentsCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO:
            delete reinterpret_cast<const safe_VkRenderPassAttachmentBeginInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT:
            delete reinterpret_cast<const safe_VkAttachmentReferenceStencilLayout *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT:
            delete reinterpret_cast<const safe_VkAttachmentDescriptionStencilLayout *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceHostQueryResetFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTimelineSemaphoreFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTimelineSemaphoreProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkSemaphoreTypeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO:
            delete reinterpret_cast<const safe_VkTimelineSemaphoreSubmitInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceBufferDeviceAddressFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO:
            delete reinterpret_cast<const safe_VkBufferOpaqueCaptureAddressCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO:
            delete reinterpret_cast<const safe_VkMemoryOpaqueCaptureAddressAllocateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVulkan13Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVulkan13Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO:
            delete reinterpret_cast<const safe_VkPipelineCreationFeedbackCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderTerminateInvocationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDevicePrivateDataFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_PRIVATE_DATA_CREATE_INFO:
            delete reinterpret_cast<const safe_VkDevicePrivateDataCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDevicePipelineCreationCacheControlFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_BARRIER_2:
            delete reinterpret_cast<const safe_VkMemoryBarrier2 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSynchronization2Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageRobustnessFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSubgroupSizeControlFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSubgroupSizeControlProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO:
            delete reinterpret_cast<const safe_VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceInlineUniformBlockFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceInlineUniformBlockProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK:
            delete reinterpret_cast<const safe_VkWriteDescriptorSetInlineUniformBlock *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_INLINE_UNIFORM_BLOCK_CREATE_INFO:
            delete reinterpret_cast<const safe_VkDescriptorPoolInlineUniformBlockCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO:
            delete reinterpret_cast<const safe_VkPipelineRenderingCreateInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDynamicRenderingFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO:
            delete reinterpret_cast<const safe_VkCommandBufferInheritanceRenderingInfo *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderIntegerDotProductFeatures *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderIntegerDotProductProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTexelBufferAlignmentProperties *>(header);
            break;
        case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3:
            delete reinterpret_cast<const safe_VkFormatProperties3 *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMaintenance4Features *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMaintenance4Properties *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkImageSwapchainCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR:
            delete reinterpret_cast<const safe_VkBindImageMemorySwapchainInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR:
            delete reinterpret_cast<const safe_VkDeviceGroupPresentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkDeviceGroupSwapchainCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR:
            delete reinterpret_cast<const safe_VkDisplayPresentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_QUERY_RESULT_STATUS_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkQueueFamilyQueryResultStatusPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkQueueFamilyVideoPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoProfileListInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_USAGE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeUsageInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH264ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH264CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH264SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH264SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH264PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH264DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
            delete reinterpret_cast<const safe_VkRenderingFragmentShadingRateAttachmentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_INFO_EXT:
            delete reinterpret_cast<const safe_VkRenderingFragmentDensityMapAttachmentInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD:
            delete reinterpret_cast<const safe_VkAttachmentSampleCountInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX:
            delete reinterpret_cast<const safe_VkMultiviewPerViewAttributesInfoNVX *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<const safe_VkImportMemoryWin32HandleInfoKHR *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<const safe_VkExportMemoryWin32HandleInfoKHR *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
            delete reinterpret_cast<const safe_VkImportMemoryFdInfoKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_KHR:
            delete reinterpret_cast<const safe_VkWin32KeyedMutexAcquireReleaseInfoKHR *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<const safe_VkExportSemaphoreWin32HandleInfoKHR *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR:
            delete reinterpret_cast<const safe_VkD3D12FenceSubmitInfoKHR *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePushDescriptorPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR:
            delete reinterpret_cast<const safe_VkPresentRegionsKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SHARED_PRESENT_SURFACE_CAPABILITIES_KHR:
            delete reinterpret_cast<const safe_VkSharedPresentSurfaceCapabilitiesKHR *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_FENCE_WIN32_HANDLE_INFO_KHR:
            delete reinterpret_cast<const safe_VkExportFenceWin32HandleInfoKHR *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePerformanceQueryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePerformanceQueryPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkQueryPoolPerformanceCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PERFORMANCE_QUERY_SUBMIT_INFO_KHR:
            delete reinterpret_cast<const safe_VkPerformanceQuerySubmitInfoKHR *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePortabilitySubsetPropertiesKHR *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderClockFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PROFILE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH265ProfileInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_CAPABILITIES_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH265CapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_ADD_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH265SessionParametersAddInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_SESSION_PARAMETERS_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH265SessionParametersCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_PICTURE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH265PictureInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_VIDEO_DECODE_H265_DPB_SLOT_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoDecodeH265DpbSlotInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_QUEUE_GLOBAL_PRIORITY_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkDeviceQueueGlobalPriorityCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceGlobalPriorityQueryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_GLOBAL_PRIORITY_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkQueueFamilyGlobalPriorityPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR:
            delete reinterpret_cast<const safe_VkFragmentShadingRateAttachmentInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkPipelineFragmentShadingRateStateCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentShadingRatePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PROTECTED_CAPABILITIES_KHR:
            delete reinterpret_cast<const safe_VkSurfaceProtectedCapabilitiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePresentWaitFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkPipelineLibraryCreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_ID_KHR:
            delete reinterpret_cast<const safe_VkPresentIdKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDevicePresentIdFeaturesKHR *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR:
            delete reinterpret_cast<const safe_VkVideoEncodeCapabilitiesKHR *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_QUERY_POOL_VIDEO_ENCODE_FEEDBACK_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkQueryPoolVideoEncodeFeedbackCreateInfoKHR *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_USAGE_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoEncodeUsageInfoKHR *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_RATE_CONTROL_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoEncodeRateControlInfoKHR *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_QUALITY_LEVEL_INFO_KHR:
            delete reinterpret_cast<const safe_VkVideoEncodeQualityLevelInfoKHR *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV:
            delete reinterpret_cast<const safe_VkQueueFamilyCheckpointProperties2NV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentShaderBarycentricPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_UNIFORM_CONTROL_FLOW_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_WORKGROUP_MEMORY_EXPLICIT_LAYOUT_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMaintenance5FeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMaintenance5PropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkPipelineCreateFlags2CreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO_KHR:
            delete reinterpret_cast<const safe_VkBufferUsageFlags2CreateInfoKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCooperativeMatrixPropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkDebugReportCallbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD:
            delete reinterpret_cast<const safe_VkPipelineRasterizationStateRasterizationOrderAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkDedicatedAllocationImageCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkDedicatedAllocationBufferCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV:
            delete reinterpret_cast<const safe_VkDedicatedAllocationMemoryAllocateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTransformFeedbackPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineRasterizationStateStreamCreateInfoEXT *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264CapabilitiesEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_QUALITY_LEVEL_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264QualityLevelPropertiesEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264SessionCreateInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_ADD_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264SessionParametersAddInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264SessionParametersCreateInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_GET_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264SessionParametersGetInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_SESSION_PARAMETERS_FEEDBACK_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264SessionParametersFeedbackInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PICTURE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264PictureInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_DPB_SLOT_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264DpbSlotInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264ProfileInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264RateControlInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_RATE_CONTROL_LAYER_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264RateControlLayerInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_GOP_REMAINING_FRAME_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH264GopRemainingFrameInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265CapabilitiesEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265SessionCreateInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_QUALITY_LEVEL_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265QualityLevelPropertiesEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_ADD_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265SessionParametersAddInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265SessionParametersCreateInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_GET_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265SessionParametersGetInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_SESSION_PARAMETERS_FEEDBACK_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265SessionParametersFeedbackInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PICTURE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265PictureInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_DPB_SLOT_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265DpbSlotInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265ProfileInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265RateControlInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_RATE_CONTROL_LAYER_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265RateControlLayerInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_GOP_REMAINING_FRAME_INFO_EXT:
            delete reinterpret_cast<const safe_VkVideoEncodeH265GopRemainingFrameInfoEXT *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD:
            delete reinterpret_cast<const safe_VkTextureLODGatherFormatPropertiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkExternalMemoryImageCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV:
            delete reinterpret_cast<const safe_VkExportMemoryAllocateInfoNV *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV:
            delete reinterpret_cast<const safe_VkImportMemoryWin32HandleInfoNV *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV:
            delete reinterpret_cast<const safe_VkExportMemoryWin32HandleInfoNV *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV:
            delete reinterpret_cast<const safe_VkWin32KeyedMutexAcquireReleaseInfoNV *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT:
            delete reinterpret_cast<const safe_VkValidationFlagsEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT:
            delete reinterpret_cast<const safe_VkImageViewASTCDecodeModeEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ASTC_DECODE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceASTCDecodeFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePipelineRobustnessFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePipelineRobustnessPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_ROBUSTNESS_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineRobustnessCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT:
            delete reinterpret_cast<const safe_VkCommandBufferInheritanceConditionalRenderingInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineViewportWScalingStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSwapchainCounterCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE:
            delete reinterpret_cast<const safe_VkPresentTimesInfoGOOGLE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineViewportSwizzleStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDiscardRectanglePropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineDiscardRectangleStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceConservativeRasterizationPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineRasterizationConservativeStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT:
            delete reinterpret_cast<const safe_VkDebugUtilsObjectNameInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkDebugUtilsMessengerCreateInfoEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID:
            delete reinterpret_cast<const safe_VkAndroidHardwareBufferUsageANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID:
            delete reinterpret_cast<const safe_VkAndroidHardwareBufferFormatPropertiesANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
            delete reinterpret_cast<const safe_VkImportAndroidHardwareBufferInfoANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID:
            delete reinterpret_cast<const safe_VkExternalFormatANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID:
            delete reinterpret_cast<const safe_VkAndroidHardwareBufferFormatProperties2ANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX:
            delete reinterpret_cast<const safe_VkPipelineShaderStageNodeCreateInfoAMDX *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT:
            delete reinterpret_cast<const safe_VkSampleLocationsInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT:
            delete reinterpret_cast<const safe_VkRenderPassSampleLocationsBeginInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineSampleLocationsStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSampleLocationsPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineCoverageToColorStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineCoverageModulationStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT:
            delete reinterpret_cast<const safe_VkDrmFormatModifierPropertiesListEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageDrmFormatModifierInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkImageDrmFormatModifierListCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkImageDrmFormatModifierExplicitCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_2_EXT:
            delete reinterpret_cast<const safe_VkDrmFormatModifierPropertiesList2EXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SHADER_MODULE_VALIDATION_CACHE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkShaderModuleValidationCacheCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShadingRateImageFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShadingRateImagePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV:
            delete reinterpret_cast<const safe_VkWriteDescriptorSetAccelerationStructureNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_IMAGE_FORMAT_INFO_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageViewImageFormatInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_FILTER_CUBIC_IMAGE_VIEW_IMAGE_FORMAT_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkFilterCubicImageViewImageFormatPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
            delete reinterpret_cast<const safe_VkImportMemoryHostPointerInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExternalMemoryHostPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD:
            delete reinterpret_cast<const safe_VkPipelineCompilerControlCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderCorePropertiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD:
            delete reinterpret_cast<const safe_VkDeviceMemoryOverallocationCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineVertexInputDivisorStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_GGP
        case VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP:
            delete reinterpret_cast<const safe_VkPresentFrameTokenGGP *>(header);
            break;
#endif // VK_USE_PLATFORM_GGP
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceComputeShaderDerivativesFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMeshShaderFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMeshShaderPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkQueueFamilyCheckpointPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL:
            delete reinterpret_cast<const safe_VkQueryPoolPerformanceQueryCreateInfoINTEL *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PCI_BUS_INFO_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePCIBusInfoPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD:
            delete reinterpret_cast<const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD:
            delete reinterpret_cast<const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentDensityMapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkRenderPassFragmentDensityMapCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderCoreProperties2AMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMemoryBudgetPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MEMORY_PRIORITY_ALLOCATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkMemoryPriorityAllocateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceBufferDeviceAddressFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkBufferDeviceAddressCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkValidationFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineCoverageReductionStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_IMAGE_ARRAYS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceProvokingVertexFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROVOKING_VERTEX_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceProvokingVertexPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSurfaceFullScreenExclusiveInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_FULL_SCREEN_EXCLUSIVE_EXT:
            delete reinterpret_cast<const safe_VkSurfaceCapabilitiesFullScreenExclusiveEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT:
            delete reinterpret_cast<const safe_VkSurfaceFullScreenExclusiveWin32InfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_WIN32_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceLineRasterizationFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceLineRasterizationPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineRasterizationLineStateCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceIndexTypeUint8FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceHostImageCopyFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceHostImageCopyPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBRESOURCE_HOST_MEMCPY_SIZE_EXT:
            delete reinterpret_cast<const safe_VkSubresourceHostMemcpySizeEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY_EXT:
            delete reinterpret_cast<const safe_VkHostImageCopyDevicePerformanceQueryEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT:
            delete reinterpret_cast<const safe_VkSurfacePresentModeEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT:
            delete reinterpret_cast<const safe_VkSurfacePresentScalingCapabilitiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT:
            delete reinterpret_cast<const safe_VkSurfacePresentModeCompatibilityEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_FENCE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSwapchainPresentFenceInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODES_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSwapchainPresentModesCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_MODE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSwapchainPresentModeInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSwapchainPresentScalingCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkGraphicsPipelineShaderGroupsCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV:
            delete reinterpret_cast<const safe_VkCommandBufferInheritanceViewportScissorInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM:
            delete reinterpret_cast<const safe_VkRenderPassTransformBeginInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM:
            delete reinterpret_cast<const safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_BIAS_CONTROL_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT:
            delete reinterpret_cast<const safe_VkDepthBiasRepresentationInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_MEMORY_REPORT_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_DEVICE_MEMORY_REPORT_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkDeviceDeviceMemoryReportCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRobustness2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRobustness2PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSamplerCustomBorderColorCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCustomBorderColorPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDevicePresentBarrierFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV:
            delete reinterpret_cast<const safe_VkSurfaceCapabilitiesPresentBarrierNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkSwapchainPresentBarrierCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkDeviceDiagnosticsConfigCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV:
            delete reinterpret_cast<const safe_VkQueryLowLatencySupportNV *>(header);
            break;
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_OBJECT_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkExportMetalObjectCreateInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_DEVICE_INFO_EXT:
            delete reinterpret_cast<const safe_VkExportMetalDeviceInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_COMMAND_QUEUE_INFO_EXT:
            delete reinterpret_cast<const safe_VkExportMetalCommandQueueInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_BUFFER_INFO_EXT:
            delete reinterpret_cast<const safe_VkExportMetalBufferInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_BUFFER_INFO_EXT:
            delete reinterpret_cast<const safe_VkImportMetalBufferInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_TEXTURE_INFO_EXT:
            delete reinterpret_cast<const safe_VkExportMetalTextureInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_TEXTURE_INFO_EXT:
            delete reinterpret_cast<const safe_VkImportMetalTextureInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_IO_SURFACE_INFO_EXT:
            delete reinterpret_cast<const safe_VkExportMetalIOSurfaceInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_IO_SURFACE_INFO_EXT:
            delete reinterpret_cast<const safe_VkImportMetalIOSurfaceInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_EXPORT_METAL_SHARED_EVENT_INFO_EXT:
            delete reinterpret_cast<const safe_VkExportMetalSharedEventInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
#ifdef VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_IMPORT_METAL_SHARED_EVENT_INFO_EXT:
            delete reinterpret_cast<const safe_VkImportMetalSharedEventInfoEXT *>(header);
            break;
#endif // VK_USE_PLATFORM_METAL_EXT
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDescriptorBufferPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_DENSITY_MAP_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDescriptorBufferDensityMapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_PUSH_DESCRIPTOR_BUFFER_HANDLE_EXT:
            delete reinterpret_cast<const safe_VkDescriptorBufferBindingPushDescriptorBufferHandleEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPAQUE_CAPTURE_DESCRIPTOR_DATA_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkOpaqueCaptureDescriptorDataCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceGraphicsPipelineLibraryPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkGraphicsPipelineLibraryCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV:
            delete reinterpret_cast<const safe_VkAccelerationStructureGeometryMotionTrianglesDataNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV:
            delete reinterpret_cast<const safe_VkAccelerationStructureMotionInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_2_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentDensityMap2PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM:
            delete reinterpret_cast<const safe_VkCopyCommandTransformInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT:
            delete reinterpret_cast<const safe_VkImageCompressionControlEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkImageCompressionPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevice4444FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFaultFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MUTABLE_DESCRIPTOR_TYPE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkMutableDescriptorTypeCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDrmPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DEVICE_ADDRESS_BINDING_CALLBACK_DATA_EXT:
            delete reinterpret_cast<const safe_VkDeviceAddressBindingCallbackDataEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_CONTROL_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDepthClipControlFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineViewportDepthClipControlCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVE_TOPOLOGY_LIST_RESTART_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA:
            delete reinterpret_cast<const safe_VkImportMemoryZirconHandleInfoFUCHSIA *>(header);
            break;
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA:
            delete reinterpret_cast<const safe_VkImportMemoryBufferCollectionFUCHSIA *>(header);
            break;
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA:
            delete reinterpret_cast<const safe_VkBufferCollectionImageCreateInfoFUCHSIA *>(header);
            break;
#endif // VK_USE_PLATFORM_FUCHSIA
#ifdef VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA:
            delete reinterpret_cast<const safe_VkBufferCollectionBufferCreateInfoFUCHSIA *>(header);
            break;
#endif // VK_USE_PLATFORM_FUCHSIA
        case VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI:
            delete reinterpret_cast<const safe_VkSubpassShadingPipelineCreateInfoHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI:
            delete reinterpret_cast<const safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROPERTIES_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_FRAME_BOUNDARY_EXT:
            delete reinterpret_cast<const safe_VkFrameBoundaryEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_RESOLVE_PERFORMANCE_QUERY_EXT:
            delete reinterpret_cast<const safe_VkSubpassResolvePerformanceQueryEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT:
            delete reinterpret_cast<const safe_VkMultisampledRenderToSingleSampledInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_COLOR_WRITE_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineColorWriteCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_MIN_LOD_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkImageViewMinLodCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultiDrawFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTI_DRAW_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultiDrawPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_2D_VIEW_OF_3D_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderTileImageFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TILE_IMAGE_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderTileImagePropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceOpacityMicromapPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT:
            delete reinterpret_cast<const safe_VkAccelerationStructureTrianglesOpacityMicromapEXT *>(header);
            break;
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV:
            delete reinterpret_cast<const safe_VkAccelerationStructureTrianglesDisplacementMicromapNV *>(header);
            break;
#endif // VK_ENABLE_BETA_EXTENSIONS
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI:
            delete reinterpret_cast<const safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI:
            delete reinterpret_cast<const safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BORDER_COLOR_SWIZZLE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_BORDER_COLOR_COMPONENT_MAPPING_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkSamplerBorderColorComponentMappingCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PAGEABLE_DEVICE_LOCAL_MEMORY_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderCorePropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkImageViewSlicedCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLAMP_ZERO_ONE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDepthClampZeroOneFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NON_SEAMLESS_CUBE_MAP_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM:
            delete reinterpret_cast<const safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_COMPRESSION_CONTROL_SWAPCHAIN_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM:
            delete reinterpret_cast<const safe_VkImageViewSampleWeightCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageProcessingFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageProcessingPropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_NESTED_COMMAND_BUFFER_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceNestedCommandBufferPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_ACQUIRE_UNMODIFIED_EXT:
            delete reinterpret_cast<const safe_VkExternalMemoryAcquireUnmodifiedEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExtendedDynamicState3PropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_MERGE_FEEDBACK_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_CONTROL_EXT:
            delete reinterpret_cast<const safe_VkRenderPassCreationControlEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_CREATION_FEEDBACK_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkRenderPassCreationFeedbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_RENDER_PASS_SUBPASS_FEEDBACK_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkRenderPassSubpassFeedbackCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG:
            delete reinterpret_cast<const safe_VkDirectDriverLoadingListLUNARG *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MODULE_IDENTIFIER_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderModuleIdentifierPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_MODULE_IDENTIFIER_CREATE_INFO_EXT:
            delete reinterpret_cast<const safe_VkPipelineShaderStageModuleIdentifierCreateInfoEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceOpticalFlowFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceOpticalFlowPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV:
            delete reinterpret_cast<const safe_VkOpticalFlowImageFormatInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV:
            delete reinterpret_cast<const safe_VkOpticalFlowSessionCreatePrivateDataInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LEGACY_DITHERING_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePipelineProtectedAccessFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID:
            delete reinterpret_cast<const safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID *>(header);
            break;
#endif // VK_USE_PLATFORM_ANDROID_KHR
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderObjectFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderObjectPropertiesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC:
            delete reinterpret_cast<const safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(header);
            break;
        case VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC:
            delete reinterpret_cast<const safe_VkAmigoProfilingSubmitInfoSEC *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_LIBRARY_GROUP_HANDLES_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_UNUSED_ATTACHMENTS_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV:
            delete reinterpret_cast<const safe_VkLatencySubmissionPresentIdNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV:
            delete reinterpret_cast<const safe_VkSwapchainLatencyCreateInfoNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV:
            delete reinterpret_cast<const safe_VkLatencySurfaceCapabilitiesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM:
            delete reinterpret_cast<const safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM:
            delete reinterpret_cast<const safe_VkSamplerBlockMatchWindowCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM:
            delete reinterpret_cast<const safe_VkSamplerCubicWeightsCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM:
            delete reinterpret_cast<const safe_VkBlitImageCubicWeightsInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM:
            delete reinterpret_cast<const safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM:
            delete reinterpret_cast<const safe_VkPhysicalDeviceCubicClampFeaturesQCOM *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ATTACHMENT_FEEDBACK_LOOP_DYNAMIC_STATE_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(header);
            break;
#ifdef VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX:
            delete reinterpret_cast<const safe_VkScreenBufferFormatPropertiesQNX *>(header);
            break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX:
            delete reinterpret_cast<const safe_VkImportScreenBufferInfoQNX *>(header);
            break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX:
            delete reinterpret_cast<const safe_VkExternalFormatQNX *>(header);
            break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX:
            delete reinterpret_cast<const safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(header);
            break;
#endif // VK_USE_PLATFORM_SCREEN_QNX
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV:
            delete reinterpret_cast<const safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(header);
            break;
        case VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR:
            delete reinterpret_cast<const safe_VkWriteDescriptorSetAccelerationStructureKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceAccelerationStructurePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayTracingPipelinePropertiesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR:
            delete reinterpret_cast<const safe_VkPhysicalDeviceRayQueryFeaturesKHR *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMeshShaderFeaturesEXT *>(header);
            break;
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT:
            delete reinterpret_cast<const safe_VkPhysicalDeviceMeshShaderPropertiesEXT *>(header);
            break;
        default: // Encountered an unknown sType
            // If sType is in custom list, free custom struct memory and clean up
            for (auto item : custom_stype_info) {
                if (item.first == header->sType) {
                    if (header->pNext) {
                        FreePnextChain(header->pNext);
                    }
                    free(const_cast<void *>(pNext));
                    pNext = nullptr;
                    break;
                }
            }
            if (pNext) {
                FreePnextChain(header->pNext);
            }
            break;
    }
}  // clang-format on

// NOLINTEND
