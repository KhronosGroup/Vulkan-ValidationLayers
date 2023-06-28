/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "vulkan_object.h"
#include <vector>
#include <iostream>

#include "generated/vk_function_pointers.h"
#include "generated/vk_typemap_helper.h"

// Extensions not allowed to use if promoted
// TODO - Generate these
#include <array>
#include <string.h>
static std::array<const char *, 3> ignore_extensions{
    "VK_AMD_negative_viewport_height",
    "VK_EXT_buffer_device_address",
    "VK_KHR_portability_subset",
};

static bool IsExtensionForPromoted(const char *vk_extension) {
    for (const auto &extension : ignore_extensions) {
        if (strcmp(extension, vk_extension) == 0) {
            return true;
        }
    }
    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
                                                          const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                          void *is_valid) {
    // Allow us to catch some VUs that are really not a 'failure'
    if (
        // These are the fragmentDensityMap feature VUs (VUID-VkDeviceCreateInfo-fragmentDensityMap-*)
        // We need to enable the EXT and KHR extensions as there are 2 seperate capabilities we might find
        (callback_data->messageIdNumber == static_cast<int32_t>(0x8dec816a)) ||
        (callback_data->messageIdNumber == static_cast<int32_t>(0x449ecae8)) ||
        (callback_data->messageIdNumber == static_cast<int32_t>(0x6baba20a)) ||
        // geom/tessellation shaders might not be writting to a PointSize
        // even though shaderTessellationAndGeometryPointSize is set
        (callback_data->messageIdNumber == static_cast<int32_t>(0x64e29d24))) {
        // Ignored
    } else if (callback_data->messageIdNumber == static_cast<int32_t>(0x4e2c336f)) {
        // TODO - When parsing Geometry shaders, the BuiltIn block might not be the default
        // block used in the Vertex Shader
        std::cerr << callback_data->pMessage << "\n";
    } else if (callback_data->messageIdNumber == static_cast<int32_t>(0x2a1bf17f)) {
        // spirv-val failed error message, don't quit, issue with SPIR-V or spirv-val
        // Usually the SPIR-V is just missing the OpCapability for what it is using
        std::cerr << callback_data->pMessage << "\n";
    } else {
        std::cerr << callback_data->pMessage << "\n";
        *(static_cast<bool *>(is_valid)) = false;
    }
    return VK_FALSE;
}

VulkanInstance::VulkanInstance() {
    // Instance Creation
    vk::InitCore("vulkan");
    {
        auto debug_utils_create_info = LvlInitStruct<VkDebugUtilsMessengerCreateInfoEXT>();
        debug_utils_create_info.flags = 0;
        debug_utils_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_utils_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debug_utils_create_info.pfnUserCallback = DebugUtilMessengerCallback;
        debug_utils_create_info.pUserData = &is_valid;

        std::vector<const char *> instance_extensions;
        uint32_t count = 0;
        vk::EnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vk::EnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
        for (uint32_t i = 0; i < count; i++) {
            instance_extensions.push_back(extensions[i].extensionName);
        }

        VkApplicationInfo app_info;
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;
        // Use latest version of Vulkan incase the shader use newest SPIR-V version
        app_info.apiVersion = VK_API_VERSION_1_3;
        app_info.pApplicationName = "SPIRV-Hopper";
        app_info.applicationVersion = 1;
        app_info.pEngineName = "SPIRV-Hopper";
        app_info.engineVersion = 1;

        VkInstanceCreateInfo instance_info;
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pNext = &debug_utils_create_info;
        instance_info.flags = 0;
        instance_info.pApplicationInfo = &app_info;
        instance_info.enabledLayerCount = 0;
        instance_info.ppEnabledLayerNames = nullptr;
        instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
        instance_info.ppEnabledExtensionNames = instance_extensions.data();
        vk::CreateInstance(&instance_info, nullptr, &instance);

        auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vk::GetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        vkCreateDebugUtilsMessengerEXT(instance, &debug_utils_create_info, nullptr, &debug_utils_messenger);
    }

    // Physical Device
    {
        // Grab first GPU
        uint32_t count = 0;
        vk::EnumeratePhysicalDevices(instance, &count, nullptr);
        std::vector<VkPhysicalDevice> physical_devices(count);
        vk::EnumeratePhysicalDevices(instance, &count, physical_devices.data());
        gpu = physical_devices[0];

        vk::GetPhysicalDeviceProperties(gpu, &properties);
        vk::GetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
    }

    // Query so downstrem layers don't blow up
    {
        uint32_t count = 0;
        vk::GetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(count);
        vk::GetPhysicalDeviceQueueFamilyProperties(gpu, &count, queue_families.data());
    }

    // Logical Device Creation
    {
        std::vector<const char *> device_extensions;
        std::vector<VkExtensionProperties> extensions;
        {
            uint32_t count = 0;
            vk::EnumerateDeviceExtensionProperties(gpu, nullptr, &count, nullptr);
            extensions.resize(count);
            vk::EnumerateDeviceExtensionProperties(gpu, nullptr, &count, extensions.data());
            for (uint32_t i = 0; i < count; i++) {
                // TODO - Generate these
                if (!IsExtensionForPromoted(extensions[i].extensionName)) {
                    device_extensions.push_back(extensions[i].extensionName);
                }
            }
        }

        // TODO - Generate these
        // Currently assumes using MockICD and profiles so all of these are enabled
        auto physical_device_mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesEXT>();
        auto physical_device_ray_tracing_pipeline_features =
            LvlInitStruct<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(&physical_device_mesh_shader_features);
        auto physical_device_fragment_shader_interlock_features =
            LvlInitStruct<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>(&physical_device_ray_tracing_pipeline_features);
        auto physical_device_shader_atomic_float_features =
            LvlInitStruct<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(&physical_device_fragment_shader_interlock_features);
        auto physical_device_shader_atomic_float2_features =
            LvlInitStruct<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(&physical_device_shader_atomic_float_features);
        auto physical_device_workgroup_memory_explicit_layout_features =
            LvlInitStruct<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>(&physical_device_shader_atomic_float2_features);
        auto physical_device_ray_query_features =
            LvlInitStruct<VkPhysicalDeviceRayQueryFeaturesKHR>(&physical_device_workgroup_memory_explicit_layout_features);
        auto physical_device_ray_tracing_maintenance1_features =
            LvlInitStruct<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>(&physical_device_ray_query_features);
        auto physical_device_fragment_shading_rate_features =
            LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(&physical_device_ray_tracing_maintenance1_features);
        auto physical_device_fragment_shader_barycentric_features =
            LvlInitStruct<VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR>(&physical_device_fragment_shading_rate_features);
        auto physical_device_transform_feedback_features =
            LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(&physical_device_fragment_shader_barycentric_features);
        auto physical_device_shader_image_atomic_int64_features =
            LvlInitStruct<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(&physical_device_transform_feedback_features);
        auto physical_device_fragment_density_map_features =
            LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(&physical_device_shader_image_atomic_int64_features);
        auto physical_device_vulkan11_features =
            LvlInitStruct<VkPhysicalDeviceVulkan11Features>(&physical_device_fragment_density_map_features);
        auto physical_device_vulkan12_features =
            LvlInitStruct<VkPhysicalDeviceVulkan12Features>(&physical_device_vulkan11_features);
        auto physical_device_vulkan13_features =
            LvlInitStruct<VkPhysicalDeviceVulkan13Features>(&physical_device_vulkan12_features);
        auto physical_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&physical_device_vulkan13_features);
        vk::GetPhysicalDeviceFeatures2(gpu, &physical_features2);

        // Don't actually need a queue, so just grab the first one
        auto queue_info = LvlInitStruct<VkDeviceQueueCreateInfo>();
        queue_info.flags = 0;
        queue_info.queueFamilyIndex = 0;
        queue_info.queueCount = 1;
        float queue_priority = 1.0f;
        queue_info.pQueuePriorities = &queue_priority;

        auto device_info = LvlInitStruct<VkDeviceCreateInfo>(&physical_features2);
        device_info.flags = 0;
        device_info.pQueueCreateInfos = &queue_info;
        device_info.queueCreateInfoCount = 1;
        device_info.enabledLayerCount = 0;
        device_info.ppEnabledLayerNames = nullptr;
        device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        device_info.ppEnabledExtensionNames = device_extensions.data();
        device_info.pEnabledFeatures = nullptr;

        vk::CreateDevice(gpu, &device_info, nullptr, &device);
    }
}

VulkanInstance::~VulkanInstance() {
    if (device != VK_NULL_HANDLE) {
        vk::DestroyDevice(device, nullptr);
    }
    if (debug_utils_messenger != VK_NULL_HANDLE) {
        auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vk::GetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        vkDestroyDebugUtilsMessengerEXT(instance, debug_utils_messenger, nullptr);
    }
    if (instance != VK_NULL_HANDLE) {
        vk::DestroyInstance(instance, nullptr);
    }
}