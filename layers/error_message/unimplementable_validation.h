/*
 * Copyright (c) 2023-2024 LunarG, Inc.
 * Copyright (c) 2023-2024 Valve Corporation
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
#pragma once
// clang-format off

// This file list all VUID that are not possible to validate.
// This file should never be included, but here for searchability and statistics

const char* unimplementable_validation[] = {
    // sparseAddressSpaceSize can't be tracked in a layer
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/2403
    "VUID-vkCreateBuffer-flags-00911",

    // Some of the early extensions were not created with a feature bit. This means if the extension is used, we considered it
    // "enabled". This becomes a problem as some coniditional VUIDs depend on the Extension to be enabled, this means we are left
    // with 2 variations of the VUIDs, but only one is not possible to ever get to.
    // The following are a list of these:
    "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06869",  // VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872

    // This VUID cannot be validated at vkCmdEndDebugUtilsLabelEXT time. Needs spec clarification.
    // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5671
    "VUID-vkCmdEndDebugUtilsLabelEXT-commandBuffer-01912",

    // These VUIDs cannot be validated beyond making sure the pointer is not null
    "VUID-VkMemoryToImageCopyEXT-pHostPointer-09061", "VUID-VkImageToMemoryCopyEXT-pHostPointer-09066"

    // these are already taken care in spirv-val for 08737
    "VUID-VkShaderModuleCreateInfo-pCode-08736", "VUID-VkShaderCreateInfoEXT-pCode-08736",
    "VUID-VkShaderModuleCreateInfo-pCode-08738", "VUID-VkShaderCreateInfoEXT-pCode-08738",

    // These are checked already in VUID-vkGetPrivateData-objectType-04018 and VUID-vkSetPrivateData-objectHandle-04016
    "VUID-vkGetPrivateData-device-parameter",
    "VUID-vkSetPrivateData-device-parameter",

    // These ask if pData is a certain size, but no way to validate a pointer to memory is a certain size.
    // There is already another implicit VU checking if pData is not null.
    "VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-pData-08073",
    "VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-pData-08077",
    "VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-pData-08081",
    "VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-pData-08085",
    "VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-pData-08089",

    // These would need to be checked by the loader as it uses these to call into the layers/drivers
    "VUID-vkEnumerateInstanceVersion-pApiVersion-parameter",
    "VUID-vkEnumerateDeviceExtensionProperties-pPropertyCount-parameter",
    "VUID-vkEnumerateDeviceLayerProperties-pPropertyCount-parameter",
    "VUID-vkEnumerateInstanceLayerProperties-pPropertyCount-parameter",
    "VUID-vkEnumerateInstanceExtensionProperties-pPropertyCount-parameter",

    // Caches are called between application runs so there is no way for a layer to track this information
    "VUID-VkPipelineCacheCreateInfo-initialDataSize-00768",
    "VUID-VkPipelineCacheCreateInfo-initialDataSize-00769",
    "VUID-VkValidationCacheCreateInfoEXT-initialDataSize-01534",
    "VUID-VkValidationCacheCreateInfoEXT-initialDataSize-01535",
    // The header data returned from vkGetPipelineCacheData is the driver's responsibility to make correct.
    // There is CTS for this, and not within the scope of the Validation Layers to check
    "VUID-VkPipelineCacheHeaderVersionOne-headerSize-04967",
    "VUID-VkPipelineCacheHeaderVersionOne-headerVersion-04968",
    "VUID-VkPipelineCacheHeaderVersionOne-headerSize-08990",

    // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6639#note_468463
    "VUID-VkIndirectCommandsVertexBufferTokenEXT-vertexBindingUnit-11134",

    // These implicit VUs ask to check for a valid structure that has no sType,
    // there is nothing that can actually be validated
    //
    // VkImageSubresourceLayers
    "VUID-VkImageBlit-dstSubresource-parameter",
    "VUID-VkImageBlit-srcSubresource-parameter",
    "VUID-VkImageBlit2-dstSubresource-parameter",
    "VUID-VkImageBlit2-srcSubresource-parameter",
    "VUID-VkImageCopy-dstSubresource-parameter",
    "VUID-VkImageCopy-srcSubresource-parameter",
    "VUID-VkImageCopy2-dstSubresource-parameter",
    "VUID-VkImageCopy2-srcSubresource-parameter",
    "VUID-VkImageResolve-dstSubresource-parameter",
    "VUID-VkImageResolve-srcSubresource-parameter",
    "VUID-VkImageResolve2-dstSubresource-parameter",
    "VUID-VkImageResolve2-srcSubresource-parameter",
    "VUID-VkBufferImageCopy-imageSubresource-parameter",
    "VUID-VkBufferImageCopy2-imageSubresource-parameter",
    "VUID-VkMemoryToImageCopyEXT-imageSubresource-parameter",
    "VUID-VkImageToMemoryCopyEXT-imageSubresource-parameter",
    "VUID-VkCopyMemoryToImageIndirectCommandNV-imageSubresource-parameter",
    // VkImageSubresourceRange
    "VUID-VkImageMemoryBarrier-subresourceRange-parameter",
    "VUID-VkImageMemoryBarrier2-subresourceRange-parameter",
    "VUID-VkHostImageLayoutTransitionInfoEXT-subresourceRange-parameter",
    "VUID-VkImageViewCreateInfo-subresourceRange-parameter",
    // VkImageSubresource
    "VUID-VkImageSubresource2KHR-imageSubresource-parameter",
    "VUID-VkSparseImageMemoryBind-subresource-parameter",
    // VkStencilOpState
    "VUID-VkPipelineDepthStencilStateCreateInfo-front-parameter",
    "VUID-VkPipelineDepthStencilStateCreateInfo-back-parameter",
    // VkClearValue
    "VUID-VkRenderingAttachmentInfo-clearValue-parameter",
    // VkComponentMapping
    "VUID-VkImageViewCreateInfo-components-parameter",
    "VUID-VkSamplerYcbcrConversionCreateInfo-components-parameter",
    "VUID-VkSamplerBorderColorComponentMappingCreateInfoEXT-components-parameter",
    // VkAttachmentReference
    "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-parameter",
    // VkVideoEncodeH264QpKHR and VkVideoEncodeH264FrameSizeKHR
    "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-maxFrameSize-parameter",
    "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-maxQp-parameter",
    "VUID-VkVideoEncodeH264RateControlLayerInfoKHR-minQp-parameter",
    // VkVideoEncodeH265QpKHR and VkVideoEncodeH265FrameSizeKHR
    "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-maxFrameSize-parameter",
    "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-maxQp-parameter",
    "VUID-VkVideoEncodeH265RateControlLayerInfoKHR-minQp-parameter",
    // VkVideoPictureResourceInfoKHR
    "VUID-VkVideoDecodeInfoKHR-dstPictureResource-parameter",
    "VUID-VkVideoEncodeInfoKHR-srcPictureResource-parameter",

    // When:
    //   Struct A has a pointer field to Struct B
    //   Struct B has a non-pointer field to Struct C
    // you get a situation where Struct B has a VU that is not hit because we validate it in Struct C
    "VUID-VkAttachmentSampleLocationsEXT-sampleLocationsInfo-parameter",              // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkSubpassSampleLocationsEXT-sampleLocationsInfo-parameter",                 // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkPipelineSampleLocationsStateCreateInfoEXT-sampleLocationsInfo-parameter", // VUID-VkSampleLocationsInfoEXT-sType-sType
    "VUID-VkComputePipelineCreateInfo-stage-parameter", // VUID-VkPipelineShaderStageCreateInfo-sType-sType

    // Not possible as described in https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6324
    "VUID-VkGraphicsPipelineCreateInfo-pTessellationState-09023",
    "VUID-VkGraphicsPipelineCreateInfo-pViewportState-09025",
    "VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-09027",
    "VUID-VkGraphicsPipelineCreateInfo-pDepthStencilState-09029",
    "VUID-VkGraphicsPipelineCreateInfo-pInputAssemblyState-09032",
    "VUID-VkGraphicsPipelineCreateInfo-pDepthStencilState-09034",
    "VUID-VkGraphicsPipelineCreateInfo-pDepthStencilState-09036",
    "VUID-VkGraphicsPipelineCreateInfo-pColorBlendState-09038",
    "VUID-VkGraphicsPipelineCreateInfo-pRasterizationState-09039",
    "VUID-VkGraphicsPipelineCreateInfo-pRasterizationState-09040",
    // another variation of it
    "VUID-vkGetDeviceFaultInfoEXT-pFaultCounts-07337",
    "VUID-vkGetDeviceFaultInfoEXT-pFaultCounts-07338",
    "VUID-vkGetDeviceFaultInfoEXT-pFaultCounts-07339",
    "VUID-VkRenderingInputAttachmentIndexInfoKHR-pDepthInputAttachmentIndex-parameter",
    "VUID-VkRenderingInputAttachmentIndexInfoKHR-pStencilInputAttachmentIndex-parameter"

    // These VUs have "is not NULL it must be a pointer to a valid pointer to valid structure" language
    // There is no actual way to validate thsese
    // https://gitlab.khronos.org/vulkan/vulkan/-/issues/3718
    "VUID-VkDescriptorGetInfoEXT-pUniformTexelBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageTexelBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pUniformBuffer-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageBuffer-parameter",
    // These occur in stateless validation when a pointer member is optional and the length member is also optional
    "VUID-VkPipelineColorBlendStateCreateInfo-pAttachments-parameter",
    "VUID-VkSubpassDescription-pResolveAttachments-parameter",
    "VUID-VkTimelineSemaphoreSubmitInfo-pWaitSemaphoreValues-parameter",
    "VUID-VkTimelineSemaphoreSubmitInfo-pSignalSemaphoreValues-parameter",
    "VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-pStdSPSs-parameter",
    "VUID-VkVideoEncodeH264SessionParametersAddInfoKHR-pStdPPSs-parameter",
    "VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-pStdVPSs-parameter",
    "VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-pStdSPSs-parameter",
    "VUID-VkVideoEncodeH265SessionParametersAddInfoKHR-pStdPPSs-parameter",
    "VUID-VkD3D12FenceSubmitInfoKHR-pWaitSemaphoreValues-parameter",
    "VUID-VkD3D12FenceSubmitInfoKHR-pSignalSemaphoreValues-parameter",
    "VUID-VkPresentRegionKHR-pRectangles-parameter",
    "VUID-VkBindDescriptorSetsInfoKHR-pDynamicOffsets-parameter",
    "VUID-VkPhysicalDeviceHostImageCopyPropertiesEXT-pCopySrcLayouts-parameter",
    "VUID-VkPhysicalDeviceHostImageCopyPropertiesEXT-pCopyDstLayouts-parameter",
    "VUID-VkSurfacePresentModeCompatibilityEXT-pPresentModes-parameter",
    "VUID-VkFrameBoundaryEXT-pImages-parameter",
    "VUID-VkFrameBoundaryEXT-pBuffers-parameter",
    "VUID-VkFrameBoundaryEXT-pTag-parameter",
    "VUID-VkMicromapBuildInfoEXT-pUsageCounts-parameter",
    "VUID-VkMicromapBuildInfoEXT-ppUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesOpacityMicromapEXT-pUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesOpacityMicromapEXT-ppUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesDisplacementMicromapNV-pUsageCounts-parameter",
    "VUID-VkAccelerationStructureTrianglesDisplacementMicromapNV-ppUsageCounts-parameter",
    "VUID-VkShaderCreateInfoEXT-pSetLayouts-parameter",
    "VUID-VkShaderCreateInfoEXT-pPushConstantRanges-parameter",
    "VUID-VkLatencySurfaceCapabilitiesNV-pPresentModes-parameter",
    "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-parameter",
    "VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-parameter",
    "VUID-vkCmdBindVertexBuffers2-pSizes-parameter",
    "VUID-vkCmdBindVertexBuffers2-pStrides-parameter",
    "VUID-VkDescriptorGetInfoEXT-pSampledImage-parameter",
    "VUID-VkDescriptorGetInfoEXT-pSampler-parameter",
    "VUID-VkDescriptorGetInfoEXT-pStorageImage-parameter",
    "VUID-vkGetAccelerationStructureBuildSizesKHR-pMaxPrimitiveCounts-parameter",
    "VUID-vkCmdDrawMultiIndexedEXT-pVertexOffset-parameter",
    "VUID-VkDisplayModeCreateInfoKHR-parameters-parameter",
    // These occur in stateless validation when a pointer member is optional and the length member is null
    "VUID-VkDeviceCreateInfo-pEnabledFeatures-parameter",
    "VUID-VkPipelineShaderStageCreateInfo-pSpecializationInfo-parameter",
    "VUID-VkSubpassDescription-pDepthStencilAttachment-parameter",
    "VUID-VkShaderCreateInfoEXT-pSpecializationInfo-parameter",
    "VUID-VkExportFenceWin32HandleInfoKHR-pAttributes-parameter",
    "VUID-VkExportSemaphoreWin32HandleInfoKHR-pAttributes-parameter",
    "VUID-VkExportMemoryWin32HandleInfoKHR-pAttributes-parameter",
    "VUID-VkExportMemoryWin32HandleInfoNV-pAttributes-parameter",
    "VUID-vkEnumerateDeviceExtensionProperties-pProperties-parameter",
    "VUID-vkEnumerateDeviceLayerProperties-pProperties-parameter",
    "VUID-vkEnumerateInstanceExtensionProperties-pProperties-parameter",
    "VUID-vkEnumerateInstanceLayerProperties-pProperties-parameter",
    // Checking for null-terminated UTF-8 string
    "VUID-VkApplicationInfo-pApplicationName-parameter",
    "VUID-VkApplicationInfo-pEngineName-parameter",
    "VUID-VkDebugUtilsObjectNameInfoEXT-pObjectName-parameter",
    "VUID-VkDebugUtilsMessengerCallbackDataEXT-pMessageIdName-parameter",
    "VUID-VkPipelineShaderStageNodeCreateInfoAMDX-pName-parameter",
    "VUID-VkShaderCreateInfoEXT-pName-parameter",
    "VUID-vkGetDeviceProcAddr-pName-parameter",
    "VUID-vkGetInstanceProcAddr-pName-parameter",
    "VUID-vkEnumerateDeviceExtensionProperties-pLayerName-parameter",
    "VUID-vkEnumerateInstanceExtensionProperties-pLayerName-parameter",
    // Can't validate a VkAllocationCallbacks structure
    "VUID-vkCreateAccelerationStructureKHR-pAllocator-parameter",
    "VUID-vkCreateAccelerationStructureNV-pAllocator-parameter",
    "VUID-vkCreateAndroidSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateBuffer-pAllocator-parameter",
    "VUID-vkCreateBufferCollectionFUCHSIA-pAllocator-parameter",
    "VUID-vkCreateBufferView-pAllocator-parameter",
    "VUID-vkCreateCommandPool-pAllocator-parameter",
    "VUID-vkCreateComputePipelines-pAllocator-parameter",
    "VUID-vkCreateCuFunctionNVX-pAllocator-parameter",
    "VUID-vkCreateCuModuleNVX-pAllocator-parameter",
    "VUID-vkCreateCudaFunctionNV-pAllocator-parameter",
    "VUID-vkCreateCudaModuleNV-pAllocator-parameter",
    "VUID-vkCreateDebugReportCallbackEXT-pAllocator-parameter",
    "VUID-vkCreateDebugUtilsMessengerEXT-pAllocator-parameter",
    "VUID-vkCreateDeferredOperationKHR-pAllocator-parameter",
    "VUID-vkCreateDescriptorPool-pAllocator-parameter",
    "VUID-vkCreateDescriptorSetLayout-pAllocator-parameter",
    "VUID-vkCreateDescriptorUpdateTemplate-pAllocator-parameter",
    "VUID-vkCreateDevice-pAllocator-parameter",
    "VUID-vkCreateDirectFBSurfaceEXT-pAllocator-parameter",
    "VUID-vkCreateDisplayModeKHR-pAllocator-parameter",
    "VUID-vkCreateDisplayPlaneSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateEvent-pAllocator-parameter",
    "VUID-vkCreateExecutionGraphPipelinesAMDX-pAllocator-parameter",
    "VUID-vkCreateFence-pAllocator-parameter",
    "VUID-vkCreateFramebuffer-pAllocator-parameter",
    "VUID-vkCreateGraphicsPipelines-pAllocator-parameter",
    "VUID-vkCreateHeadlessSurfaceEXT-pAllocator-parameter",
    "VUID-vkCreateIOSSurfaceMVK-pAllocator-parameter",
    "VUID-vkCreateImage-pAllocator-parameter",
    "VUID-vkCreateImagePipeSurfaceFUCHSIA-pAllocator-parameter",
    "VUID-vkCreateImageView-pAllocator-parameter",
    "VUID-vkCreateIndirectCommandsLayoutNV-pAllocator-parameter",
    "VUID-vkCreateInstance-pAllocator-parameter",
    "VUID-vkCreateMacOSSurfaceMVK-pAllocator-parameter",
    "VUID-vkCreateMetalSurfaceEXT-pAllocator-parameter",
    "VUID-vkCreateMicromapEXT-pAllocator-parameter",
    "VUID-vkCreateOpticalFlowSessionNV-pAllocator-parameter",
    "VUID-vkCreatePipelineCache-pAllocator-parameter",
    "VUID-vkCreatePipelineLayout-pAllocator-parameter",
    "VUID-vkCreatePrivateDataSlot-pAllocator-parameter",
    "VUID-vkCreateQueryPool-pAllocator-parameter",
    "VUID-vkCreateRayTracingPipelinesKHR-pAllocator-parameter",
    "VUID-vkCreateRayTracingPipelinesNV-pAllocator-parameter",
    "VUID-vkCreateRenderPass-pAllocator-parameter",
    "VUID-vkCreateRenderPass2-pAllocator-parameter",
    "VUID-vkCreateSampler-pAllocator-parameter",
    "VUID-vkCreateSamplerYcbcrConversion-pAllocator-parameter",
    "VUID-vkCreateScreenSurfaceQNX-pAllocator-parameter",
    "VUID-vkCreateSemaphore-pAllocator-parameter",
    "VUID-vkCreateShaderModule-pAllocator-parameter",
    "VUID-vkCreateShadersEXT-pAllocator-parameter",
    "VUID-vkCreateSharedSwapchainsKHR-pAllocator-parameter",
    "VUID-vkCreateStreamDescriptorSurfaceGGP-pAllocator-parameter",
    "VUID-vkCreateSwapchainKHR-pAllocator-parameter",
    "VUID-vkCreateValidationCacheEXT-pAllocator-parameter",
    "VUID-vkCreateViSurfaceNN-pAllocator-parameter",
    "VUID-vkCreateVideoSessionKHR-pAllocator-parameter",
    "VUID-vkCreateVideoSessionParametersKHR-pAllocator-parameter",
    "VUID-vkCreateWaylandSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateWin32SurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateXcbSurfaceKHR-pAllocator-parameter",
    "VUID-vkCreateXlibSurfaceKHR-pAllocator-parameter",
    "VUID-vkDestroyAccelerationStructureKHR-pAllocator-parameter",
    "VUID-vkDestroyAccelerationStructureNV-pAllocator-parameter",
    "VUID-vkDestroyBuffer-pAllocator-parameter",
    "VUID-vkDestroyBufferCollectionFUCHSIA-pAllocator-parameter",
    "VUID-vkDestroyBufferView-pAllocator-parameter",
    "VUID-vkDestroyCommandPool-pAllocator-parameter",
    "VUID-vkDestroyCuFunctionNVX-pAllocator-parameter",
    "VUID-vkDestroyCuModuleNVX-pAllocator-parameter",
    "VUID-vkDestroyCudaFunctionNV-pAllocator-parameter",
    "VUID-vkDestroyCudaModuleNV-pAllocator-parameter",
    "VUID-vkDestroyDebugReportCallbackEXT-pAllocator-parameter",
    "VUID-vkDestroyDebugUtilsMessengerEXT-pAllocator-parameter",
    "VUID-vkDestroyDeferredOperationKHR-pAllocator-parameter",
    "VUID-vkDestroyDescriptorPool-pAllocator-parameter",
    "VUID-vkDestroyDescriptorSetLayout-pAllocator-parameter",
    "VUID-vkDestroyDescriptorUpdateTemplate-pAllocator-parameter",
    "VUID-vkDestroyDevice-pAllocator-parameter",
    "VUID-vkDestroyEvent-pAllocator-parameter",
    "VUID-vkDestroyFence-pAllocator-parameter",
    "VUID-vkDestroyFramebuffer-pAllocator-parameter",
    "VUID-vkDestroyImage-pAllocator-parameter",
    "VUID-vkDestroyImageView-pAllocator-parameter",
    "VUID-vkDestroyIndirectCommandsLayoutNV-pAllocator-parameter",
    "VUID-vkDestroyInstance-pAllocator-parameter",
    "VUID-vkDestroyMicromapEXT-pAllocator-parameter",
    "VUID-vkDestroyOpticalFlowSessionNV-pAllocator-parameter",
    "VUID-vkDestroyPipeline-pAllocator-parameter",
    "VUID-vkDestroyPipelineCache-pAllocator-parameter",
    "VUID-vkDestroyPipelineLayout-pAllocator-parameter",
    "VUID-vkDestroyPrivateDataSlot-pAllocator-parameter",
    "VUID-vkDestroyQueryPool-pAllocator-parameter",
    "VUID-vkDestroyRenderPass-pAllocator-parameter",
    "VUID-vkDestroySampler-pAllocator-parameter",
    "VUID-vkDestroySamplerYcbcrConversion-pAllocator-parameter",
    "VUID-vkDestroySemaphore-pAllocator-parameter",
    "VUID-vkDestroyShaderEXT-pAllocator-parameter",
    "VUID-vkDestroyShaderModule-pAllocator-parameter",
    "VUID-vkDestroySurfaceKHR-pAllocator-parameter",
    "VUID-vkDestroySwapchainKHR-pAllocator-parameter",
    "VUID-vkDestroyValidationCacheEXT-pAllocator-parameter",
    "VUID-vkDestroyVideoSessionKHR-pAllocator-parameter",
    "VUID-vkDestroyVideoSessionParametersKHR-pAllocator-parameter",
    "VUID-vkFreeMemory-pAllocator-parameter",
    "VUID-vkRegisterDeviceEventEXT-pAllocator-parameter",
    "VUID-vkRegisterDisplayEventEXT-pAllocator-parameter",
    "VUID-vkAllocateMemory-pAllocator-parameter",
};

// clang-format on