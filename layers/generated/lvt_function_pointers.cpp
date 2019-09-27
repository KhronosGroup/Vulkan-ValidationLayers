// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See lvt_file_generator.py for modifications

/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 */


#include "lvt_function_pointers.h"
#include <stdio.h>

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

#if(WIN32)
    const char filename[] = "vulkan-1.dll";
#else
    const char filename[] = "libvulkan.so";
#endif

    auto loader_handle = loader_platform_open_library(filename);

    if (loader_handle == nullptr) {
        printf("%s\n", loader_platform_open_library_error(filename));
        exit(1);
    }

    CreateInstance = reinterpret_cast<PFN_vkCreateInstance>(loader_platform_get_proc_address(loader_handle, "vkCreateInstance"));
    DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(loader_platform_get_proc_address(loader_handle, "vkDestroyInstance"));
    EnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(loader_platform_get_proc_address(loader_handle, "vkEnumeratePhysicalDevices"));
    GetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceFeatures"));
    GetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceFormatProperties"));
    GetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceImageFormatProperties"));
    GetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceProperties"));
    GetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceQueueFamilyProperties"));
    GetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceMemoryProperties"));
    GetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(loader_platform_get_proc_address(loader_handle, "vkGetInstanceProcAddr"));
    GetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(loader_platform_get_proc_address(loader_handle, "vkGetDeviceProcAddr"));
    CreateDevice = reinterpret_cast<PFN_vkCreateDevice>(loader_platform_get_proc_address(loader_handle, "vkCreateDevice"));
    DestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(loader_platform_get_proc_address(loader_handle, "vkDestroyDevice"));
    EnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(loader_platform_get_proc_address(loader_handle, "vkEnumerateInstanceExtensionProperties"));
    EnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(loader_platform_get_proc_address(loader_handle, "vkEnumerateDeviceExtensionProperties"));
    EnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(loader_platform_get_proc_address(loader_handle, "vkEnumerateInstanceLayerProperties"));
    EnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(loader_platform_get_proc_address(loader_handle, "vkEnumerateDeviceLayerProperties"));
    GetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(loader_platform_get_proc_address(loader_handle, "vkGetDeviceQueue"));
    QueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(loader_platform_get_proc_address(loader_handle, "vkQueueSubmit"));
    QueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(loader_platform_get_proc_address(loader_handle, "vkQueueWaitIdle"));
    DeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(loader_platform_get_proc_address(loader_handle, "vkDeviceWaitIdle"));
    AllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(loader_platform_get_proc_address(loader_handle, "vkAllocateMemory"));
    FreeMemory = reinterpret_cast<PFN_vkFreeMemory>(loader_platform_get_proc_address(loader_handle, "vkFreeMemory"));
    MapMemory = reinterpret_cast<PFN_vkMapMemory>(loader_platform_get_proc_address(loader_handle, "vkMapMemory"));
    UnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(loader_platform_get_proc_address(loader_handle, "vkUnmapMemory"));
    FlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(loader_platform_get_proc_address(loader_handle, "vkFlushMappedMemoryRanges"));
    InvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(loader_platform_get_proc_address(loader_handle, "vkInvalidateMappedMemoryRanges"));
    GetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(loader_platform_get_proc_address(loader_handle, "vkGetDeviceMemoryCommitment"));
    BindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(loader_platform_get_proc_address(loader_handle, "vkBindBufferMemory"));
    BindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(loader_platform_get_proc_address(loader_handle, "vkBindImageMemory"));
    GetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(loader_platform_get_proc_address(loader_handle, "vkGetBufferMemoryRequirements"));
    GetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(loader_platform_get_proc_address(loader_handle, "vkGetImageMemoryRequirements"));
    GetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(loader_platform_get_proc_address(loader_handle, "vkGetImageSparseMemoryRequirements"));
    GetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceSparseImageFormatProperties"));
    QueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(loader_platform_get_proc_address(loader_handle, "vkQueueBindSparse"));
    CreateFence = reinterpret_cast<PFN_vkCreateFence>(loader_platform_get_proc_address(loader_handle, "vkCreateFence"));
    DestroyFence = reinterpret_cast<PFN_vkDestroyFence>(loader_platform_get_proc_address(loader_handle, "vkDestroyFence"));
    ResetFences = reinterpret_cast<PFN_vkResetFences>(loader_platform_get_proc_address(loader_handle, "vkResetFences"));
    GetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(loader_platform_get_proc_address(loader_handle, "vkGetFenceStatus"));
    WaitForFences = reinterpret_cast<PFN_vkWaitForFences>(loader_platform_get_proc_address(loader_handle, "vkWaitForFences"));
    CreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(loader_platform_get_proc_address(loader_handle, "vkCreateSemaphore"));
    DestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(loader_platform_get_proc_address(loader_handle, "vkDestroySemaphore"));
    CreateEvent = reinterpret_cast<PFN_vkCreateEvent>(loader_platform_get_proc_address(loader_handle, "vkCreateEvent"));
    DestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(loader_platform_get_proc_address(loader_handle, "vkDestroyEvent"));
    GetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(loader_platform_get_proc_address(loader_handle, "vkGetEventStatus"));
    SetEvent = reinterpret_cast<PFN_vkSetEvent>(loader_platform_get_proc_address(loader_handle, "vkSetEvent"));
    ResetEvent = reinterpret_cast<PFN_vkResetEvent>(loader_platform_get_proc_address(loader_handle, "vkResetEvent"));
    CreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(loader_platform_get_proc_address(loader_handle, "vkCreateQueryPool"));
    DestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(loader_platform_get_proc_address(loader_handle, "vkDestroyQueryPool"));
    GetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(loader_platform_get_proc_address(loader_handle, "vkGetQueryPoolResults"));
    CreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(loader_platform_get_proc_address(loader_handle, "vkCreateBuffer"));
    DestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(loader_platform_get_proc_address(loader_handle, "vkDestroyBuffer"));
    CreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(loader_platform_get_proc_address(loader_handle, "vkCreateBufferView"));
    DestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(loader_platform_get_proc_address(loader_handle, "vkDestroyBufferView"));
    CreateImage = reinterpret_cast<PFN_vkCreateImage>(loader_platform_get_proc_address(loader_handle, "vkCreateImage"));
    DestroyImage = reinterpret_cast<PFN_vkDestroyImage>(loader_platform_get_proc_address(loader_handle, "vkDestroyImage"));
    GetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(loader_platform_get_proc_address(loader_handle, "vkGetImageSubresourceLayout"));
    CreateImageView = reinterpret_cast<PFN_vkCreateImageView>(loader_platform_get_proc_address(loader_handle, "vkCreateImageView"));
    DestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(loader_platform_get_proc_address(loader_handle, "vkDestroyImageView"));
    CreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(loader_platform_get_proc_address(loader_handle, "vkCreateShaderModule"));
    DestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(loader_platform_get_proc_address(loader_handle, "vkDestroyShaderModule"));
    CreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(loader_platform_get_proc_address(loader_handle, "vkCreatePipelineCache"));
    DestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(loader_platform_get_proc_address(loader_handle, "vkDestroyPipelineCache"));
    GetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(loader_platform_get_proc_address(loader_handle, "vkGetPipelineCacheData"));
    MergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(loader_platform_get_proc_address(loader_handle, "vkMergePipelineCaches"));
    CreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(loader_platform_get_proc_address(loader_handle, "vkCreateGraphicsPipelines"));
    CreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(loader_platform_get_proc_address(loader_handle, "vkCreateComputePipelines"));
    DestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(loader_platform_get_proc_address(loader_handle, "vkDestroyPipeline"));
    CreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(loader_platform_get_proc_address(loader_handle, "vkCreatePipelineLayout"));
    DestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(loader_platform_get_proc_address(loader_handle, "vkDestroyPipelineLayout"));
    CreateSampler = reinterpret_cast<PFN_vkCreateSampler>(loader_platform_get_proc_address(loader_handle, "vkCreateSampler"));
    DestroySampler = reinterpret_cast<PFN_vkDestroySampler>(loader_platform_get_proc_address(loader_handle, "vkDestroySampler"));
    CreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(loader_platform_get_proc_address(loader_handle, "vkCreateDescriptorSetLayout"));
    DestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(loader_platform_get_proc_address(loader_handle, "vkDestroyDescriptorSetLayout"));
    CreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(loader_platform_get_proc_address(loader_handle, "vkCreateDescriptorPool"));
    DestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(loader_platform_get_proc_address(loader_handle, "vkDestroyDescriptorPool"));
    ResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(loader_platform_get_proc_address(loader_handle, "vkResetDescriptorPool"));
    AllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(loader_platform_get_proc_address(loader_handle, "vkAllocateDescriptorSets"));
    FreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(loader_platform_get_proc_address(loader_handle, "vkFreeDescriptorSets"));
    UpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(loader_platform_get_proc_address(loader_handle, "vkUpdateDescriptorSets"));
    CreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(loader_platform_get_proc_address(loader_handle, "vkCreateFramebuffer"));
    DestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(loader_platform_get_proc_address(loader_handle, "vkDestroyFramebuffer"));
    CreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(loader_platform_get_proc_address(loader_handle, "vkCreateRenderPass"));
    DestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(loader_platform_get_proc_address(loader_handle, "vkDestroyRenderPass"));
    GetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(loader_platform_get_proc_address(loader_handle, "vkGetRenderAreaGranularity"));
    CreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(loader_platform_get_proc_address(loader_handle, "vkCreateCommandPool"));
    DestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(loader_platform_get_proc_address(loader_handle, "vkDestroyCommandPool"));
    ResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(loader_platform_get_proc_address(loader_handle, "vkResetCommandPool"));
    AllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(loader_platform_get_proc_address(loader_handle, "vkAllocateCommandBuffers"));
    FreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(loader_platform_get_proc_address(loader_handle, "vkFreeCommandBuffers"));
    BeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(loader_platform_get_proc_address(loader_handle, "vkBeginCommandBuffer"));
    EndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(loader_platform_get_proc_address(loader_handle, "vkEndCommandBuffer"));
    ResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(loader_platform_get_proc_address(loader_handle, "vkResetCommandBuffer"));
    CmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(loader_platform_get_proc_address(loader_handle, "vkCmdBindPipeline"));
    CmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(loader_platform_get_proc_address(loader_handle, "vkCmdSetViewport"));
    CmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(loader_platform_get_proc_address(loader_handle, "vkCmdSetScissor"));
    CmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(loader_platform_get_proc_address(loader_handle, "vkCmdSetLineWidth"));
    CmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(loader_platform_get_proc_address(loader_handle, "vkCmdSetDepthBias"));
    CmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(loader_platform_get_proc_address(loader_handle, "vkCmdSetBlendConstants"));
    CmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(loader_platform_get_proc_address(loader_handle, "vkCmdSetDepthBounds"));
    CmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(loader_platform_get_proc_address(loader_handle, "vkCmdSetStencilCompareMask"));
    CmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(loader_platform_get_proc_address(loader_handle, "vkCmdSetStencilWriteMask"));
    CmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(loader_platform_get_proc_address(loader_handle, "vkCmdSetStencilReference"));
    CmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(loader_platform_get_proc_address(loader_handle, "vkCmdBindDescriptorSets"));
    CmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(loader_platform_get_proc_address(loader_handle, "vkCmdBindIndexBuffer"));
    CmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(loader_platform_get_proc_address(loader_handle, "vkCmdBindVertexBuffers"));
    CmdDraw = reinterpret_cast<PFN_vkCmdDraw>(loader_platform_get_proc_address(loader_handle, "vkCmdDraw"));
    CmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(loader_platform_get_proc_address(loader_handle, "vkCmdDrawIndexed"));
    CmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(loader_platform_get_proc_address(loader_handle, "vkCmdDrawIndirect"));
    CmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(loader_platform_get_proc_address(loader_handle, "vkCmdDrawIndexedIndirect"));
    CmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(loader_platform_get_proc_address(loader_handle, "vkCmdDispatch"));
    CmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(loader_platform_get_proc_address(loader_handle, "vkCmdDispatchIndirect"));
    CmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(loader_platform_get_proc_address(loader_handle, "vkCmdCopyBuffer"));
    CmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(loader_platform_get_proc_address(loader_handle, "vkCmdCopyImage"));
    CmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(loader_platform_get_proc_address(loader_handle, "vkCmdBlitImage"));
    CmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(loader_platform_get_proc_address(loader_handle, "vkCmdCopyBufferToImage"));
    CmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(loader_platform_get_proc_address(loader_handle, "vkCmdCopyImageToBuffer"));
    CmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(loader_platform_get_proc_address(loader_handle, "vkCmdUpdateBuffer"));
    CmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(loader_platform_get_proc_address(loader_handle, "vkCmdFillBuffer"));
    CmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(loader_platform_get_proc_address(loader_handle, "vkCmdClearColorImage"));
    CmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(loader_platform_get_proc_address(loader_handle, "vkCmdClearDepthStencilImage"));
    CmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(loader_platform_get_proc_address(loader_handle, "vkCmdClearAttachments"));
    CmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(loader_platform_get_proc_address(loader_handle, "vkCmdResolveImage"));
    CmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(loader_platform_get_proc_address(loader_handle, "vkCmdSetEvent"));
    CmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(loader_platform_get_proc_address(loader_handle, "vkCmdResetEvent"));
    CmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(loader_platform_get_proc_address(loader_handle, "vkCmdWaitEvents"));
    CmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(loader_platform_get_proc_address(loader_handle, "vkCmdPipelineBarrier"));
    CmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(loader_platform_get_proc_address(loader_handle, "vkCmdBeginQuery"));
    CmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(loader_platform_get_proc_address(loader_handle, "vkCmdEndQuery"));
    CmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(loader_platform_get_proc_address(loader_handle, "vkCmdResetQueryPool"));
    CmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(loader_platform_get_proc_address(loader_handle, "vkCmdWriteTimestamp"));
    CmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(loader_platform_get_proc_address(loader_handle, "vkCmdCopyQueryPoolResults"));
    CmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(loader_platform_get_proc_address(loader_handle, "vkCmdPushConstants"));
    CmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(loader_platform_get_proc_address(loader_handle, "vkCmdBeginRenderPass"));
    CmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(loader_platform_get_proc_address(loader_handle, "vkCmdNextSubpass"));
    CmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(loader_platform_get_proc_address(loader_handle, "vkCmdEndRenderPass"));
    CmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(loader_platform_get_proc_address(loader_handle, "vkCmdExecuteCommands"));
    EnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(loader_platform_get_proc_address(loader_handle, "vkEnumerateInstanceVersion"));
    BindBufferMemory2 = reinterpret_cast<PFN_vkBindBufferMemory2>(loader_platform_get_proc_address(loader_handle, "vkBindBufferMemory2"));
    BindImageMemory2 = reinterpret_cast<PFN_vkBindImageMemory2>(loader_platform_get_proc_address(loader_handle, "vkBindImageMemory2"));
    GetDeviceGroupPeerMemoryFeatures = reinterpret_cast<PFN_vkGetDeviceGroupPeerMemoryFeatures>(loader_platform_get_proc_address(loader_handle, "vkGetDeviceGroupPeerMemoryFeatures"));
    CmdSetDeviceMask = reinterpret_cast<PFN_vkCmdSetDeviceMask>(loader_platform_get_proc_address(loader_handle, "vkCmdSetDeviceMask"));
    CmdDispatchBase = reinterpret_cast<PFN_vkCmdDispatchBase>(loader_platform_get_proc_address(loader_handle, "vkCmdDispatchBase"));
    EnumeratePhysicalDeviceGroups = reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroups>(loader_platform_get_proc_address(loader_handle, "vkEnumeratePhysicalDeviceGroups"));
    GetImageMemoryRequirements2 = reinterpret_cast<PFN_vkGetImageMemoryRequirements2>(loader_platform_get_proc_address(loader_handle, "vkGetImageMemoryRequirements2"));
    GetBufferMemoryRequirements2 = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2>(loader_platform_get_proc_address(loader_handle, "vkGetBufferMemoryRequirements2"));
    GetImageSparseMemoryRequirements2 = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements2>(loader_platform_get_proc_address(loader_handle, "vkGetImageSparseMemoryRequirements2"));
    GetPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceFeatures2"));
    GetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceProperties2"));
    GetPhysicalDeviceFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties2>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceFormatProperties2"));
    GetPhysicalDeviceImageFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceImageFormatProperties2"));
    GetPhysicalDeviceQueueFamilyProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceQueueFamilyProperties2"));
    GetPhysicalDeviceMemoryProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceMemoryProperties2"));
    GetPhysicalDeviceSparseImageFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties2>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceSparseImageFormatProperties2"));
    TrimCommandPool = reinterpret_cast<PFN_vkTrimCommandPool>(loader_platform_get_proc_address(loader_handle, "vkTrimCommandPool"));
    GetDeviceQueue2 = reinterpret_cast<PFN_vkGetDeviceQueue2>(loader_platform_get_proc_address(loader_handle, "vkGetDeviceQueue2"));
    CreateSamplerYcbcrConversion = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversion>(loader_platform_get_proc_address(loader_handle, "vkCreateSamplerYcbcrConversion"));
    DestroySamplerYcbcrConversion = reinterpret_cast<PFN_vkDestroySamplerYcbcrConversion>(loader_platform_get_proc_address(loader_handle, "vkDestroySamplerYcbcrConversion"));
    CreateDescriptorUpdateTemplate = reinterpret_cast<PFN_vkCreateDescriptorUpdateTemplate>(loader_platform_get_proc_address(loader_handle, "vkCreateDescriptorUpdateTemplate"));
    DestroyDescriptorUpdateTemplate = reinterpret_cast<PFN_vkDestroyDescriptorUpdateTemplate>(loader_platform_get_proc_address(loader_handle, "vkDestroyDescriptorUpdateTemplate"));
    UpdateDescriptorSetWithTemplate = reinterpret_cast<PFN_vkUpdateDescriptorSetWithTemplate>(loader_platform_get_proc_address(loader_handle, "vkUpdateDescriptorSetWithTemplate"));
    GetPhysicalDeviceExternalBufferProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalBufferProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceExternalBufferProperties"));
    GetPhysicalDeviceExternalFenceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalFenceProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceExternalFenceProperties"));
    GetPhysicalDeviceExternalSemaphoreProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalSemaphoreProperties>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceExternalSemaphoreProperties"));
    GetDescriptorSetLayoutSupport = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSupport>(loader_platform_get_proc_address(loader_handle, "vkGetDescriptorSetLayoutSupport"));
    DestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(loader_platform_get_proc_address(loader_handle, "vkDestroySurfaceKHR"));
    GetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    GetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    GetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    GetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    CreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateSwapchainKHR"));
    DestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(loader_platform_get_proc_address(loader_handle, "vkDestroySwapchainKHR"));
    GetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetSwapchainImagesKHR"));
    AcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(loader_platform_get_proc_address(loader_handle, "vkAcquireNextImageKHR"));
    QueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(loader_platform_get_proc_address(loader_handle, "vkQueuePresentKHR"));
    GetDeviceGroupPresentCapabilitiesKHR = reinterpret_cast<PFN_vkGetDeviceGroupPresentCapabilitiesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetDeviceGroupPresentCapabilitiesKHR"));
    GetDeviceGroupSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetDeviceGroupSurfacePresentModesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetDeviceGroupSurfacePresentModesKHR"));
    GetPhysicalDevicePresentRectanglesKHR = reinterpret_cast<PFN_vkGetPhysicalDevicePresentRectanglesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDevicePresentRectanglesKHR"));
    AcquireNextImage2KHR = reinterpret_cast<PFN_vkAcquireNextImage2KHR>(loader_platform_get_proc_address(loader_handle, "vkAcquireNextImage2KHR"));
    GetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
    GetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
    GetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(loader_platform_get_proc_address(loader_handle, "vkGetDisplayPlaneSupportedDisplaysKHR"));
    GetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetDisplayModePropertiesKHR"));
    CreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateDisplayModeKHR"));
    GetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(loader_platform_get_proc_address(loader_handle, "vkGetDisplayPlaneCapabilitiesKHR"));
    CreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateDisplayPlaneSurfaceKHR"));
#ifdef VK_USE_PLATFORM_XLIB_KHR
    CreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateXlibSurfaceKHR"));
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    GetPhysicalDeviceXlibPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceXlibPresentationSupportKHR"));
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    CreateXcbSurfaceKHR = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateXcbSurfaceKHR"));
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    GetPhysicalDeviceXcbPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    CreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateWaylandSurfaceKHR"));
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    GetPhysicalDeviceWaylandPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    CreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateAndroidSurfaceKHR"));
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    CreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(loader_platform_get_proc_address(loader_handle, "vkCreateWin32SurfaceKHR"));
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    GetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(loader_platform_get_proc_address(loader_handle, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_MACOS_MVK
    CreateMacOSSurfaceMVK = reinterpret_cast<PFN_vkCreateMacOSSurfaceMVK>(loader_platform_get_proc_address(loader_handle, "vkCreateMacOSSurfaceMVK"));
#endif // VK_USE_PLATFORM_MACOS_MVK
}

} // namespace vk
