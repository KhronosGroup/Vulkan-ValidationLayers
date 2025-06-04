/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
// stype-check off
#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"

static const VkLayerSettingEXT kDeprecationLayerSetting = {OBJECT_LAYER_NAME, "deprecation", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                                           &kVkTrue};
static VkLayerSettingsCreateInfoEXT kDeprecationLayerSettingCreateInfo = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                          1, &kDeprecationLayerSetting};

class NegativeDeprecation : public DeprecationTest {};

TEST_F(NegativeDeprecation, RenderPass2Extension) {
    RETURN_IF_SKIP(InitFramework(&kDeprecationLayerSettingCreateInfo));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    if (!DeviceExtensionSupported(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "required extension not supported";
    }

    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-renderpass2");
    CreateRenderPass();
    m_errorMonitor->VerifyFound();

    // Only report the first time
    CreateRenderPass();
}

TEST_F(NegativeDeprecation, MultipleDifferentWarnings) {
    RETURN_IF_SKIP(InitFramework(&kDeprecationLayerSettingCreateInfo));
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Works locally
        GTEST_SKIP() << "Github Action doesn't unload VVL and static reported bool is not reset";
    }
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    if (!DeviceExtensionSupported(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) ||
        !DeviceExtensionSupported(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME)) {
        GTEST_SKIP() << "required extension not supported";
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;

    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-renderpass2");
    vkt::RenderPass render_pass(*m_device, rp_ci);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-dynamicrendering");
    vkt::Framebuffer framebuffer(*m_device, render_pass, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeprecation, MuteSingleWarning) {
    TEST_DESCRIPTION("Only mute of the two warnings to make sure mutting works");

    const char *ids[] = {"WARNING-deprecation-renderpass2"};
    VkLayerSettingEXT layer_settings[2] = {kDeprecationLayerSetting,
                                           {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids}};

    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};

    RETURN_IF_SKIP(InitFramework(&layer_setting_ci));
    RETURN_IF_SKIP(InitState());
    if (IsPlatformMockICD()) {
        // Works locally
        GTEST_SKIP() << "Github Action doesn't unload VVL and static reported bool is not reset";
    }
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    if (!DeviceExtensionSupported(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) ||
        !DeviceExtensionSupported(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME)) {
        GTEST_SKIP() << "required extension not supported";
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;

    // ignores WARNING-deprecation-renderpass2
    vkt::RenderPass render_pass(*m_device, rp_ci);

    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-dynamicrendering");
    vkt::Framebuffer framebuffer(*m_device, render_pass, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeprecation, GetPhysicalDeviceProperties2Extension) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&kDeprecationLayerSettingCreateInfo));
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-gpdp2");
    VkPhysicalDeviceFeatures features{};
    vk::GetPhysicalDeviceFeatures(Gpu(), &features);
    m_errorMonitor->VerifyFound();

    VkFormatProperties format_properties{};
    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-gpdp2");
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, &format_properties);
    m_errorMonitor->VerifyFound();

    // Only report the first time
    vk::GetPhysicalDeviceFeatures(Gpu(), &features);
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, &format_properties);
}

TEST_F(NegativeDeprecation, GetPhysicalDeviceProperties2Version) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework(&kDeprecationLayerSettingCreateInfo));
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    if (IsPlatformMockICD()) {
        // Works locally
        GTEST_SKIP() << "Github Action doesn't unload VVL and static reported bool is not reset";
    }
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GTEST_SKIP() << "Android doesn't unload VVL and static reported bool is not reset";
#endif

    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-gpdp2");
    VkPhysicalDeviceFeatures features{};
    vk::GetPhysicalDeviceFeatures(Gpu(), &features);
    m_errorMonitor->VerifyFound();

    VkFormatProperties format_properties{};
    m_errorMonitor->SetDesiredWarning("WARNING-deprecation-gpdp2");
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, &format_properties);
    m_errorMonitor->VerifyFound();
}
