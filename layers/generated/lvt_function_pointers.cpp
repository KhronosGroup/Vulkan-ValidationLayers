// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See lvt_file_generator.py for modifications

/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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


#include "lvt_function_pointers.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
// Dynamic Loading:
typedef HMODULE dl_handle;
static dl_handle open_library(const char *lib_path) {
    // Try loading the library the original way first.
    dl_handle lib_handle = LoadLibrary(lib_path);
    if (lib_handle == NULL && GetLastError() == ERROR_MOD_NOT_FOUND) {
        // If that failed, then try loading it with broader search folders.
        lib_handle = LoadLibraryEx(lib_path, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    }
    return lib_handle;
}
static char *open_library_error(const char *libPath) {
    static char errorMsg[164];
    (void)snprintf(errorMsg, 163, "Failed to open dynamic library \"%s\" with error %lu", libPath, GetLastError());
    return errorMsg;
}
static void *get_proc_address(dl_handle library, const char *name) {
    assert(library);
    assert(name);
    return (void *)GetProcAddress(library, name);
}
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)

#include <dlfcn.h>

typedef void *dl_handle;
static inline dl_handle open_library(const char *libPath) {
    // When loading the library, we use RTLD_LAZY so that not all symbols have to be
    // resolved at this time (which improves performance). Note that if not all symbols
    // can be resolved, this could cause crashes later. Use the LD_BIND_NOW environment
    // variable to force all symbols to be resolved here.
    return dlopen(libPath, RTLD_LAZY | RTLD_LOCAL);
}
static inline const char *open_library_error(const char *libPath) { return dlerror(); }
static inline void *get_proc_address(dl_handle library, const char *name) {
    assert(library);
    assert(name);
    return dlsym(library, name);
}
#else
#error Dynamic library functions must be defined for this OS.
#endif


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

#if(WIN32)
    const char filename[] = "vulkan-1.dll";
    auto lib_handle = open_library(filename);
#elif(__APPLE__)
    const char filename[] = "libvulkan.dylib";
    auto lib_handle = open_library(filename);
#else
    const char *filename = "libvulkan.so";
    auto lib_handle = open_library(filename);
    if (!lib_handle) {
        filename = "libvulkan.so.1";
        lib_handle = open_library(filename);
    }
#endif

    if (lib_handle == nullptr) {
        printf("%s\n", open_library_error(filename));
        exit(1);
    }

    CreateInstance = reinterpret_cast<PFN_vkCreateInstance>(get_proc_address(lib_handle, "vkCreateInstance"));
    DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(get_proc_address(lib_handle, "vkDestroyInstance"));
    EnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(get_proc_address(lib_handle, "vkEnumeratePhysicalDevices"));
    GetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(get_proc_address(lib_handle, "vkGetPhysicalDeviceFeatures"));
    GetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceFormatProperties"));
    GetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceImageFormatProperties"));
    GetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceProperties"));
    GetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceQueueFamilyProperties"));
    GetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceMemoryProperties"));
    GetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(get_proc_address(lib_handle, "vkGetInstanceProcAddr"));
    GetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(get_proc_address(lib_handle, "vkGetDeviceProcAddr"));
    CreateDevice = reinterpret_cast<PFN_vkCreateDevice>(get_proc_address(lib_handle, "vkCreateDevice"));
    DestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(get_proc_address(lib_handle, "vkDestroyDevice"));
    EnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(get_proc_address(lib_handle, "vkEnumerateInstanceExtensionProperties"));
    EnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(get_proc_address(lib_handle, "vkEnumerateDeviceExtensionProperties"));
    EnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(get_proc_address(lib_handle, "vkEnumerateInstanceLayerProperties"));
    EnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(get_proc_address(lib_handle, "vkEnumerateDeviceLayerProperties"));
    GetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(get_proc_address(lib_handle, "vkGetDeviceQueue"));
    QueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(get_proc_address(lib_handle, "vkQueueSubmit"));
    QueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(get_proc_address(lib_handle, "vkQueueWaitIdle"));
    DeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(get_proc_address(lib_handle, "vkDeviceWaitIdle"));
    AllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(get_proc_address(lib_handle, "vkAllocateMemory"));
    FreeMemory = reinterpret_cast<PFN_vkFreeMemory>(get_proc_address(lib_handle, "vkFreeMemory"));
    MapMemory = reinterpret_cast<PFN_vkMapMemory>(get_proc_address(lib_handle, "vkMapMemory"));
    UnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(get_proc_address(lib_handle, "vkUnmapMemory"));
    FlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(get_proc_address(lib_handle, "vkFlushMappedMemoryRanges"));
    InvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(get_proc_address(lib_handle, "vkInvalidateMappedMemoryRanges"));
    GetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(get_proc_address(lib_handle, "vkGetDeviceMemoryCommitment"));
    BindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(get_proc_address(lib_handle, "vkBindBufferMemory"));
    BindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(get_proc_address(lib_handle, "vkBindImageMemory"));
    GetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(get_proc_address(lib_handle, "vkGetBufferMemoryRequirements"));
    GetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(get_proc_address(lib_handle, "vkGetImageMemoryRequirements"));
    GetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(get_proc_address(lib_handle, "vkGetImageSparseMemoryRequirements"));
    GetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceSparseImageFormatProperties"));
    QueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(get_proc_address(lib_handle, "vkQueueBindSparse"));
    CreateFence = reinterpret_cast<PFN_vkCreateFence>(get_proc_address(lib_handle, "vkCreateFence"));
    DestroyFence = reinterpret_cast<PFN_vkDestroyFence>(get_proc_address(lib_handle, "vkDestroyFence"));
    ResetFences = reinterpret_cast<PFN_vkResetFences>(get_proc_address(lib_handle, "vkResetFences"));
    GetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(get_proc_address(lib_handle, "vkGetFenceStatus"));
    WaitForFences = reinterpret_cast<PFN_vkWaitForFences>(get_proc_address(lib_handle, "vkWaitForFences"));
    CreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(get_proc_address(lib_handle, "vkCreateSemaphore"));
    DestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(get_proc_address(lib_handle, "vkDestroySemaphore"));
    CreateEvent = reinterpret_cast<PFN_vkCreateEvent>(get_proc_address(lib_handle, "vkCreateEvent"));
    DestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(get_proc_address(lib_handle, "vkDestroyEvent"));
    GetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(get_proc_address(lib_handle, "vkGetEventStatus"));
    SetEvent = reinterpret_cast<PFN_vkSetEvent>(get_proc_address(lib_handle, "vkSetEvent"));
    ResetEvent = reinterpret_cast<PFN_vkResetEvent>(get_proc_address(lib_handle, "vkResetEvent"));
    CreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(get_proc_address(lib_handle, "vkCreateQueryPool"));
    DestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(get_proc_address(lib_handle, "vkDestroyQueryPool"));
    GetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(get_proc_address(lib_handle, "vkGetQueryPoolResults"));
    CreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(get_proc_address(lib_handle, "vkCreateBuffer"));
    DestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(get_proc_address(lib_handle, "vkDestroyBuffer"));
    CreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(get_proc_address(lib_handle, "vkCreateBufferView"));
    DestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(get_proc_address(lib_handle, "vkDestroyBufferView"));
    CreateImage = reinterpret_cast<PFN_vkCreateImage>(get_proc_address(lib_handle, "vkCreateImage"));
    DestroyImage = reinterpret_cast<PFN_vkDestroyImage>(get_proc_address(lib_handle, "vkDestroyImage"));
    GetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(get_proc_address(lib_handle, "vkGetImageSubresourceLayout"));
    CreateImageView = reinterpret_cast<PFN_vkCreateImageView>(get_proc_address(lib_handle, "vkCreateImageView"));
    DestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(get_proc_address(lib_handle, "vkDestroyImageView"));
    CreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(get_proc_address(lib_handle, "vkCreateShaderModule"));
    DestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(get_proc_address(lib_handle, "vkDestroyShaderModule"));
    CreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(get_proc_address(lib_handle, "vkCreatePipelineCache"));
    DestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(get_proc_address(lib_handle, "vkDestroyPipelineCache"));
    GetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(get_proc_address(lib_handle, "vkGetPipelineCacheData"));
    MergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(get_proc_address(lib_handle, "vkMergePipelineCaches"));
    CreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(get_proc_address(lib_handle, "vkCreateGraphicsPipelines"));
    CreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(get_proc_address(lib_handle, "vkCreateComputePipelines"));
    DestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(get_proc_address(lib_handle, "vkDestroyPipeline"));
    CreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(get_proc_address(lib_handle, "vkCreatePipelineLayout"));
    DestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(get_proc_address(lib_handle, "vkDestroyPipelineLayout"));
    CreateSampler = reinterpret_cast<PFN_vkCreateSampler>(get_proc_address(lib_handle, "vkCreateSampler"));
    DestroySampler = reinterpret_cast<PFN_vkDestroySampler>(get_proc_address(lib_handle, "vkDestroySampler"));
    CreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(get_proc_address(lib_handle, "vkCreateDescriptorSetLayout"));
    DestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(get_proc_address(lib_handle, "vkDestroyDescriptorSetLayout"));
    CreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(get_proc_address(lib_handle, "vkCreateDescriptorPool"));
    DestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(get_proc_address(lib_handle, "vkDestroyDescriptorPool"));
    ResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(get_proc_address(lib_handle, "vkResetDescriptorPool"));
    AllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(get_proc_address(lib_handle, "vkAllocateDescriptorSets"));
    FreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(get_proc_address(lib_handle, "vkFreeDescriptorSets"));
    UpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(get_proc_address(lib_handle, "vkUpdateDescriptorSets"));
    CreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(get_proc_address(lib_handle, "vkCreateFramebuffer"));
    DestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(get_proc_address(lib_handle, "vkDestroyFramebuffer"));
    CreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(get_proc_address(lib_handle, "vkCreateRenderPass"));
    DestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(get_proc_address(lib_handle, "vkDestroyRenderPass"));
    GetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(get_proc_address(lib_handle, "vkGetRenderAreaGranularity"));
    CreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(get_proc_address(lib_handle, "vkCreateCommandPool"));
    DestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(get_proc_address(lib_handle, "vkDestroyCommandPool"));
    ResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(get_proc_address(lib_handle, "vkResetCommandPool"));
    AllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(get_proc_address(lib_handle, "vkAllocateCommandBuffers"));
    FreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(get_proc_address(lib_handle, "vkFreeCommandBuffers"));
    BeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(get_proc_address(lib_handle, "vkBeginCommandBuffer"));
    EndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(get_proc_address(lib_handle, "vkEndCommandBuffer"));
    ResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(get_proc_address(lib_handle, "vkResetCommandBuffer"));
    CmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(get_proc_address(lib_handle, "vkCmdBindPipeline"));
    CmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(get_proc_address(lib_handle, "vkCmdSetViewport"));
    CmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(get_proc_address(lib_handle, "vkCmdSetScissor"));
    CmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(get_proc_address(lib_handle, "vkCmdSetLineWidth"));
    CmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(get_proc_address(lib_handle, "vkCmdSetDepthBias"));
    CmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(get_proc_address(lib_handle, "vkCmdSetBlendConstants"));
    CmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(get_proc_address(lib_handle, "vkCmdSetDepthBounds"));
    CmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(get_proc_address(lib_handle, "vkCmdSetStencilCompareMask"));
    CmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(get_proc_address(lib_handle, "vkCmdSetStencilWriteMask"));
    CmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(get_proc_address(lib_handle, "vkCmdSetStencilReference"));
    CmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(get_proc_address(lib_handle, "vkCmdBindDescriptorSets"));
    CmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(get_proc_address(lib_handle, "vkCmdBindIndexBuffer"));
    CmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(get_proc_address(lib_handle, "vkCmdBindVertexBuffers"));
    CmdDraw = reinterpret_cast<PFN_vkCmdDraw>(get_proc_address(lib_handle, "vkCmdDraw"));
    CmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(get_proc_address(lib_handle, "vkCmdDrawIndexed"));
    CmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(get_proc_address(lib_handle, "vkCmdDrawIndirect"));
    CmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(get_proc_address(lib_handle, "vkCmdDrawIndexedIndirect"));
    CmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(get_proc_address(lib_handle, "vkCmdDispatch"));
    CmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(get_proc_address(lib_handle, "vkCmdDispatchIndirect"));
    CmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(get_proc_address(lib_handle, "vkCmdCopyBuffer"));
    CmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(get_proc_address(lib_handle, "vkCmdCopyImage"));
    CmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(get_proc_address(lib_handle, "vkCmdBlitImage"));
    CmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(get_proc_address(lib_handle, "vkCmdCopyBufferToImage"));
    CmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(get_proc_address(lib_handle, "vkCmdCopyImageToBuffer"));
    CmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(get_proc_address(lib_handle, "vkCmdUpdateBuffer"));
    CmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(get_proc_address(lib_handle, "vkCmdFillBuffer"));
    CmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(get_proc_address(lib_handle, "vkCmdClearColorImage"));
    CmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(get_proc_address(lib_handle, "vkCmdClearDepthStencilImage"));
    CmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(get_proc_address(lib_handle, "vkCmdClearAttachments"));
    CmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(get_proc_address(lib_handle, "vkCmdResolveImage"));
    CmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(get_proc_address(lib_handle, "vkCmdSetEvent"));
    CmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(get_proc_address(lib_handle, "vkCmdResetEvent"));
    CmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(get_proc_address(lib_handle, "vkCmdWaitEvents"));
    CmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(get_proc_address(lib_handle, "vkCmdPipelineBarrier"));
    CmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(get_proc_address(lib_handle, "vkCmdBeginQuery"));
    CmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(get_proc_address(lib_handle, "vkCmdEndQuery"));
    CmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(get_proc_address(lib_handle, "vkCmdResetQueryPool"));
    CmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(get_proc_address(lib_handle, "vkCmdWriteTimestamp"));
    CmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(get_proc_address(lib_handle, "vkCmdCopyQueryPoolResults"));
    CmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(get_proc_address(lib_handle, "vkCmdPushConstants"));
    CmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(get_proc_address(lib_handle, "vkCmdBeginRenderPass"));
    CmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(get_proc_address(lib_handle, "vkCmdNextSubpass"));
    CmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(get_proc_address(lib_handle, "vkCmdEndRenderPass"));
    CmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(get_proc_address(lib_handle, "vkCmdExecuteCommands"));
    EnumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(get_proc_address(lib_handle, "vkEnumerateInstanceVersion"));
    BindBufferMemory2 = reinterpret_cast<PFN_vkBindBufferMemory2>(get_proc_address(lib_handle, "vkBindBufferMemory2"));
    BindImageMemory2 = reinterpret_cast<PFN_vkBindImageMemory2>(get_proc_address(lib_handle, "vkBindImageMemory2"));
    GetDeviceGroupPeerMemoryFeatures = reinterpret_cast<PFN_vkGetDeviceGroupPeerMemoryFeatures>(get_proc_address(lib_handle, "vkGetDeviceGroupPeerMemoryFeatures"));
    CmdSetDeviceMask = reinterpret_cast<PFN_vkCmdSetDeviceMask>(get_proc_address(lib_handle, "vkCmdSetDeviceMask"));
    CmdDispatchBase = reinterpret_cast<PFN_vkCmdDispatchBase>(get_proc_address(lib_handle, "vkCmdDispatchBase"));
    EnumeratePhysicalDeviceGroups = reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroups>(get_proc_address(lib_handle, "vkEnumeratePhysicalDeviceGroups"));
    GetImageMemoryRequirements2 = reinterpret_cast<PFN_vkGetImageMemoryRequirements2>(get_proc_address(lib_handle, "vkGetImageMemoryRequirements2"));
    GetBufferMemoryRequirements2 = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2>(get_proc_address(lib_handle, "vkGetBufferMemoryRequirements2"));
    GetImageSparseMemoryRequirements2 = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements2>(get_proc_address(lib_handle, "vkGetImageSparseMemoryRequirements2"));
    GetPhysicalDeviceFeatures2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2>(get_proc_address(lib_handle, "vkGetPhysicalDeviceFeatures2"));
    GetPhysicalDeviceProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(get_proc_address(lib_handle, "vkGetPhysicalDeviceProperties2"));
    GetPhysicalDeviceFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties2>(get_proc_address(lib_handle, "vkGetPhysicalDeviceFormatProperties2"));
    GetPhysicalDeviceImageFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties2>(get_proc_address(lib_handle, "vkGetPhysicalDeviceImageFormatProperties2"));
    GetPhysicalDeviceQueueFamilyProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>(get_proc_address(lib_handle, "vkGetPhysicalDeviceQueueFamilyProperties2"));
    GetPhysicalDeviceMemoryProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(get_proc_address(lib_handle, "vkGetPhysicalDeviceMemoryProperties2"));
    GetPhysicalDeviceSparseImageFormatProperties2 = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties2>(get_proc_address(lib_handle, "vkGetPhysicalDeviceSparseImageFormatProperties2"));
    TrimCommandPool = reinterpret_cast<PFN_vkTrimCommandPool>(get_proc_address(lib_handle, "vkTrimCommandPool"));
    GetDeviceQueue2 = reinterpret_cast<PFN_vkGetDeviceQueue2>(get_proc_address(lib_handle, "vkGetDeviceQueue2"));
    CreateSamplerYcbcrConversion = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversion>(get_proc_address(lib_handle, "vkCreateSamplerYcbcrConversion"));
    DestroySamplerYcbcrConversion = reinterpret_cast<PFN_vkDestroySamplerYcbcrConversion>(get_proc_address(lib_handle, "vkDestroySamplerYcbcrConversion"));
    CreateDescriptorUpdateTemplate = reinterpret_cast<PFN_vkCreateDescriptorUpdateTemplate>(get_proc_address(lib_handle, "vkCreateDescriptorUpdateTemplate"));
    DestroyDescriptorUpdateTemplate = reinterpret_cast<PFN_vkDestroyDescriptorUpdateTemplate>(get_proc_address(lib_handle, "vkDestroyDescriptorUpdateTemplate"));
    UpdateDescriptorSetWithTemplate = reinterpret_cast<PFN_vkUpdateDescriptorSetWithTemplate>(get_proc_address(lib_handle, "vkUpdateDescriptorSetWithTemplate"));
    GetPhysicalDeviceExternalBufferProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalBufferProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceExternalBufferProperties"));
    GetPhysicalDeviceExternalFenceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalFenceProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceExternalFenceProperties"));
    GetPhysicalDeviceExternalSemaphoreProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceExternalSemaphoreProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceExternalSemaphoreProperties"));
    GetDescriptorSetLayoutSupport = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSupport>(get_proc_address(lib_handle, "vkGetDescriptorSetLayoutSupport"));
    CmdDrawIndirectCount = reinterpret_cast<PFN_vkCmdDrawIndirectCount>(get_proc_address(lib_handle, "vkCmdDrawIndirectCount"));
    CmdDrawIndexedIndirectCount = reinterpret_cast<PFN_vkCmdDrawIndexedIndirectCount>(get_proc_address(lib_handle, "vkCmdDrawIndexedIndirectCount"));
    CreateRenderPass2 = reinterpret_cast<PFN_vkCreateRenderPass2>(get_proc_address(lib_handle, "vkCreateRenderPass2"));
    CmdBeginRenderPass2 = reinterpret_cast<PFN_vkCmdBeginRenderPass2>(get_proc_address(lib_handle, "vkCmdBeginRenderPass2"));
    CmdNextSubpass2 = reinterpret_cast<PFN_vkCmdNextSubpass2>(get_proc_address(lib_handle, "vkCmdNextSubpass2"));
    CmdEndRenderPass2 = reinterpret_cast<PFN_vkCmdEndRenderPass2>(get_proc_address(lib_handle, "vkCmdEndRenderPass2"));
    ResetQueryPool = reinterpret_cast<PFN_vkResetQueryPool>(get_proc_address(lib_handle, "vkResetQueryPool"));
    GetSemaphoreCounterValue = reinterpret_cast<PFN_vkGetSemaphoreCounterValue>(get_proc_address(lib_handle, "vkGetSemaphoreCounterValue"));
    WaitSemaphores = reinterpret_cast<PFN_vkWaitSemaphores>(get_proc_address(lib_handle, "vkWaitSemaphores"));
    SignalSemaphore = reinterpret_cast<PFN_vkSignalSemaphore>(get_proc_address(lib_handle, "vkSignalSemaphore"));
    GetBufferDeviceAddress = reinterpret_cast<PFN_vkGetBufferDeviceAddress>(get_proc_address(lib_handle, "vkGetBufferDeviceAddress"));
    GetBufferOpaqueCaptureAddress = reinterpret_cast<PFN_vkGetBufferOpaqueCaptureAddress>(get_proc_address(lib_handle, "vkGetBufferOpaqueCaptureAddress"));
    GetDeviceMemoryOpaqueCaptureAddress = reinterpret_cast<PFN_vkGetDeviceMemoryOpaqueCaptureAddress>(get_proc_address(lib_handle, "vkGetDeviceMemoryOpaqueCaptureAddress"));
    GetPhysicalDeviceToolProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceToolProperties>(get_proc_address(lib_handle, "vkGetPhysicalDeviceToolProperties"));
    CreatePrivateDataSlot = reinterpret_cast<PFN_vkCreatePrivateDataSlot>(get_proc_address(lib_handle, "vkCreatePrivateDataSlot"));
    DestroyPrivateDataSlot = reinterpret_cast<PFN_vkDestroyPrivateDataSlot>(get_proc_address(lib_handle, "vkDestroyPrivateDataSlot"));
    SetPrivateData = reinterpret_cast<PFN_vkSetPrivateData>(get_proc_address(lib_handle, "vkSetPrivateData"));
    GetPrivateData = reinterpret_cast<PFN_vkGetPrivateData>(get_proc_address(lib_handle, "vkGetPrivateData"));
    CmdSetEvent2 = reinterpret_cast<PFN_vkCmdSetEvent2>(get_proc_address(lib_handle, "vkCmdSetEvent2"));
    CmdResetEvent2 = reinterpret_cast<PFN_vkCmdResetEvent2>(get_proc_address(lib_handle, "vkCmdResetEvent2"));
    CmdWaitEvents2 = reinterpret_cast<PFN_vkCmdWaitEvents2>(get_proc_address(lib_handle, "vkCmdWaitEvents2"));
    CmdPipelineBarrier2 = reinterpret_cast<PFN_vkCmdPipelineBarrier2>(get_proc_address(lib_handle, "vkCmdPipelineBarrier2"));
    CmdWriteTimestamp2 = reinterpret_cast<PFN_vkCmdWriteTimestamp2>(get_proc_address(lib_handle, "vkCmdWriteTimestamp2"));
    QueueSubmit2 = reinterpret_cast<PFN_vkQueueSubmit2>(get_proc_address(lib_handle, "vkQueueSubmit2"));
    CmdCopyBuffer2 = reinterpret_cast<PFN_vkCmdCopyBuffer2>(get_proc_address(lib_handle, "vkCmdCopyBuffer2"));
    CmdCopyImage2 = reinterpret_cast<PFN_vkCmdCopyImage2>(get_proc_address(lib_handle, "vkCmdCopyImage2"));
    CmdCopyBufferToImage2 = reinterpret_cast<PFN_vkCmdCopyBufferToImage2>(get_proc_address(lib_handle, "vkCmdCopyBufferToImage2"));
    CmdCopyImageToBuffer2 = reinterpret_cast<PFN_vkCmdCopyImageToBuffer2>(get_proc_address(lib_handle, "vkCmdCopyImageToBuffer2"));
    CmdBlitImage2 = reinterpret_cast<PFN_vkCmdBlitImage2>(get_proc_address(lib_handle, "vkCmdBlitImage2"));
    CmdResolveImage2 = reinterpret_cast<PFN_vkCmdResolveImage2>(get_proc_address(lib_handle, "vkCmdResolveImage2"));
    CmdBeginRendering = reinterpret_cast<PFN_vkCmdBeginRendering>(get_proc_address(lib_handle, "vkCmdBeginRendering"));
    CmdEndRendering = reinterpret_cast<PFN_vkCmdEndRendering>(get_proc_address(lib_handle, "vkCmdEndRendering"));
    CmdSetCullMode = reinterpret_cast<PFN_vkCmdSetCullMode>(get_proc_address(lib_handle, "vkCmdSetCullMode"));
    CmdSetFrontFace = reinterpret_cast<PFN_vkCmdSetFrontFace>(get_proc_address(lib_handle, "vkCmdSetFrontFace"));
    CmdSetPrimitiveTopology = reinterpret_cast<PFN_vkCmdSetPrimitiveTopology>(get_proc_address(lib_handle, "vkCmdSetPrimitiveTopology"));
    CmdSetViewportWithCount = reinterpret_cast<PFN_vkCmdSetViewportWithCount>(get_proc_address(lib_handle, "vkCmdSetViewportWithCount"));
    CmdSetScissorWithCount = reinterpret_cast<PFN_vkCmdSetScissorWithCount>(get_proc_address(lib_handle, "vkCmdSetScissorWithCount"));
    CmdBindVertexBuffers2 = reinterpret_cast<PFN_vkCmdBindVertexBuffers2>(get_proc_address(lib_handle, "vkCmdBindVertexBuffers2"));
    CmdSetDepthTestEnable = reinterpret_cast<PFN_vkCmdSetDepthTestEnable>(get_proc_address(lib_handle, "vkCmdSetDepthTestEnable"));
    CmdSetDepthWriteEnable = reinterpret_cast<PFN_vkCmdSetDepthWriteEnable>(get_proc_address(lib_handle, "vkCmdSetDepthWriteEnable"));
    CmdSetDepthCompareOp = reinterpret_cast<PFN_vkCmdSetDepthCompareOp>(get_proc_address(lib_handle, "vkCmdSetDepthCompareOp"));
    CmdSetDepthBoundsTestEnable = reinterpret_cast<PFN_vkCmdSetDepthBoundsTestEnable>(get_proc_address(lib_handle, "vkCmdSetDepthBoundsTestEnable"));
    CmdSetStencilTestEnable = reinterpret_cast<PFN_vkCmdSetStencilTestEnable>(get_proc_address(lib_handle, "vkCmdSetStencilTestEnable"));
    CmdSetStencilOp = reinterpret_cast<PFN_vkCmdSetStencilOp>(get_proc_address(lib_handle, "vkCmdSetStencilOp"));
    CmdSetRasterizerDiscardEnable = reinterpret_cast<PFN_vkCmdSetRasterizerDiscardEnable>(get_proc_address(lib_handle, "vkCmdSetRasterizerDiscardEnable"));
    CmdSetDepthBiasEnable = reinterpret_cast<PFN_vkCmdSetDepthBiasEnable>(get_proc_address(lib_handle, "vkCmdSetDepthBiasEnable"));
    CmdSetPrimitiveRestartEnable = reinterpret_cast<PFN_vkCmdSetPrimitiveRestartEnable>(get_proc_address(lib_handle, "vkCmdSetPrimitiveRestartEnable"));
    GetDeviceBufferMemoryRequirements = reinterpret_cast<PFN_vkGetDeviceBufferMemoryRequirements>(get_proc_address(lib_handle, "vkGetDeviceBufferMemoryRequirements"));
    GetDeviceImageMemoryRequirements = reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirements>(get_proc_address(lib_handle, "vkGetDeviceImageMemoryRequirements"));
    GetDeviceImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetDeviceImageSparseMemoryRequirements>(get_proc_address(lib_handle, "vkGetDeviceImageSparseMemoryRequirements"));
    DestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(get_proc_address(lib_handle, "vkDestroySurfaceKHR"));
    GetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    GetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    GetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    GetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    CreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(get_proc_address(lib_handle, "vkCreateSwapchainKHR"));
    DestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(get_proc_address(lib_handle, "vkDestroySwapchainKHR"));
    GetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(get_proc_address(lib_handle, "vkGetSwapchainImagesKHR"));
    AcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(get_proc_address(lib_handle, "vkAcquireNextImageKHR"));
    QueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(get_proc_address(lib_handle, "vkQueuePresentKHR"));
    GetDeviceGroupPresentCapabilitiesKHR = reinterpret_cast<PFN_vkGetDeviceGroupPresentCapabilitiesKHR>(get_proc_address(lib_handle, "vkGetDeviceGroupPresentCapabilitiesKHR"));
    GetDeviceGroupSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetDeviceGroupSurfacePresentModesKHR>(get_proc_address(lib_handle, "vkGetDeviceGroupSurfacePresentModesKHR"));
    GetPhysicalDevicePresentRectanglesKHR = reinterpret_cast<PFN_vkGetPhysicalDevicePresentRectanglesKHR>(get_proc_address(lib_handle, "vkGetPhysicalDevicePresentRectanglesKHR"));
    AcquireNextImage2KHR = reinterpret_cast<PFN_vkAcquireNextImage2KHR>(get_proc_address(lib_handle, "vkAcquireNextImage2KHR"));
    GetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
    GetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
    GetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(get_proc_address(lib_handle, "vkGetDisplayPlaneSupportedDisplaysKHR"));
    GetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(get_proc_address(lib_handle, "vkGetDisplayModePropertiesKHR"));
    CreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(get_proc_address(lib_handle, "vkCreateDisplayModeKHR"));
    GetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(get_proc_address(lib_handle, "vkGetDisplayPlaneCapabilitiesKHR"));
    CreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(get_proc_address(lib_handle, "vkCreateDisplayPlaneSurfaceKHR"));
#ifdef VK_USE_PLATFORM_XLIB_KHR
    CreateXlibSurfaceKHR = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(get_proc_address(lib_handle, "vkCreateXlibSurfaceKHR"));
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XLIB_KHR
    GetPhysicalDeviceXlibPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceXlibPresentationSupportKHR"));
#endif // VK_USE_PLATFORM_XLIB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    CreateXcbSurfaceKHR = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(get_proc_address(lib_handle, "vkCreateXcbSurfaceKHR"));
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_XCB_KHR
    GetPhysicalDeviceXcbPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceXcbPresentationSupportKHR"));
#endif // VK_USE_PLATFORM_XCB_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    CreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(get_proc_address(lib_handle, "vkCreateWaylandSurfaceKHR"));
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    GetPhysicalDeviceWaylandPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
#endif // VK_USE_PLATFORM_WAYLAND_KHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    CreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(get_proc_address(lib_handle, "vkCreateAndroidSurfaceKHR"));
#endif // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    CreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(get_proc_address(lib_handle, "vkCreateWin32SurfaceKHR"));
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_WIN32_KHR
    GetPhysicalDeviceWin32PresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(get_proc_address(lib_handle, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
#endif // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_MACOS_MVK
    CreateMacOSSurfaceMVK = reinterpret_cast<PFN_vkCreateMacOSSurfaceMVK>(get_proc_address(lib_handle, "vkCreateMacOSSurfaceMVK"));
#endif // VK_USE_PLATFORM_MACOS_MVK
}

} // namespace vk
