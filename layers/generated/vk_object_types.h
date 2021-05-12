// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See helper_file_generator.py for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (c) 2015-2021 Google Inc.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 *
 ****************************************************************************/


#pragma once

#include "cast_utils.h"

// Object Type enum for validation layer internal object handling
typedef enum VulkanObjectType {
    kVulkanObjectTypeUnknown = 0,
    kVulkanObjectTypeBuffer = 1,
    kVulkanObjectTypeImage = 2,
    kVulkanObjectTypeInstance = 3,
    kVulkanObjectTypePhysicalDevice = 4,
    kVulkanObjectTypeDevice = 5,
    kVulkanObjectTypeQueue = 6,
    kVulkanObjectTypeSemaphore = 7,
    kVulkanObjectTypeCommandBuffer = 8,
    kVulkanObjectTypeFence = 9,
    kVulkanObjectTypeDeviceMemory = 10,
    kVulkanObjectTypeEvent = 11,
    kVulkanObjectTypeQueryPool = 12,
    kVulkanObjectTypeBufferView = 13,
    kVulkanObjectTypeImageView = 14,
    kVulkanObjectTypeShaderModule = 15,
    kVulkanObjectTypePipelineCache = 16,
    kVulkanObjectTypePipelineLayout = 17,
    kVulkanObjectTypePipeline = 18,
    kVulkanObjectTypeRenderPass = 19,
    kVulkanObjectTypeDescriptorSetLayout = 20,
    kVulkanObjectTypeSampler = 21,
    kVulkanObjectTypeDescriptorSet = 22,
    kVulkanObjectTypeDescriptorPool = 23,
    kVulkanObjectTypeFramebuffer = 24,
    kVulkanObjectTypeCommandPool = 25,
    kVulkanObjectTypeSamplerYcbcrConversion = 26,
    kVulkanObjectTypeDescriptorUpdateTemplate = 27,
    kVulkanObjectTypeSurfaceKHR = 28,
    kVulkanObjectTypeSwapchainKHR = 29,
    kVulkanObjectTypeDisplayKHR = 30,
    kVulkanObjectTypeDisplayModeKHR = 31,
    kVulkanObjectTypeVideoSessionKHR = 32,
    kVulkanObjectTypeVideoSessionParametersKHR = 33,
    kVulkanObjectTypeDeferredOperationKHR = 34,
    kVulkanObjectTypeDebugReportCallbackEXT = 35,
    kVulkanObjectTypeCuModuleNVX = 36,
    kVulkanObjectTypeCuFunctionNVX = 37,
    kVulkanObjectTypeDebugUtilsMessengerEXT = 38,
    kVulkanObjectTypeValidationCacheEXT = 39,
    kVulkanObjectTypeAccelerationStructureNV = 40,
    kVulkanObjectTypePerformanceConfigurationINTEL = 41,
    kVulkanObjectTypeIndirectCommandsLayoutNV = 42,
    kVulkanObjectTypePrivateDataSlotEXT = 43,
    kVulkanObjectTypeAccelerationStructureKHR = 44,
    kVulkanObjectTypeMax = 45,
    // Aliases for backwards compatibilty of "promoted" types
    kVulkanObjectTypeDescriptorUpdateTemplateKHR = kVulkanObjectTypeDescriptorUpdateTemplate,
    kVulkanObjectTypeSamplerYcbcrConversionKHR = kVulkanObjectTypeSamplerYcbcrConversion,
} VulkanObjectType;

// Array of object name strings for OBJECT_TYPE enum conversion
static const char * const object_string[kVulkanObjectTypeMax] = {
    "VkNonDispatchableHandle",
    "VkBuffer",
    "VkImage",
    "VkInstance",
    "VkPhysicalDevice",
    "VkDevice",
    "VkQueue",
    "VkSemaphore",
    "VkCommandBuffer",
    "VkFence",
    "VkDeviceMemory",
    "VkEvent",
    "VkQueryPool",
    "VkBufferView",
    "VkImageView",
    "VkShaderModule",
    "VkPipelineCache",
    "VkPipelineLayout",
    "VkPipeline",
    "VkRenderPass",
    "VkDescriptorSetLayout",
    "VkSampler",
    "VkDescriptorSet",
    "VkDescriptorPool",
    "VkFramebuffer",
    "VkCommandPool",
    "VkSamplerYcbcrConversion",
    "VkDescriptorUpdateTemplate",
    "VkSurfaceKHR",
    "VkSwapchainKHR",
    "VkDisplayKHR",
    "VkDisplayModeKHR",
    "VkVideoSessionKHR",
    "VkVideoSessionParametersKHR",
    "VkDeferredOperationKHR",
    "VkDebugReportCallbackEXT",
    "VkCuModuleNVX",
    "VkCuFunctionNVX",
    "VkDebugUtilsMessengerEXT",
    "VkValidationCacheEXT",
    "VkAccelerationStructureNV",
    "VkPerformanceConfigurationINTEL",
    "VkIndirectCommandsLayoutNV",
    "VkPrivateDataSlotEXT",
    "VkAccelerationStructureKHR",
};

// Helper array to get Vulkan VK_EXT_debug_report object type enum from the internal layers version
const VkDebugReportObjectTypeEXT get_debug_report_enum[] = {
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, // kVulkanObjectTypeUnknown
    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,   // kVulkanObjectTypeBuffer
    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT,   // kVulkanObjectTypeImage
    VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT,   // kVulkanObjectTypeInstance
    VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT,   // kVulkanObjectTypePhysicalDevice
    VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT,   // kVulkanObjectTypeDevice
    VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT,   // kVulkanObjectTypeQueue
    VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT,   // kVulkanObjectTypeSemaphore
    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,   // kVulkanObjectTypeCommandBuffer
    VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT,   // kVulkanObjectTypeFence
    VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT,   // kVulkanObjectTypeDeviceMemory
    VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT,   // kVulkanObjectTypeEvent
    VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT,   // kVulkanObjectTypeQueryPool
    VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT,   // kVulkanObjectTypeBufferView
    VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,   // kVulkanObjectTypeImageView
    VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT,   // kVulkanObjectTypeShaderModule
    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT,   // kVulkanObjectTypePipelineCache
    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,   // kVulkanObjectTypePipelineLayout
    VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT,   // kVulkanObjectTypePipeline
    VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT,   // kVulkanObjectTypeRenderPass
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,   // kVulkanObjectTypeDescriptorSetLayout
    VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT,   // kVulkanObjectTypeSampler
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,   // kVulkanObjectTypeDescriptorSet
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,   // kVulkanObjectTypeDescriptorPool
    VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT,   // kVulkanObjectTypeFramebuffer
    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,   // kVulkanObjectTypeCommandPool
    VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT,   // kVulkanObjectTypeSamplerYcbcrConversion
    VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT,   // kVulkanObjectTypeDescriptorUpdateTemplate
    VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT,   // kVulkanObjectTypeSurfaceKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT,   // kVulkanObjectTypeSwapchainKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT,   // kVulkanObjectTypeDisplayKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT,   // kVulkanObjectTypeDisplayModeKHR
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypeVideoSessionKHR
#else
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypeVideoSessionKHR
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypeVideoSessionParametersKHR
#else
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypeVideoSessionParametersKHR
#endif
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypeDeferredOperationKHR
    VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT,   // kVulkanObjectTypeDebugReportCallbackEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT,   // kVulkanObjectTypeCuModuleNVX
    VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT,   // kVulkanObjectTypeCuFunctionNVX
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypeDebugUtilsMessengerEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT,   // kVulkanObjectTypeValidationCacheEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT,   // kVulkanObjectTypeAccelerationStructureNV
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypePerformanceConfigurationINTEL
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypeIndirectCommandsLayoutNV
    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT,   // kVulkanObjectTypePrivateDataSlotEXT
    VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT,   // kVulkanObjectTypeAccelerationStructureKHR
};

// Helper function to get Official Vulkan VkObjectType enum from the internal layers version
static inline VkObjectType ConvertVulkanObjectToCoreObject(VulkanObjectType internal_type) {
    switch (internal_type) {
        case kVulkanObjectTypeBuffer: return VK_OBJECT_TYPE_BUFFER;
        case kVulkanObjectTypeImage: return VK_OBJECT_TYPE_IMAGE;
        case kVulkanObjectTypeInstance: return VK_OBJECT_TYPE_INSTANCE;
        case kVulkanObjectTypePhysicalDevice: return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        case kVulkanObjectTypeDevice: return VK_OBJECT_TYPE_DEVICE;
        case kVulkanObjectTypeQueue: return VK_OBJECT_TYPE_QUEUE;
        case kVulkanObjectTypeSemaphore: return VK_OBJECT_TYPE_SEMAPHORE;
        case kVulkanObjectTypeCommandBuffer: return VK_OBJECT_TYPE_COMMAND_BUFFER;
        case kVulkanObjectTypeFence: return VK_OBJECT_TYPE_FENCE;
        case kVulkanObjectTypeDeviceMemory: return VK_OBJECT_TYPE_DEVICE_MEMORY;
        case kVulkanObjectTypeEvent: return VK_OBJECT_TYPE_EVENT;
        case kVulkanObjectTypeQueryPool: return VK_OBJECT_TYPE_QUERY_POOL;
        case kVulkanObjectTypeBufferView: return VK_OBJECT_TYPE_BUFFER_VIEW;
        case kVulkanObjectTypeImageView: return VK_OBJECT_TYPE_IMAGE_VIEW;
        case kVulkanObjectTypeShaderModule: return VK_OBJECT_TYPE_SHADER_MODULE;
        case kVulkanObjectTypePipelineCache: return VK_OBJECT_TYPE_PIPELINE_CACHE;
        case kVulkanObjectTypePipelineLayout: return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
        case kVulkanObjectTypePipeline: return VK_OBJECT_TYPE_PIPELINE;
        case kVulkanObjectTypeRenderPass: return VK_OBJECT_TYPE_RENDER_PASS;
        case kVulkanObjectTypeDescriptorSetLayout: return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
        case kVulkanObjectTypeSampler: return VK_OBJECT_TYPE_SAMPLER;
        case kVulkanObjectTypeDescriptorSet: return VK_OBJECT_TYPE_DESCRIPTOR_SET;
        case kVulkanObjectTypeDescriptorPool: return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        case kVulkanObjectTypeFramebuffer: return VK_OBJECT_TYPE_FRAMEBUFFER;
        case kVulkanObjectTypeCommandPool: return VK_OBJECT_TYPE_COMMAND_POOL;
        case kVulkanObjectTypeSamplerYcbcrConversion: return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
        case kVulkanObjectTypeDescriptorUpdateTemplate: return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
        case kVulkanObjectTypeSurfaceKHR: return VK_OBJECT_TYPE_SURFACE_KHR;
        case kVulkanObjectTypeSwapchainKHR: return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        case kVulkanObjectTypeDisplayKHR: return VK_OBJECT_TYPE_DISPLAY_KHR;
        case kVulkanObjectTypeDisplayModeKHR: return VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
        case kVulkanObjectTypeVideoSessionKHR: return VK_OBJECT_TYPE_VIDEO_SESSION_KHR;
        case kVulkanObjectTypeVideoSessionParametersKHR: return VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR;
        case kVulkanObjectTypeDeferredOperationKHR: return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR;
        case kVulkanObjectTypeDebugReportCallbackEXT: return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
        case kVulkanObjectTypeCuModuleNVX: return VK_OBJECT_TYPE_CU_MODULE_NVX;
        case kVulkanObjectTypeCuFunctionNVX: return VK_OBJECT_TYPE_CU_FUNCTION_NVX;
        case kVulkanObjectTypeDebugUtilsMessengerEXT: return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT;
        case kVulkanObjectTypeValidationCacheEXT: return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
        case kVulkanObjectTypeAccelerationStructureNV: return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV;
        case kVulkanObjectTypePerformanceConfigurationINTEL: return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL;
        case kVulkanObjectTypeIndirectCommandsLayoutNV: return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV;
        case kVulkanObjectTypePrivateDataSlotEXT: return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT;
        case kVulkanObjectTypeAccelerationStructureKHR: return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        default: return VK_OBJECT_TYPE_UNKNOWN;
    }
};

// Helper function to get internal layers object ids from the official Vulkan VkObjectType enum
static inline VulkanObjectType ConvertCoreObjectToVulkanObject(VkObjectType vulkan_object_type) {
    switch (vulkan_object_type) {
        case VK_OBJECT_TYPE_BUFFER: return kVulkanObjectTypeBuffer;
        case VK_OBJECT_TYPE_IMAGE: return kVulkanObjectTypeImage;
        case VK_OBJECT_TYPE_INSTANCE: return kVulkanObjectTypeInstance;
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return kVulkanObjectTypePhysicalDevice;
        case VK_OBJECT_TYPE_DEVICE: return kVulkanObjectTypeDevice;
        case VK_OBJECT_TYPE_QUEUE: return kVulkanObjectTypeQueue;
        case VK_OBJECT_TYPE_SEMAPHORE: return kVulkanObjectTypeSemaphore;
        case VK_OBJECT_TYPE_COMMAND_BUFFER: return kVulkanObjectTypeCommandBuffer;
        case VK_OBJECT_TYPE_FENCE: return kVulkanObjectTypeFence;
        case VK_OBJECT_TYPE_DEVICE_MEMORY: return kVulkanObjectTypeDeviceMemory;
        case VK_OBJECT_TYPE_EVENT: return kVulkanObjectTypeEvent;
        case VK_OBJECT_TYPE_QUERY_POOL: return kVulkanObjectTypeQueryPool;
        case VK_OBJECT_TYPE_BUFFER_VIEW: return kVulkanObjectTypeBufferView;
        case VK_OBJECT_TYPE_IMAGE_VIEW: return kVulkanObjectTypeImageView;
        case VK_OBJECT_TYPE_SHADER_MODULE: return kVulkanObjectTypeShaderModule;
        case VK_OBJECT_TYPE_PIPELINE_CACHE: return kVulkanObjectTypePipelineCache;
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return kVulkanObjectTypePipelineLayout;
        case VK_OBJECT_TYPE_PIPELINE: return kVulkanObjectTypePipeline;
        case VK_OBJECT_TYPE_RENDER_PASS: return kVulkanObjectTypeRenderPass;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return kVulkanObjectTypeDescriptorSetLayout;
        case VK_OBJECT_TYPE_SAMPLER: return kVulkanObjectTypeSampler;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET: return kVulkanObjectTypeDescriptorSet;
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return kVulkanObjectTypeDescriptorPool;
        case VK_OBJECT_TYPE_FRAMEBUFFER: return kVulkanObjectTypeFramebuffer;
        case VK_OBJECT_TYPE_COMMAND_POOL: return kVulkanObjectTypeCommandPool;
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return kVulkanObjectTypeSamplerYcbcrConversion;
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return kVulkanObjectTypeDescriptorUpdateTemplate;
        case VK_OBJECT_TYPE_SURFACE_KHR: return kVulkanObjectTypeSurfaceKHR;
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return kVulkanObjectTypeSwapchainKHR;
        case VK_OBJECT_TYPE_DISPLAY_KHR: return kVulkanObjectTypeDisplayKHR;
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return kVulkanObjectTypeDisplayModeKHR;
        case VK_OBJECT_TYPE_VIDEO_SESSION_KHR: return kVulkanObjectTypeVideoSessionKHR;
        case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR: return kVulkanObjectTypeVideoSessionParametersKHR;
        case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR: return kVulkanObjectTypeDeferredOperationKHR;
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return kVulkanObjectTypeDebugReportCallbackEXT;
        case VK_OBJECT_TYPE_CU_MODULE_NVX: return kVulkanObjectTypeCuModuleNVX;
        case VK_OBJECT_TYPE_CU_FUNCTION_NVX: return kVulkanObjectTypeCuFunctionNVX;
        case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: return kVulkanObjectTypeDebugUtilsMessengerEXT;
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: return kVulkanObjectTypeValidationCacheEXT;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: return kVulkanObjectTypeAccelerationStructureNV;
        case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL: return kVulkanObjectTypePerformanceConfigurationINTEL;
        case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV: return kVulkanObjectTypeIndirectCommandsLayoutNV;
        case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT: return kVulkanObjectTypePrivateDataSlotEXT;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: return kVulkanObjectTypeAccelerationStructureKHR;
        default: return kVulkanObjectTypeUnknown;
    }
};

static inline VkObjectType convertDebugReportObjectToCoreObject(VkDebugReportObjectTypeEXT debug_report_obj) {
    switch (debug_report_obj) {
        case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT: return VK_OBJECT_TYPE_UNKNOWN;
        case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT: return VK_OBJECT_TYPE_INSTANCE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT: return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT: return VK_OBJECT_TYPE_DEVICE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT: return VK_OBJECT_TYPE_QUEUE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT: return VK_OBJECT_TYPE_SEMAPHORE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT: return VK_OBJECT_TYPE_COMMAND_BUFFER;
        case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT: return VK_OBJECT_TYPE_FENCE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT: return VK_OBJECT_TYPE_DEVICE_MEMORY;
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT: return VK_OBJECT_TYPE_BUFFER;
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT: return VK_OBJECT_TYPE_IMAGE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT: return VK_OBJECT_TYPE_EVENT;
        case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT: return VK_OBJECT_TYPE_QUERY_POOL;
        case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT: return VK_OBJECT_TYPE_BUFFER_VIEW;
        case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT: return VK_OBJECT_TYPE_IMAGE_VIEW;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT: return VK_OBJECT_TYPE_SHADER_MODULE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT: return VK_OBJECT_TYPE_PIPELINE_CACHE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT: return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
        case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT: return VK_OBJECT_TYPE_RENDER_PASS;
        case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT: return VK_OBJECT_TYPE_PIPELINE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT: return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT: return VK_OBJECT_TYPE_SAMPLER;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT: return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT: return VK_OBJECT_TYPE_DESCRIPTOR_SET;
        case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT: return VK_OBJECT_TYPE_FRAMEBUFFER;
        case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT: return VK_OBJECT_TYPE_COMMAND_POOL;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT: return VK_OBJECT_TYPE_SURFACE_KHR;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT: return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT: return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT: return VK_OBJECT_TYPE_DISPLAY_KHR;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT: return VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
        case VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT: return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
        case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT: return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
        case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT: return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT: return VK_OBJECT_TYPE_CU_MODULE_NVX;
        case VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT: return VK_OBJECT_TYPE_CU_FUNCTION_NVX;
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT: return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
        case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT: return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV;
        default: return VK_OBJECT_TYPE_UNKNOWN;
    }
}

static inline VkDebugReportObjectTypeEXT convertCoreObjectToDebugReportObject(VkObjectType core_report_obj) {
    switch (core_report_obj) {
        case VK_OBJECT_TYPE_UNKNOWN: return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        case VK_OBJECT_TYPE_INSTANCE: return VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
        case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
        case VK_OBJECT_TYPE_DEVICE: return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
        case VK_OBJECT_TYPE_QUEUE: return VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
        case VK_OBJECT_TYPE_SEMAPHORE: return VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT;
        case VK_OBJECT_TYPE_COMMAND_BUFFER: return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
        case VK_OBJECT_TYPE_FENCE: return VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT;
        case VK_OBJECT_TYPE_DEVICE_MEMORY: return VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
        case VK_OBJECT_TYPE_BUFFER: return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
        case VK_OBJECT_TYPE_IMAGE: return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
        case VK_OBJECT_TYPE_EVENT: return VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
        case VK_OBJECT_TYPE_QUERY_POOL: return VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT;
        case VK_OBJECT_TYPE_BUFFER_VIEW: return VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT;
        case VK_OBJECT_TYPE_IMAGE_VIEW: return VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT;
        case VK_OBJECT_TYPE_SHADER_MODULE: return VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
        case VK_OBJECT_TYPE_PIPELINE_CACHE: return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT;
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT;
        case VK_OBJECT_TYPE_RENDER_PASS: return VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT;
        case VK_OBJECT_TYPE_PIPELINE: return VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT;
        case VK_OBJECT_TYPE_SAMPLER: return VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_SET: return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT;
        case VK_OBJECT_TYPE_FRAMEBUFFER: return VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT;
        case VK_OBJECT_TYPE_COMMAND_POOL: return VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT;
        case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT;
        case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT;
        case VK_OBJECT_TYPE_SURFACE_KHR: return VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
        case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
        case VK_OBJECT_TYPE_DISPLAY_KHR: return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT;
        case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT;
        case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT;
        case VK_OBJECT_TYPE_CU_MODULE_NVX: return VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT;
        case VK_OBJECT_TYPE_CU_FUNCTION_NVX: return VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: return VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT;
        case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: return VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT;
        case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: return VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT;
        default: return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    }
}

// Traits objects from each type statically map from Vk<handleType> to the various enums
template <typename VkType> struct VkHandleInfo {};
template <VulkanObjectType id> struct VulkanObjectTypeInfo {};

// The following line must match the vulkan_core.h condition guarding VK_DEFINE_NON_DISPATCHABLE_HANDLE
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) ||                 defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define TYPESAFE_NONDISPATCHABLE_HANDLES
#else
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkNonDispatchableHandle)

template <> struct VkHandleInfo<VkNonDispatchableHandle> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeUnknown;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_UNKNOWN;
    static const char* Typename() {
        return "VkNonDispatchableHandle";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeUnknown> {
    typedef VkNonDispatchableHandle Type;
};

#endif //  VK_DEFINE_HANDLE logic duplication
template <> struct VkHandleInfo<VkCommandBuffer> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeCommandBuffer;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
    static const char* Typename() {
        return "VkCommandBuffer";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeCommandBuffer> {
    typedef VkCommandBuffer Type;
};
template <> struct VkHandleInfo<VkDevice> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDevice;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DEVICE;
    static const char* Typename() {
        return "VkDevice";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDevice> {
    typedef VkDevice Type;
};
template <> struct VkHandleInfo<VkInstance> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeInstance;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_INSTANCE;
    static const char* Typename() {
        return "VkInstance";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeInstance> {
    typedef VkInstance Type;
};
template <> struct VkHandleInfo<VkPhysicalDevice> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypePhysicalDevice;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_PHYSICAL_DEVICE;
    static const char* Typename() {
        return "VkPhysicalDevice";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypePhysicalDevice> {
    typedef VkPhysicalDevice Type;
};
template <> struct VkHandleInfo<VkQueue> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeQueue;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_QUEUE;
    static const char* Typename() {
        return "VkQueue";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeQueue> {
    typedef VkQueue Type;
};
#ifdef TYPESAFE_NONDISPATCHABLE_HANDLES
template <> struct VkHandleInfo<VkAccelerationStructureKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeAccelerationStructureKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
    static const char* Typename() {
        return "VkAccelerationStructureKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeAccelerationStructureKHR> {
    typedef VkAccelerationStructureKHR Type;
};
template <> struct VkHandleInfo<VkAccelerationStructureNV> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeAccelerationStructureNV;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV;
    static const char* Typename() {
        return "VkAccelerationStructureNV";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeAccelerationStructureNV> {
    typedef VkAccelerationStructureNV Type;
};
template <> struct VkHandleInfo<VkBuffer> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeBuffer;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_BUFFER;
    static const char* Typename() {
        return "VkBuffer";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeBuffer> {
    typedef VkBuffer Type;
};
template <> struct VkHandleInfo<VkBufferView> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeBufferView;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_BUFFER_VIEW;
    static const char* Typename() {
        return "VkBufferView";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeBufferView> {
    typedef VkBufferView Type;
};
template <> struct VkHandleInfo<VkCommandPool> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeCommandPool;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_COMMAND_POOL;
    static const char* Typename() {
        return "VkCommandPool";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeCommandPool> {
    typedef VkCommandPool Type;
};
template <> struct VkHandleInfo<VkCuFunctionNVX> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeCuFunctionNVX;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_CU_FUNCTION_NVX;
    static const char* Typename() {
        return "VkCuFunctionNVX";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeCuFunctionNVX> {
    typedef VkCuFunctionNVX Type;
};
template <> struct VkHandleInfo<VkCuModuleNVX> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeCuModuleNVX;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_CU_MODULE_NVX;
    static const char* Typename() {
        return "VkCuModuleNVX";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeCuModuleNVX> {
    typedef VkCuModuleNVX Type;
};
template <> struct VkHandleInfo<VkDebugReportCallbackEXT> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDebugReportCallbackEXT;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
    static const char* Typename() {
        return "VkDebugReportCallbackEXT";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDebugReportCallbackEXT> {
    typedef VkDebugReportCallbackEXT Type;
};
template <> struct VkHandleInfo<VkDebugUtilsMessengerEXT> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDebugUtilsMessengerEXT;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT;
    static const char* Typename() {
        return "VkDebugUtilsMessengerEXT";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDebugUtilsMessengerEXT> {
    typedef VkDebugUtilsMessengerEXT Type;
};
template <> struct VkHandleInfo<VkDeferredOperationKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDeferredOperationKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR;
    static const char* Typename() {
        return "VkDeferredOperationKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDeferredOperationKHR> {
    typedef VkDeferredOperationKHR Type;
};
template <> struct VkHandleInfo<VkDescriptorPool> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDescriptorPool;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DESCRIPTOR_POOL;
    static const char* Typename() {
        return "VkDescriptorPool";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDescriptorPool> {
    typedef VkDescriptorPool Type;
};
template <> struct VkHandleInfo<VkDescriptorSet> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDescriptorSet;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DESCRIPTOR_SET;
    static const char* Typename() {
        return "VkDescriptorSet";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDescriptorSet> {
    typedef VkDescriptorSet Type;
};
template <> struct VkHandleInfo<VkDescriptorSetLayout> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDescriptorSetLayout;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
    static const char* Typename() {
        return "VkDescriptorSetLayout";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDescriptorSetLayout> {
    typedef VkDescriptorSetLayout Type;
};
template <> struct VkHandleInfo<VkDescriptorUpdateTemplate> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDescriptorUpdateTemplate;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
    static const char* Typename() {
        return "VkDescriptorUpdateTemplate";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDescriptorUpdateTemplate> {
    typedef VkDescriptorUpdateTemplate Type;
};
template <> struct VkHandleInfo<VkDeviceMemory> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDeviceMemory;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DEVICE_MEMORY;
    static const char* Typename() {
        return "VkDeviceMemory";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDeviceMemory> {
    typedef VkDeviceMemory Type;
};
template <> struct VkHandleInfo<VkDisplayKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDisplayKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DISPLAY_KHR;
    static const char* Typename() {
        return "VkDisplayKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDisplayKHR> {
    typedef VkDisplayKHR Type;
};
template <> struct VkHandleInfo<VkDisplayModeKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeDisplayModeKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
    static const char* Typename() {
        return "VkDisplayModeKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeDisplayModeKHR> {
    typedef VkDisplayModeKHR Type;
};
template <> struct VkHandleInfo<VkEvent> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeEvent;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_EVENT;
    static const char* Typename() {
        return "VkEvent";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeEvent> {
    typedef VkEvent Type;
};
template <> struct VkHandleInfo<VkFence> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeFence;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_FENCE;
    static const char* Typename() {
        return "VkFence";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeFence> {
    typedef VkFence Type;
};
template <> struct VkHandleInfo<VkFramebuffer> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeFramebuffer;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_FRAMEBUFFER;
    static const char* Typename() {
        return "VkFramebuffer";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeFramebuffer> {
    typedef VkFramebuffer Type;
};
template <> struct VkHandleInfo<VkImage> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeImage;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_IMAGE;
    static const char* Typename() {
        return "VkImage";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeImage> {
    typedef VkImage Type;
};
template <> struct VkHandleInfo<VkImageView> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeImageView;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_IMAGE_VIEW;
    static const char* Typename() {
        return "VkImageView";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeImageView> {
    typedef VkImageView Type;
};
template <> struct VkHandleInfo<VkIndirectCommandsLayoutNV> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeIndirectCommandsLayoutNV;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV;
    static const char* Typename() {
        return "VkIndirectCommandsLayoutNV";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeIndirectCommandsLayoutNV> {
    typedef VkIndirectCommandsLayoutNV Type;
};
template <> struct VkHandleInfo<VkPerformanceConfigurationINTEL> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypePerformanceConfigurationINTEL;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL;
    static const char* Typename() {
        return "VkPerformanceConfigurationINTEL";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypePerformanceConfigurationINTEL> {
    typedef VkPerformanceConfigurationINTEL Type;
};
template <> struct VkHandleInfo<VkPipeline> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypePipeline;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_PIPELINE;
    static const char* Typename() {
        return "VkPipeline";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypePipeline> {
    typedef VkPipeline Type;
};
template <> struct VkHandleInfo<VkPipelineCache> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypePipelineCache;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_PIPELINE_CACHE;
    static const char* Typename() {
        return "VkPipelineCache";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypePipelineCache> {
    typedef VkPipelineCache Type;
};
template <> struct VkHandleInfo<VkPipelineLayout> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypePipelineLayout;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_PIPELINE_LAYOUT;
    static const char* Typename() {
        return "VkPipelineLayout";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypePipelineLayout> {
    typedef VkPipelineLayout Type;
};
template <> struct VkHandleInfo<VkPrivateDataSlotEXT> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypePrivateDataSlotEXT;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT;
    static const char* Typename() {
        return "VkPrivateDataSlotEXT";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypePrivateDataSlotEXT> {
    typedef VkPrivateDataSlotEXT Type;
};
template <> struct VkHandleInfo<VkQueryPool> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeQueryPool;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_QUERY_POOL;
    static const char* Typename() {
        return "VkQueryPool";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeQueryPool> {
    typedef VkQueryPool Type;
};
template <> struct VkHandleInfo<VkRenderPass> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeRenderPass;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_RENDER_PASS;
    static const char* Typename() {
        return "VkRenderPass";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeRenderPass> {
    typedef VkRenderPass Type;
};
template <> struct VkHandleInfo<VkSampler> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeSampler;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_SAMPLER;
    static const char* Typename() {
        return "VkSampler";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeSampler> {
    typedef VkSampler Type;
};
template <> struct VkHandleInfo<VkSamplerYcbcrConversion> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeSamplerYcbcrConversion;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
    static const char* Typename() {
        return "VkSamplerYcbcrConversion";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeSamplerYcbcrConversion> {
    typedef VkSamplerYcbcrConversion Type;
};
template <> struct VkHandleInfo<VkSemaphore> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeSemaphore;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_SEMAPHORE;
    static const char* Typename() {
        return "VkSemaphore";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeSemaphore> {
    typedef VkSemaphore Type;
};
template <> struct VkHandleInfo<VkShaderModule> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeShaderModule;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_SHADER_MODULE;
    static const char* Typename() {
        return "VkShaderModule";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeShaderModule> {
    typedef VkShaderModule Type;
};
template <> struct VkHandleInfo<VkSurfaceKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeSurfaceKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_SURFACE_KHR;
    static const char* Typename() {
        return "VkSurfaceKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeSurfaceKHR> {
    typedef VkSurfaceKHR Type;
};
template <> struct VkHandleInfo<VkSwapchainKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeSwapchainKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR;
    static const char* Typename() {
        return "VkSwapchainKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeSwapchainKHR> {
    typedef VkSwapchainKHR Type;
};
template <> struct VkHandleInfo<VkValidationCacheEXT> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeValidationCacheEXT;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
    static const char* Typename() {
        return "VkValidationCacheEXT";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeValidationCacheEXT> {
    typedef VkValidationCacheEXT Type;
};
#ifdef VK_ENABLE_BETA_EXTENSIONS
template <> struct VkHandleInfo<VkVideoSessionKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeVideoSessionKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_VIDEO_SESSION_KHR;
    static const char* Typename() {
        return "VkVideoSessionKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeVideoSessionKHR> {
    typedef VkVideoSessionKHR Type;
};
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
template <> struct VkHandleInfo<VkVideoSessionParametersKHR> {
    static const VulkanObjectType kVulkanObjectType = kVulkanObjectTypeVideoSessionParametersKHR;
    static const VkDebugReportObjectTypeEXT kDebugReportObjectType = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
    static const VkObjectType kVkObjectType = VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR;
    static const char* Typename() {
        return "VkVideoSessionParametersKHR";
    }
};
template <> struct VulkanObjectTypeInfo<kVulkanObjectTypeVideoSessionParametersKHR> {
    typedef VkVideoSessionParametersKHR Type;
};
#endif
#endif // TYPESAFE_NONDISPATCHABLE_HANDLES
struct VulkanTypedHandle {
    uint64_t handle;
    VulkanObjectType type;
    // node is optional, and if non-NULL is used to avoid a hash table lookup
    class BASE_NODE *node;
    template <typename Handle>
    VulkanTypedHandle(Handle handle_, VulkanObjectType type_, class BASE_NODE *node_ = nullptr) :
        handle(CastToUint64(handle_)),
        type(type_),
        node(node_) {
#ifdef TYPESAFE_NONDISPATCHABLE_HANDLES
        // For 32 bit it's not always safe to check for traits <-> type
        // as all non-dispatchable handles have the same type-id and thus traits,
        // but on 64 bit we can validate the passed type matches the passed handle
        assert(type == VkHandleInfo<Handle>::kVulkanObjectType);
#endif // TYPESAFE_NONDISPATCHABLE_HANDLES
    }
    template <typename Handle>
    Handle Cast() const {
#ifdef TYPESAFE_NONDISPATCHABLE_HANDLES
        assert(type == VkHandleInfo<Handle>::kVulkanObjectType);
#endif // TYPESAFE_NONDISPATCHABLE_HANDLES
        return CastFromUint64<Handle>(handle);
    }
    VulkanTypedHandle() :
        handle(CastToUint64(VK_NULL_HANDLE)),
        type(kVulkanObjectTypeUnknown),
        node(nullptr) {}
};

