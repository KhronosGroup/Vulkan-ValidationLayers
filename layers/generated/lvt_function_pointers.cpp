// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See lvt_file_generator.py for modifications

/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
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
 */


#include "lvt_function_pointers.h"
#include <vulkan/vulkan.hpp>

namespace vk {

PFN_vkCreateInstance CreateInstance;
PFN_vkDestroyInstance DestroyInstance;
PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceFeatures GetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceImageFormatProperties GetPhysicalDeviceImageFormatProperties;
PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties GetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
PFN_vkCreateDevice CreateDevice;
PFN_vkDestroyDevice DestroyDevice;
PFN_vkEnumerateInstanceExtensionProperties EnumerateInstanceExtensionProperties;
PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties EnumerateInstanceLayerProperties;
PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties;
PFN_vkGetDeviceQueue GetDeviceQueue;
PFN_vkQueueSubmit QueueSubmit;
PFN_vkQueueWaitIdle QueueWaitIdle;
PFN_vkDeviceWaitIdle DeviceWaitIdle;
PFN_vkAllocateMemory AllocateMemory;
PFN_vkFreeMemory FreeMemory;
PFN_vkMapMemory MapMemory;
PFN_vkUnmapMemory UnmapMemory;
PFN_vkFlushMappedMemoryRanges FlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges;
PFN_vkGetDeviceMemoryCommitment GetDeviceMemoryCommitment;
PFN_vkBindBufferMemory BindBufferMemory;
PFN_vkBindImageMemory BindImageMemory;
PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements;
PFN_vkGetImageMemoryRequirements GetImageMemoryRequirements;
PFN_vkGetImageSparseMemoryRequirements GetImageSparseMemoryRequirements;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties GetPhysicalDeviceSparseImageFormatProperties;
PFN_vkQueueBindSparse QueueBindSparse;
PFN_vkCreateFence CreateFence;
PFN_vkDestroyFence DestroyFence;
PFN_vkResetFences ResetFences;
PFN_vkGetFenceStatus GetFenceStatus;
PFN_vkWaitForFences WaitForFences;
PFN_vkCreateSemaphore CreateSemaphore;
PFN_vkDestroySemaphore DestroySemaphore;
PFN_vkCreateEvent CreateEvent;
PFN_vkDestroyEvent DestroyEvent;
PFN_vkGetEventStatus GetEventStatus;
PFN_vkSetEvent SetEvent;
PFN_vkResetEvent ResetEvent;
PFN_vkCreateQueryPool CreateQueryPool;
PFN_vkDestroyQueryPool DestroyQueryPool;
PFN_vkGetQueryPoolResults GetQueryPoolResults;
PFN_vkCreateBuffer CreateBuffer;
PFN_vkDestroyBuffer DestroyBuffer;
PFN_vkCreateBufferView CreateBufferView;
PFN_vkDestroyBufferView DestroyBufferView;
PFN_vkCreateImage CreateImage;
PFN_vkDestroyImage DestroyImage;
PFN_vkGetImageSubresourceLayout GetImageSubresourceLayout;
PFN_vkCreateImageView CreateImageView;
PFN_vkDestroyImageView DestroyImageView;
PFN_vkCreateShaderModule CreateShaderModule;
PFN_vkDestroyShaderModule DestroyShaderModule;
PFN_vkCreatePipelineCache CreatePipelineCache;
PFN_vkDestroyPipelineCache DestroyPipelineCache;
PFN_vkGetPipelineCacheData GetPipelineCacheData;
PFN_vkMergePipelineCaches MergePipelineCaches;
PFN_vkCreateGraphicsPipelines CreateGraphicsPipelines;
PFN_vkCreateComputePipelines CreateComputePipelines;
PFN_vkDestroyPipeline DestroyPipeline;
PFN_vkCreatePipelineLayout CreatePipelineLayout;
PFN_vkDestroyPipelineLayout DestroyPipelineLayout;
PFN_vkCreateSampler CreateSampler;
PFN_vkDestroySampler DestroySampler;
PFN_vkCreateDescriptorSetLayout CreateDescriptorSetLayout;
PFN_vkDestroyDescriptorSetLayout DestroyDescriptorSetLayout;
PFN_vkCreateDescriptorPool CreateDescriptorPool;
PFN_vkDestroyDescriptorPool DestroyDescriptorPool;
PFN_vkResetDescriptorPool ResetDescriptorPool;
PFN_vkAllocateDescriptorSets AllocateDescriptorSets;
PFN_vkFreeDescriptorSets FreeDescriptorSets;
PFN_vkUpdateDescriptorSets UpdateDescriptorSets;
PFN_vkCreateFramebuffer CreateFramebuffer;
PFN_vkDestroyFramebuffer DestroyFramebuffer;
PFN_vkCreateRenderPass CreateRenderPass;
PFN_vkDestroyRenderPass DestroyRenderPass;
PFN_vkGetRenderAreaGranularity GetRenderAreaGranularity;
PFN_vkCreateCommandPool CreateCommandPool;
PFN_vkDestroyCommandPool DestroyCommandPool;
PFN_vkResetCommandPool ResetCommandPool;
PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
PFN_vkFreeCommandBuffers FreeCommandBuffers;
PFN_vkBeginCommandBuffer BeginCommandBuffer;
PFN_vkEndCommandBuffer EndCommandBuffer;
PFN_vkResetCommandBuffer ResetCommandBuffer;
PFN_vkCmdBindPipeline CmdBindPipeline;
PFN_vkCmdSetViewport CmdSetViewport;
PFN_vkCmdSetScissor CmdSetScissor;
PFN_vkCmdSetLineWidth CmdSetLineWidth;
PFN_vkCmdSetDepthBias CmdSetDepthBias;
PFN_vkCmdSetBlendConstants CmdSetBlendConstants;
PFN_vkCmdSetDepthBounds CmdSetDepthBounds;
PFN_vkCmdSetStencilCompareMask CmdSetStencilCompareMask;
PFN_vkCmdSetStencilWriteMask CmdSetStencilWriteMask;
PFN_vkCmdSetStencilReference CmdSetStencilReference;
PFN_vkCmdBindDescriptorSets CmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer CmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers CmdBindVertexBuffers;
PFN_vkCmdDraw CmdDraw;
PFN_vkCmdDrawIndexed CmdDrawIndexed;
PFN_vkCmdDrawIndirect CmdDrawIndirect;
PFN_vkCmdDrawIndexedIndirect CmdDrawIndexedIndirect;
PFN_vkCmdDispatch CmdDispatch;
PFN_vkCmdDispatchIndirect CmdDispatchIndirect;
PFN_vkCmdCopyBuffer CmdCopyBuffer;
PFN_vkCmdCopyImage CmdCopyImage;
PFN_vkCmdBlitImage CmdBlitImage;
PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage;
PFN_vkCmdCopyImageToBuffer CmdCopyImageToBuffer;
PFN_vkCmdUpdateBuffer CmdUpdateBuffer;
PFN_vkCmdFillBuffer CmdFillBuffer;
PFN_vkCmdClearColorImage CmdClearColorImage;
PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage;
PFN_vkCmdClearAttachments CmdClearAttachments;
PFN_vkCmdResolveImage CmdResolveImage;
PFN_vkCmdSetEvent CmdSetEvent;
PFN_vkCmdResetEvent CmdResetEvent;
PFN_vkCmdWaitEvents CmdWaitEvents;
PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
PFN_vkCmdBeginQuery CmdBeginQuery;
PFN_vkCmdEndQuery CmdEndQuery;
PFN_vkCmdResetQueryPool CmdResetQueryPool;
PFN_vkCmdWriteTimestamp CmdWriteTimestamp;
PFN_vkCmdCopyQueryPoolResults CmdCopyQueryPoolResults;
PFN_vkCmdPushConstants CmdPushConstants;
PFN_vkCmdBeginRenderPass CmdBeginRenderPass;
PFN_vkCmdNextSubpass CmdNextSubpass;
PFN_vkCmdEndRenderPass CmdEndRenderPass;
PFN_vkCmdExecuteCommands CmdExecuteCommands;
PFN_vkEnumerateInstanceVersion EnumerateInstanceVersion;
PFN_vkBindBufferMemory2 BindBufferMemory2;
PFN_vkBindImageMemory2 BindImageMemory2;
PFN_vkGetDeviceGroupPeerMemoryFeatures GetDeviceGroupPeerMemoryFeatures;
PFN_vkCmdSetDeviceMask CmdSetDeviceMask;
PFN_vkCmdDispatchBase CmdDispatchBase;
PFN_vkEnumeratePhysicalDeviceGroups EnumeratePhysicalDeviceGroups;
PFN_vkGetImageMemoryRequirements2 GetImageMemoryRequirements2;
PFN_vkGetBufferMemoryRequirements2 GetBufferMemoryRequirements2;
PFN_vkGetImageSparseMemoryRequirements2 GetImageSparseMemoryRequirements2;
PFN_vkGetPhysicalDeviceFeatures2 GetPhysicalDeviceFeatures2;
PFN_vkGetPhysicalDeviceProperties2 GetPhysicalDeviceProperties2;
PFN_vkGetPhysicalDeviceFormatProperties2 GetPhysicalDeviceFormatProperties2;
PFN_vkGetPhysicalDeviceImageFormatProperties2 GetPhysicalDeviceImageFormatProperties2;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 GetPhysicalDeviceQueueFamilyProperties2;
PFN_vkGetPhysicalDeviceMemoryProperties2 GetPhysicalDeviceMemoryProperties2;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 GetPhysicalDeviceSparseImageFormatProperties2;
PFN_vkTrimCommandPool TrimCommandPool;
PFN_vkGetDeviceQueue2 GetDeviceQueue2;
PFN_vkCreateSamplerYcbcrConversion CreateSamplerYcbcrConversion;
PFN_vkDestroySamplerYcbcrConversion DestroySamplerYcbcrConversion;
PFN_vkCreateDescriptorUpdateTemplate CreateDescriptorUpdateTemplate;
PFN_vkDestroyDescriptorUpdateTemplate DestroyDescriptorUpdateTemplate;
PFN_vkUpdateDescriptorSetWithTemplate UpdateDescriptorSetWithTemplate;
PFN_vkGetPhysicalDeviceExternalBufferProperties GetPhysicalDeviceExternalBufferProperties;
PFN_vkGetPhysicalDeviceExternalFenceProperties GetPhysicalDeviceExternalFenceProperties;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties GetPhysicalDeviceExternalSemaphoreProperties;
PFN_vkGetDescriptorSetLayoutSupport GetDescriptorSetLayoutSupport;
PFN_vkCmdDrawIndirectCount CmdDrawIndirectCount;
PFN_vkCmdDrawIndexedIndirectCount CmdDrawIndexedIndirectCount;
PFN_vkCreateRenderPass2 CreateRenderPass2;
PFN_vkCmdBeginRenderPass2 CmdBeginRenderPass2;
PFN_vkCmdNextSubpass2 CmdNextSubpass2;
PFN_vkCmdEndRenderPass2 CmdEndRenderPass2;
PFN_vkResetQueryPool ResetQueryPool;
PFN_vkGetSemaphoreCounterValue GetSemaphoreCounterValue;
PFN_vkWaitSemaphores WaitSemaphores;
PFN_vkSignalSemaphore SignalSemaphore;
PFN_vkGetBufferDeviceAddress GetBufferDeviceAddress;
PFN_vkGetBufferOpaqueCaptureAddress GetBufferOpaqueCaptureAddress;
PFN_vkGetDeviceMemoryOpaqueCaptureAddress GetDeviceMemoryOpaqueCaptureAddress;
PFN_vkGetPhysicalDeviceToolProperties GetPhysicalDeviceToolProperties;
PFN_vkCreatePrivateDataSlot CreatePrivateDataSlot;
PFN_vkDestroyPrivateDataSlot DestroyPrivateDataSlot;
PFN_vkSetPrivateData SetPrivateData;
PFN_vkGetPrivateData GetPrivateData;
PFN_vkCmdSetEvent2 CmdSetEvent2;
PFN_vkCmdResetEvent2 CmdResetEvent2;
PFN_vkCmdWaitEvents2 CmdWaitEvents2;
PFN_vkCmdPipelineBarrier2 CmdPipelineBarrier2;
PFN_vkCmdWriteTimestamp2 CmdWriteTimestamp2;
PFN_vkQueueSubmit2 QueueSubmit2;
PFN_vkCmdCopyBuffer2 CmdCopyBuffer2;
PFN_vkCmdCopyImage2 CmdCopyImage2;
PFN_vkCmdCopyBufferToImage2 CmdCopyBufferToImage2;
PFN_vkCmdCopyImageToBuffer2 CmdCopyImageToBuffer2;
PFN_vkCmdBlitImage2 CmdBlitImage2;
PFN_vkCmdResolveImage2 CmdResolveImage2;
PFN_vkCmdBeginRendering CmdBeginRendering;
PFN_vkCmdEndRendering CmdEndRendering;
PFN_vkCmdSetCullMode CmdSetCullMode;
PFN_vkCmdSetFrontFace CmdSetFrontFace;
PFN_vkCmdSetPrimitiveTopology CmdSetPrimitiveTopology;
PFN_vkCmdSetViewportWithCount CmdSetViewportWithCount;
PFN_vkCmdSetScissorWithCount CmdSetScissorWithCount;
PFN_vkCmdBindVertexBuffers2 CmdBindVertexBuffers2;
PFN_vkCmdSetDepthTestEnable CmdSetDepthTestEnable;
PFN_vkCmdSetDepthWriteEnable CmdSetDepthWriteEnable;
PFN_vkCmdSetDepthCompareOp CmdSetDepthCompareOp;
PFN_vkCmdSetDepthBoundsTestEnable CmdSetDepthBoundsTestEnable;
PFN_vkCmdSetStencilTestEnable CmdSetStencilTestEnable;
PFN_vkCmdSetStencilOp CmdSetStencilOp;
PFN_vkCmdSetRasterizerDiscardEnable CmdSetRasterizerDiscardEnable;
PFN_vkCmdSetDepthBiasEnable CmdSetDepthBiasEnable;
PFN_vkCmdSetPrimitiveRestartEnable CmdSetPrimitiveRestartEnable;
PFN_vkGetDeviceBufferMemoryRequirements GetDeviceBufferMemoryRequirements;
PFN_vkGetDeviceImageMemoryRequirements GetDeviceImageMemoryRequirements;
PFN_vkGetDeviceImageSparseMemoryRequirements GetDeviceImageSparseMemoryRequirements;
PFN_vkDestroySurfaceKHR DestroySurfaceKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
PFN_vkAcquireNextImageKHR AcquireNextImageKHR;
PFN_vkQueuePresentKHR QueuePresentKHR;
PFN_vkGetDeviceGroupPresentCapabilitiesKHR GetDeviceGroupPresentCapabilitiesKHR;
PFN_vkGetDeviceGroupSurfacePresentModesKHR GetDeviceGroupSurfacePresentModesKHR;
PFN_vkGetPhysicalDevicePresentRectanglesKHR GetPhysicalDevicePresentRectanglesKHR;
PFN_vkAcquireNextImage2KHR AcquireNextImage2KHR;
PFN_vkGetPhysicalDeviceDisplayPropertiesKHR GetPhysicalDeviceDisplayPropertiesKHR;
PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR GetPhysicalDeviceDisplayPlanePropertiesKHR;
PFN_vkGetDisplayPlaneSupportedDisplaysKHR GetDisplayPlaneSupportedDisplaysKHR;
PFN_vkGetDisplayModePropertiesKHR GetDisplayModePropertiesKHR;
PFN_vkCreateDisplayModeKHR CreateDisplayModeKHR;
PFN_vkGetDisplayPlaneCapabilitiesKHR GetDisplayPlaneCapabilitiesKHR;
PFN_vkCreateDisplayPlaneSurfaceKHR CreateDisplayPlaneSurfaceKHR;
#ifdef VK_USE_PLATFORM_XLIB_KHR
PFN_vkCreateXlibSurfaceKHR CreateXlibSurfaceKHR;
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR GetPhysicalDeviceXlibPresentationSupportKHR;
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
PFN_vkCreateXcbSurfaceKHR CreateXcbSurfaceKHR;
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR GetPhysicalDeviceXcbPresentationSupportKHR;
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
PFN_vkCreateWaylandSurfaceKHR CreateWaylandSurfaceKHR;
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
PFN_vkCreateAndroidSurfaceKHR CreateAndroidSurfaceKHR;
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
PFN_vkCreateWin32SurfaceKHR CreateWin32SurfaceKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR GetPhysicalDeviceWin32PresentationSupportKHR;
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_MACOS_MVK
PFN_vkCreateMacOSSurfaceMVK CreateMacOSSurfaceMVK;
#endif // VK_USE_PLATFORM_MACOS_MVK


void InitDispatchTable() {
    static DynamicLoader loader;
    CreateInstance = loader.getProcAddress<PFN_vkCreateInstance>("vkCreateInstance");
    DestroyInstance = loader.getProcAddress<PFN_vkDestroyInstance>("vkDestroyInstance");
    EnumeratePhysicalDevices = loader.getProcAddress<PFN_vkEnumeratePhysicalDevices>("vkEnumeratePhysicalDevices");
    GetPhysicalDeviceFeatures = loader.getProcAddress<PFN_vkGetPhysicalDeviceFeatures>("vkGetPhysicalDeviceFeatures");
    GetPhysicalDeviceFormatProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceFormatProperties>("vkGetPhysicalDeviceFormatProperties");
    GetPhysicalDeviceImageFormatProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceImageFormatProperties>("vkGetPhysicalDeviceImageFormatProperties");
    GetPhysicalDeviceProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceProperties>("vkGetPhysicalDeviceProperties");
    GetPhysicalDeviceQueueFamilyProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceQueueFamilyProperties>("vkGetPhysicalDeviceQueueFamilyProperties");
    GetPhysicalDeviceMemoryProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceMemoryProperties>("vkGetPhysicalDeviceMemoryProperties");
    GetInstanceProcAddr = loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    GetDeviceProcAddr = loader.getProcAddress<PFN_vkGetDeviceProcAddr>("vkGetDeviceProcAddr");
    CreateDevice = loader.getProcAddress<PFN_vkCreateDevice>("vkCreateDevice");
    DestroyDevice = loader.getProcAddress<PFN_vkDestroyDevice>("vkDestroyDevice");
    EnumerateInstanceExtensionProperties = loader.getProcAddress<PFN_vkEnumerateInstanceExtensionProperties>("vkEnumerateInstanceExtensionProperties");
    EnumerateDeviceExtensionProperties = loader.getProcAddress<PFN_vkEnumerateDeviceExtensionProperties>("vkEnumerateDeviceExtensionProperties");
    EnumerateInstanceLayerProperties = loader.getProcAddress<PFN_vkEnumerateInstanceLayerProperties>("vkEnumerateInstanceLayerProperties");
    EnumerateDeviceLayerProperties = loader.getProcAddress<PFN_vkEnumerateDeviceLayerProperties>("vkEnumerateDeviceLayerProperties");
    GetDeviceQueue = loader.getProcAddress<PFN_vkGetDeviceQueue>("vkGetDeviceQueue");
    QueueSubmit = loader.getProcAddress<PFN_vkQueueSubmit>("vkQueueSubmit");
    QueueWaitIdle = loader.getProcAddress<PFN_vkQueueWaitIdle>("vkQueueWaitIdle");
    DeviceWaitIdle = loader.getProcAddress<PFN_vkDeviceWaitIdle>("vkDeviceWaitIdle");
    AllocateMemory = loader.getProcAddress<PFN_vkAllocateMemory>("vkAllocateMemory");
    FreeMemory = loader.getProcAddress<PFN_vkFreeMemory>("vkFreeMemory");
    MapMemory = loader.getProcAddress<PFN_vkMapMemory>("vkMapMemory");
    UnmapMemory = loader.getProcAddress<PFN_vkUnmapMemory>("vkUnmapMemory");
    FlushMappedMemoryRanges = loader.getProcAddress<PFN_vkFlushMappedMemoryRanges>("vkFlushMappedMemoryRanges");
    InvalidateMappedMemoryRanges = loader.getProcAddress<PFN_vkInvalidateMappedMemoryRanges>("vkInvalidateMappedMemoryRanges");
    GetDeviceMemoryCommitment = loader.getProcAddress<PFN_vkGetDeviceMemoryCommitment>("vkGetDeviceMemoryCommitment");
    BindBufferMemory = loader.getProcAddress<PFN_vkBindBufferMemory>("vkBindBufferMemory");
    BindImageMemory = loader.getProcAddress<PFN_vkBindImageMemory>("vkBindImageMemory");
    GetBufferMemoryRequirements = loader.getProcAddress<PFN_vkGetBufferMemoryRequirements>("vkGetBufferMemoryRequirements");
    GetImageMemoryRequirements = loader.getProcAddress<PFN_vkGetImageMemoryRequirements>("vkGetImageMemoryRequirements");
    GetImageSparseMemoryRequirements = loader.getProcAddress<PFN_vkGetImageSparseMemoryRequirements>("vkGetImageSparseMemoryRequirements");
    GetPhysicalDeviceSparseImageFormatProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>("vkGetPhysicalDeviceSparseImageFormatProperties");
    QueueBindSparse = loader.getProcAddress<PFN_vkQueueBindSparse>("vkQueueBindSparse");
    CreateFence = loader.getProcAddress<PFN_vkCreateFence>("vkCreateFence");
    DestroyFence = loader.getProcAddress<PFN_vkDestroyFence>("vkDestroyFence");
    ResetFences = loader.getProcAddress<PFN_vkResetFences>("vkResetFences");
    GetFenceStatus = loader.getProcAddress<PFN_vkGetFenceStatus>("vkGetFenceStatus");
    WaitForFences = loader.getProcAddress<PFN_vkWaitForFences>("vkWaitForFences");
    CreateSemaphore = loader.getProcAddress<PFN_vkCreateSemaphore>("vkCreateSemaphore");
    DestroySemaphore = loader.getProcAddress<PFN_vkDestroySemaphore>("vkDestroySemaphore");
    CreateEvent = loader.getProcAddress<PFN_vkCreateEvent>("vkCreateEvent");
    DestroyEvent = loader.getProcAddress<PFN_vkDestroyEvent>("vkDestroyEvent");
    GetEventStatus = loader.getProcAddress<PFN_vkGetEventStatus>("vkGetEventStatus");
    SetEvent = loader.getProcAddress<PFN_vkSetEvent>("vkSetEvent");
    ResetEvent = loader.getProcAddress<PFN_vkResetEvent>("vkResetEvent");
    CreateQueryPool = loader.getProcAddress<PFN_vkCreateQueryPool>("vkCreateQueryPool");
    DestroyQueryPool = loader.getProcAddress<PFN_vkDestroyQueryPool>("vkDestroyQueryPool");
    GetQueryPoolResults = loader.getProcAddress<PFN_vkGetQueryPoolResults>("vkGetQueryPoolResults");
    CreateBuffer = loader.getProcAddress<PFN_vkCreateBuffer>("vkCreateBuffer");
    DestroyBuffer = loader.getProcAddress<PFN_vkDestroyBuffer>("vkDestroyBuffer");
    CreateBufferView = loader.getProcAddress<PFN_vkCreateBufferView>("vkCreateBufferView");
    DestroyBufferView = loader.getProcAddress<PFN_vkDestroyBufferView>("vkDestroyBufferView");
    CreateImage = loader.getProcAddress<PFN_vkCreateImage>("vkCreateImage");
    DestroyImage = loader.getProcAddress<PFN_vkDestroyImage>("vkDestroyImage");
    GetImageSubresourceLayout = loader.getProcAddress<PFN_vkGetImageSubresourceLayout>("vkGetImageSubresourceLayout");
    CreateImageView = loader.getProcAddress<PFN_vkCreateImageView>("vkCreateImageView");
    DestroyImageView = loader.getProcAddress<PFN_vkDestroyImageView>("vkDestroyImageView");
    CreateShaderModule = loader.getProcAddress<PFN_vkCreateShaderModule>("vkCreateShaderModule");
    DestroyShaderModule = loader.getProcAddress<PFN_vkDestroyShaderModule>("vkDestroyShaderModule");
    CreatePipelineCache = loader.getProcAddress<PFN_vkCreatePipelineCache>("vkCreatePipelineCache");
    DestroyPipelineCache = loader.getProcAddress<PFN_vkDestroyPipelineCache>("vkDestroyPipelineCache");
    GetPipelineCacheData = loader.getProcAddress<PFN_vkGetPipelineCacheData>("vkGetPipelineCacheData");
    MergePipelineCaches = loader.getProcAddress<PFN_vkMergePipelineCaches>("vkMergePipelineCaches");
    CreateGraphicsPipelines = loader.getProcAddress<PFN_vkCreateGraphicsPipelines>("vkCreateGraphicsPipelines");
    CreateComputePipelines = loader.getProcAddress<PFN_vkCreateComputePipelines>("vkCreateComputePipelines");
    DestroyPipeline = loader.getProcAddress<PFN_vkDestroyPipeline>("vkDestroyPipeline");
    CreatePipelineLayout = loader.getProcAddress<PFN_vkCreatePipelineLayout>("vkCreatePipelineLayout");
    DestroyPipelineLayout = loader.getProcAddress<PFN_vkDestroyPipelineLayout>("vkDestroyPipelineLayout");
    CreateSampler = loader.getProcAddress<PFN_vkCreateSampler>("vkCreateSampler");
    DestroySampler = loader.getProcAddress<PFN_vkDestroySampler>("vkDestroySampler");
    CreateDescriptorSetLayout = loader.getProcAddress<PFN_vkCreateDescriptorSetLayout>("vkCreateDescriptorSetLayout");
    DestroyDescriptorSetLayout = loader.getProcAddress<PFN_vkDestroyDescriptorSetLayout>("vkDestroyDescriptorSetLayout");
    CreateDescriptorPool = loader.getProcAddress<PFN_vkCreateDescriptorPool>("vkCreateDescriptorPool");
    DestroyDescriptorPool = loader.getProcAddress<PFN_vkDestroyDescriptorPool>("vkDestroyDescriptorPool");
    ResetDescriptorPool = loader.getProcAddress<PFN_vkResetDescriptorPool>("vkResetDescriptorPool");
    AllocateDescriptorSets = loader.getProcAddress<PFN_vkAllocateDescriptorSets>("vkAllocateDescriptorSets");
    FreeDescriptorSets = loader.getProcAddress<PFN_vkFreeDescriptorSets>("vkFreeDescriptorSets");
    UpdateDescriptorSets = loader.getProcAddress<PFN_vkUpdateDescriptorSets>("vkUpdateDescriptorSets");
    CreateFramebuffer = loader.getProcAddress<PFN_vkCreateFramebuffer>("vkCreateFramebuffer");
    DestroyFramebuffer = loader.getProcAddress<PFN_vkDestroyFramebuffer>("vkDestroyFramebuffer");
    CreateRenderPass = loader.getProcAddress<PFN_vkCreateRenderPass>("vkCreateRenderPass");
    DestroyRenderPass = loader.getProcAddress<PFN_vkDestroyRenderPass>("vkDestroyRenderPass");
    GetRenderAreaGranularity = loader.getProcAddress<PFN_vkGetRenderAreaGranularity>("vkGetRenderAreaGranularity");
    CreateCommandPool = loader.getProcAddress<PFN_vkCreateCommandPool>("vkCreateCommandPool");
    DestroyCommandPool = loader.getProcAddress<PFN_vkDestroyCommandPool>("vkDestroyCommandPool");
    ResetCommandPool = loader.getProcAddress<PFN_vkResetCommandPool>("vkResetCommandPool");
    AllocateCommandBuffers = loader.getProcAddress<PFN_vkAllocateCommandBuffers>("vkAllocateCommandBuffers");
    FreeCommandBuffers = loader.getProcAddress<PFN_vkFreeCommandBuffers>("vkFreeCommandBuffers");
    BeginCommandBuffer = loader.getProcAddress<PFN_vkBeginCommandBuffer>("vkBeginCommandBuffer");
    EndCommandBuffer = loader.getProcAddress<PFN_vkEndCommandBuffer>("vkEndCommandBuffer");
    ResetCommandBuffer = loader.getProcAddress<PFN_vkResetCommandBuffer>("vkResetCommandBuffer");
    CmdBindPipeline = loader.getProcAddress<PFN_vkCmdBindPipeline>("vkCmdBindPipeline");
    CmdSetViewport = loader.getProcAddress<PFN_vkCmdSetViewport>("vkCmdSetViewport");
    CmdSetScissor = loader.getProcAddress<PFN_vkCmdSetScissor>("vkCmdSetScissor");
    CmdSetLineWidth = loader.getProcAddress<PFN_vkCmdSetLineWidth>("vkCmdSetLineWidth");
    CmdSetDepthBias = loader.getProcAddress<PFN_vkCmdSetDepthBias>("vkCmdSetDepthBias");
    CmdSetBlendConstants = loader.getProcAddress<PFN_vkCmdSetBlendConstants>("vkCmdSetBlendConstants");
    CmdSetDepthBounds = loader.getProcAddress<PFN_vkCmdSetDepthBounds>("vkCmdSetDepthBounds");
    CmdSetStencilCompareMask = loader.getProcAddress<PFN_vkCmdSetStencilCompareMask>("vkCmdSetStencilCompareMask");
    CmdSetStencilWriteMask = loader.getProcAddress<PFN_vkCmdSetStencilWriteMask>("vkCmdSetStencilWriteMask");
    CmdSetStencilReference = loader.getProcAddress<PFN_vkCmdSetStencilReference>("vkCmdSetStencilReference");
    CmdBindDescriptorSets = loader.getProcAddress<PFN_vkCmdBindDescriptorSets>("vkCmdBindDescriptorSets");
    CmdBindIndexBuffer = loader.getProcAddress<PFN_vkCmdBindIndexBuffer>("vkCmdBindIndexBuffer");
    CmdBindVertexBuffers = loader.getProcAddress<PFN_vkCmdBindVertexBuffers>("vkCmdBindVertexBuffers");
    CmdDraw = loader.getProcAddress<PFN_vkCmdDraw>("vkCmdDraw");
    CmdDrawIndexed = loader.getProcAddress<PFN_vkCmdDrawIndexed>("vkCmdDrawIndexed");
    CmdDrawIndirect = loader.getProcAddress<PFN_vkCmdDrawIndirect>("vkCmdDrawIndirect");
    CmdDrawIndexedIndirect = loader.getProcAddress<PFN_vkCmdDrawIndexedIndirect>("vkCmdDrawIndexedIndirect");
    CmdDispatch = loader.getProcAddress<PFN_vkCmdDispatch>("vkCmdDispatch");
    CmdDispatchIndirect = loader.getProcAddress<PFN_vkCmdDispatchIndirect>("vkCmdDispatchIndirect");
    CmdCopyBuffer = loader.getProcAddress<PFN_vkCmdCopyBuffer>("vkCmdCopyBuffer");
    CmdCopyImage = loader.getProcAddress<PFN_vkCmdCopyImage>("vkCmdCopyImage");
    CmdBlitImage = loader.getProcAddress<PFN_vkCmdBlitImage>("vkCmdBlitImage");
    CmdCopyBufferToImage = loader.getProcAddress<PFN_vkCmdCopyBufferToImage>("vkCmdCopyBufferToImage");
    CmdCopyImageToBuffer = loader.getProcAddress<PFN_vkCmdCopyImageToBuffer>("vkCmdCopyImageToBuffer");
    CmdUpdateBuffer = loader.getProcAddress<PFN_vkCmdUpdateBuffer>("vkCmdUpdateBuffer");
    CmdFillBuffer = loader.getProcAddress<PFN_vkCmdFillBuffer>("vkCmdFillBuffer");
    CmdClearColorImage = loader.getProcAddress<PFN_vkCmdClearColorImage>("vkCmdClearColorImage");
    CmdClearDepthStencilImage = loader.getProcAddress<PFN_vkCmdClearDepthStencilImage>("vkCmdClearDepthStencilImage");
    CmdClearAttachments = loader.getProcAddress<PFN_vkCmdClearAttachments>("vkCmdClearAttachments");
    CmdResolveImage = loader.getProcAddress<PFN_vkCmdResolveImage>("vkCmdResolveImage");
    CmdSetEvent = loader.getProcAddress<PFN_vkCmdSetEvent>("vkCmdSetEvent");
    CmdResetEvent = loader.getProcAddress<PFN_vkCmdResetEvent>("vkCmdResetEvent");
    CmdWaitEvents = loader.getProcAddress<PFN_vkCmdWaitEvents>("vkCmdWaitEvents");
    CmdPipelineBarrier = loader.getProcAddress<PFN_vkCmdPipelineBarrier>("vkCmdPipelineBarrier");
    CmdBeginQuery = loader.getProcAddress<PFN_vkCmdBeginQuery>("vkCmdBeginQuery");
    CmdEndQuery = loader.getProcAddress<PFN_vkCmdEndQuery>("vkCmdEndQuery");
    CmdResetQueryPool = loader.getProcAddress<PFN_vkCmdResetQueryPool>("vkCmdResetQueryPool");
    CmdWriteTimestamp = loader.getProcAddress<PFN_vkCmdWriteTimestamp>("vkCmdWriteTimestamp");
    CmdCopyQueryPoolResults = loader.getProcAddress<PFN_vkCmdCopyQueryPoolResults>("vkCmdCopyQueryPoolResults");
    CmdPushConstants = loader.getProcAddress<PFN_vkCmdPushConstants>("vkCmdPushConstants");
    CmdBeginRenderPass = loader.getProcAddress<PFN_vkCmdBeginRenderPass>("vkCmdBeginRenderPass");
    CmdNextSubpass = loader.getProcAddress<PFN_vkCmdNextSubpass>("vkCmdNextSubpass");
    CmdEndRenderPass = loader.getProcAddress<PFN_vkCmdEndRenderPass>("vkCmdEndRenderPass");
    CmdExecuteCommands = loader.getProcAddress<PFN_vkCmdExecuteCommands>("vkCmdExecuteCommands");
    EnumerateInstanceVersion = loader.getProcAddress<PFN_vkEnumerateInstanceVersion>("vkEnumerateInstanceVersion");
    BindBufferMemory2 = loader.getProcAddress<PFN_vkBindBufferMemory2>("vkBindBufferMemory2");
    BindImageMemory2 = loader.getProcAddress<PFN_vkBindImageMemory2>("vkBindImageMemory2");
    GetDeviceGroupPeerMemoryFeatures = loader.getProcAddress<PFN_vkGetDeviceGroupPeerMemoryFeatures>("vkGetDeviceGroupPeerMemoryFeatures");
    CmdSetDeviceMask = loader.getProcAddress<PFN_vkCmdSetDeviceMask>("vkCmdSetDeviceMask");
    CmdDispatchBase = loader.getProcAddress<PFN_vkCmdDispatchBase>("vkCmdDispatchBase");
    EnumeratePhysicalDeviceGroups = loader.getProcAddress<PFN_vkEnumeratePhysicalDeviceGroups>("vkEnumeratePhysicalDeviceGroups");
    GetImageMemoryRequirements2 = loader.getProcAddress<PFN_vkGetImageMemoryRequirements2>("vkGetImageMemoryRequirements2");
    GetBufferMemoryRequirements2 = loader.getProcAddress<PFN_vkGetBufferMemoryRequirements2>("vkGetBufferMemoryRequirements2");
    GetImageSparseMemoryRequirements2 = loader.getProcAddress<PFN_vkGetImageSparseMemoryRequirements2>("vkGetImageSparseMemoryRequirements2");
    GetPhysicalDeviceFeatures2 = loader.getProcAddress<PFN_vkGetPhysicalDeviceFeatures2>("vkGetPhysicalDeviceFeatures2");
    GetPhysicalDeviceProperties2 = loader.getProcAddress<PFN_vkGetPhysicalDeviceProperties2>("vkGetPhysicalDeviceProperties2");
    GetPhysicalDeviceFormatProperties2 = loader.getProcAddress<PFN_vkGetPhysicalDeviceFormatProperties2>("vkGetPhysicalDeviceFormatProperties2");
    GetPhysicalDeviceImageFormatProperties2 = loader.getProcAddress<PFN_vkGetPhysicalDeviceImageFormatProperties2>("vkGetPhysicalDeviceImageFormatProperties2");
    GetPhysicalDeviceQueueFamilyProperties2 = loader.getProcAddress<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>("vkGetPhysicalDeviceQueueFamilyProperties2");
    GetPhysicalDeviceMemoryProperties2 = loader.getProcAddress<PFN_vkGetPhysicalDeviceMemoryProperties2>("vkGetPhysicalDeviceMemoryProperties2");
    GetPhysicalDeviceSparseImageFormatProperties2 = loader.getProcAddress<PFN_vkGetPhysicalDeviceSparseImageFormatProperties2>("vkGetPhysicalDeviceSparseImageFormatProperties2");
    TrimCommandPool = loader.getProcAddress<PFN_vkTrimCommandPool>("vkTrimCommandPool");
    GetDeviceQueue2 = loader.getProcAddress<PFN_vkGetDeviceQueue2>("vkGetDeviceQueue2");
    CreateSamplerYcbcrConversion = loader.getProcAddress<PFN_vkCreateSamplerYcbcrConversion>("vkCreateSamplerYcbcrConversion");
    DestroySamplerYcbcrConversion = loader.getProcAddress<PFN_vkDestroySamplerYcbcrConversion>("vkDestroySamplerYcbcrConversion");
    CreateDescriptorUpdateTemplate = loader.getProcAddress<PFN_vkCreateDescriptorUpdateTemplate>("vkCreateDescriptorUpdateTemplate");
    DestroyDescriptorUpdateTemplate = loader.getProcAddress<PFN_vkDestroyDescriptorUpdateTemplate>("vkDestroyDescriptorUpdateTemplate");
    UpdateDescriptorSetWithTemplate = loader.getProcAddress<PFN_vkUpdateDescriptorSetWithTemplate>("vkUpdateDescriptorSetWithTemplate");
    GetPhysicalDeviceExternalBufferProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceExternalBufferProperties>("vkGetPhysicalDeviceExternalBufferProperties");
    GetPhysicalDeviceExternalFenceProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceExternalFenceProperties>("vkGetPhysicalDeviceExternalFenceProperties");
    GetPhysicalDeviceExternalSemaphoreProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceExternalSemaphoreProperties>("vkGetPhysicalDeviceExternalSemaphoreProperties");
    GetDescriptorSetLayoutSupport = loader.getProcAddress<PFN_vkGetDescriptorSetLayoutSupport>("vkGetDescriptorSetLayoutSupport");
    CmdDrawIndirectCount = loader.getProcAddress<PFN_vkCmdDrawIndirectCount>("vkCmdDrawIndirectCount");
    CmdDrawIndexedIndirectCount = loader.getProcAddress<PFN_vkCmdDrawIndexedIndirectCount>("vkCmdDrawIndexedIndirectCount");
    CreateRenderPass2 = loader.getProcAddress<PFN_vkCreateRenderPass2>("vkCreateRenderPass2");
    CmdBeginRenderPass2 = loader.getProcAddress<PFN_vkCmdBeginRenderPass2>("vkCmdBeginRenderPass2");
    CmdNextSubpass2 = loader.getProcAddress<PFN_vkCmdNextSubpass2>("vkCmdNextSubpass2");
    CmdEndRenderPass2 = loader.getProcAddress<PFN_vkCmdEndRenderPass2>("vkCmdEndRenderPass2");
    ResetQueryPool = loader.getProcAddress<PFN_vkResetQueryPool>("vkResetQueryPool");
    GetSemaphoreCounterValue = loader.getProcAddress<PFN_vkGetSemaphoreCounterValue>("vkGetSemaphoreCounterValue");
    WaitSemaphores = loader.getProcAddress<PFN_vkWaitSemaphores>("vkWaitSemaphores");
    SignalSemaphore = loader.getProcAddress<PFN_vkSignalSemaphore>("vkSignalSemaphore");
    GetBufferDeviceAddress = loader.getProcAddress<PFN_vkGetBufferDeviceAddress>("vkGetBufferDeviceAddress");
    GetBufferOpaqueCaptureAddress = loader.getProcAddress<PFN_vkGetBufferOpaqueCaptureAddress>("vkGetBufferOpaqueCaptureAddress");
    GetDeviceMemoryOpaqueCaptureAddress = loader.getProcAddress<PFN_vkGetDeviceMemoryOpaqueCaptureAddress>("vkGetDeviceMemoryOpaqueCaptureAddress");
    GetPhysicalDeviceToolProperties = loader.getProcAddress<PFN_vkGetPhysicalDeviceToolProperties>("vkGetPhysicalDeviceToolProperties");
    CreatePrivateDataSlot = loader.getProcAddress<PFN_vkCreatePrivateDataSlot>("vkCreatePrivateDataSlot");
    DestroyPrivateDataSlot = loader.getProcAddress<PFN_vkDestroyPrivateDataSlot>("vkDestroyPrivateDataSlot");
    SetPrivateData = loader.getProcAddress<PFN_vkSetPrivateData>("vkSetPrivateData");
    GetPrivateData = loader.getProcAddress<PFN_vkGetPrivateData>("vkGetPrivateData");
    CmdSetEvent2 = loader.getProcAddress<PFN_vkCmdSetEvent2>("vkCmdSetEvent2");
    CmdResetEvent2 = loader.getProcAddress<PFN_vkCmdResetEvent2>("vkCmdResetEvent2");
    CmdWaitEvents2 = loader.getProcAddress<PFN_vkCmdWaitEvents2>("vkCmdWaitEvents2");
    CmdPipelineBarrier2 = loader.getProcAddress<PFN_vkCmdPipelineBarrier2>("vkCmdPipelineBarrier2");
    CmdWriteTimestamp2 = loader.getProcAddress<PFN_vkCmdWriteTimestamp2>("vkCmdWriteTimestamp2");
    QueueSubmit2 = loader.getProcAddress<PFN_vkQueueSubmit2>("vkQueueSubmit2");
    CmdCopyBuffer2 = loader.getProcAddress<PFN_vkCmdCopyBuffer2>("vkCmdCopyBuffer2");
    CmdCopyImage2 = loader.getProcAddress<PFN_vkCmdCopyImage2>("vkCmdCopyImage2");
    CmdCopyBufferToImage2 = loader.getProcAddress<PFN_vkCmdCopyBufferToImage2>("vkCmdCopyBufferToImage2");
    CmdCopyImageToBuffer2 = loader.getProcAddress<PFN_vkCmdCopyImageToBuffer2>("vkCmdCopyImageToBuffer2");
    CmdBlitImage2 = loader.getProcAddress<PFN_vkCmdBlitImage2>("vkCmdBlitImage2");
    CmdResolveImage2 = loader.getProcAddress<PFN_vkCmdResolveImage2>("vkCmdResolveImage2");
    CmdBeginRendering = loader.getProcAddress<PFN_vkCmdBeginRendering>("vkCmdBeginRendering");
    CmdEndRendering = loader.getProcAddress<PFN_vkCmdEndRendering>("vkCmdEndRendering");
    CmdSetCullMode = loader.getProcAddress<PFN_vkCmdSetCullMode>("vkCmdSetCullMode");
    CmdSetFrontFace = loader.getProcAddress<PFN_vkCmdSetFrontFace>("vkCmdSetFrontFace");
    CmdSetPrimitiveTopology = loader.getProcAddress<PFN_vkCmdSetPrimitiveTopology>("vkCmdSetPrimitiveTopology");
    CmdSetViewportWithCount = loader.getProcAddress<PFN_vkCmdSetViewportWithCount>("vkCmdSetViewportWithCount");
    CmdSetScissorWithCount = loader.getProcAddress<PFN_vkCmdSetScissorWithCount>("vkCmdSetScissorWithCount");
    CmdBindVertexBuffers2 = loader.getProcAddress<PFN_vkCmdBindVertexBuffers2>("vkCmdBindVertexBuffers2");
    CmdSetDepthTestEnable = loader.getProcAddress<PFN_vkCmdSetDepthTestEnable>("vkCmdSetDepthTestEnable");
    CmdSetDepthWriteEnable = loader.getProcAddress<PFN_vkCmdSetDepthWriteEnable>("vkCmdSetDepthWriteEnable");
    CmdSetDepthCompareOp = loader.getProcAddress<PFN_vkCmdSetDepthCompareOp>("vkCmdSetDepthCompareOp");
    CmdSetDepthBoundsTestEnable = loader.getProcAddress<PFN_vkCmdSetDepthBoundsTestEnable>("vkCmdSetDepthBoundsTestEnable");
    CmdSetStencilTestEnable = loader.getProcAddress<PFN_vkCmdSetStencilTestEnable>("vkCmdSetStencilTestEnable");
    CmdSetStencilOp = loader.getProcAddress<PFN_vkCmdSetStencilOp>("vkCmdSetStencilOp");
    CmdSetRasterizerDiscardEnable = loader.getProcAddress<PFN_vkCmdSetRasterizerDiscardEnable>("vkCmdSetRasterizerDiscardEnable");
    CmdSetDepthBiasEnable = loader.getProcAddress<PFN_vkCmdSetDepthBiasEnable>("vkCmdSetDepthBiasEnable");
    CmdSetPrimitiveRestartEnable = loader.getProcAddress<PFN_vkCmdSetPrimitiveRestartEnable>("vkCmdSetPrimitiveRestartEnable");
    GetDeviceBufferMemoryRequirements = loader.getProcAddress<PFN_vkGetDeviceBufferMemoryRequirements>("vkGetDeviceBufferMemoryRequirements");
    GetDeviceImageMemoryRequirements = loader.getProcAddress<PFN_vkGetDeviceImageMemoryRequirements>("vkGetDeviceImageMemoryRequirements");
    GetDeviceImageSparseMemoryRequirements = loader.getProcAddress<PFN_vkGetDeviceImageSparseMemoryRequirements>("vkGetDeviceImageSparseMemoryRequirements");
    DestroySurfaceKHR = loader.getProcAddress<PFN_vkDestroySurfaceKHR>("vkDestroySurfaceKHR");
    GetPhysicalDeviceSurfaceSupportKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>("vkGetPhysicalDeviceSurfaceSupportKHR");
    GetPhysicalDeviceSurfaceCapabilitiesKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>("vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    GetPhysicalDeviceSurfaceFormatsKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>("vkGetPhysicalDeviceSurfaceFormatsKHR");
    GetPhysicalDeviceSurfacePresentModesKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>("vkGetPhysicalDeviceSurfacePresentModesKHR");
    CreateSwapchainKHR = loader.getProcAddress<PFN_vkCreateSwapchainKHR>("vkCreateSwapchainKHR");
    DestroySwapchainKHR = loader.getProcAddress<PFN_vkDestroySwapchainKHR>("vkDestroySwapchainKHR");
    GetSwapchainImagesKHR = loader.getProcAddress<PFN_vkGetSwapchainImagesKHR>("vkGetSwapchainImagesKHR");
    AcquireNextImageKHR = loader.getProcAddress<PFN_vkAcquireNextImageKHR>("vkAcquireNextImageKHR");
    QueuePresentKHR = loader.getProcAddress<PFN_vkQueuePresentKHR>("vkQueuePresentKHR");
    GetDeviceGroupPresentCapabilitiesKHR = loader.getProcAddress<PFN_vkGetDeviceGroupPresentCapabilitiesKHR>("vkGetDeviceGroupPresentCapabilitiesKHR");
    GetDeviceGroupSurfacePresentModesKHR = loader.getProcAddress<PFN_vkGetDeviceGroupSurfacePresentModesKHR>("vkGetDeviceGroupSurfacePresentModesKHR");
    GetPhysicalDevicePresentRectanglesKHR = loader.getProcAddress<PFN_vkGetPhysicalDevicePresentRectanglesKHR>("vkGetPhysicalDevicePresentRectanglesKHR");
    AcquireNextImage2KHR = loader.getProcAddress<PFN_vkAcquireNextImage2KHR>("vkAcquireNextImage2KHR");
    GetPhysicalDeviceDisplayPropertiesKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>("vkGetPhysicalDeviceDisplayPropertiesKHR");
    GetPhysicalDeviceDisplayPlanePropertiesKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>("vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    GetDisplayPlaneSupportedDisplaysKHR = loader.getProcAddress<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>("vkGetDisplayPlaneSupportedDisplaysKHR");
    GetDisplayModePropertiesKHR = loader.getProcAddress<PFN_vkGetDisplayModePropertiesKHR>("vkGetDisplayModePropertiesKHR");
    CreateDisplayModeKHR = loader.getProcAddress<PFN_vkCreateDisplayModeKHR>("vkCreateDisplayModeKHR");
    GetDisplayPlaneCapabilitiesKHR = loader.getProcAddress<PFN_vkGetDisplayPlaneCapabilitiesKHR>("vkGetDisplayPlaneCapabilitiesKHR");
    CreateDisplayPlaneSurfaceKHR = loader.getProcAddress<PFN_vkCreateDisplayPlaneSurfaceKHR>("vkCreateDisplayPlaneSurfaceKHR");
#ifdef VK_USE_PLATFORM_XLIB_KHR
    CreateXlibSurfaceKHR = loader.getProcAddress<PFN_vkCreateXlibSurfaceKHR>("vkCreateXlibSurfaceKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    GetPhysicalDeviceXlibPresentationSupportKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>("vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    CreateXcbSurfaceKHR = loader.getProcAddress<PFN_vkCreateXcbSurfaceKHR>("vkCreateXcbSurfaceKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    GetPhysicalDeviceXcbPresentationSupportKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>("vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    CreateWaylandSurfaceKHR = loader.getProcAddress<PFN_vkCreateWaylandSurfaceKHR>("vkCreateWaylandSurfaceKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    GetPhysicalDeviceWaylandPresentationSupportKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>("vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    CreateAndroidSurfaceKHR = loader.getProcAddress<PFN_vkCreateAndroidSurfaceKHR>("vkCreateAndroidSurfaceKHR");
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    CreateWin32SurfaceKHR = loader.getProcAddress<PFN_vkCreateWin32SurfaceKHR>("vkCreateWin32SurfaceKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    GetPhysicalDeviceWin32PresentationSupportKHR = loader.getProcAddress<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>("vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_MACOS_MVK
    CreateMacOSSurfaceMVK = loader.getProcAddress<PFN_vkCreateMacOSSurfaceMVK>("vkCreateMacOSSurfaceMVK");
#endif // VK_USE_PLATFORM_MACOS_MVK
}

} // namespace vk
