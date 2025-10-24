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

void LegacyTest::CreateRenderPass() {
    vkt::Image image(*m_device, 8, 8, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    VkAttachmentReference attachment_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attach_desc;

    vkt::RenderPass rp(*m_device, rp_ci);
}

class PositiveLegacy : public LegacyTest {};

TEST_F(PositiveLegacy, SettingExplicitOff) {
    TEST_DESCRIPTION("Turn off setting explicitly");
    const VkLayerSettingEXT layer_setting = {OBJECT_LAYER_NAME, "legacy_detection", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                             &kVkFalse};
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&layer_setting_ci));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    CreateRenderPass();
}

TEST_F(PositiveLegacy, SettingDefault) {
    TEST_DESCRIPTION("Make sure default settings have no warning");
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);
    CreateRenderPass();
}

TEST_F(PositiveLegacy, MuteMultipleWarnings) {
    const char *ids[] = {"WARNING-legacy-gpdp2", "WARNING-legacy-renderpass2", "WARNING-legacy-dynamicrendering"};
    VkLayerSettingEXT layer_settings[3] = {
        {OBJECT_LAYER_NAME, "legacy_detection", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
        {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 3, ids}};
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, layer_settings};

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework(&layer_setting_ci));
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;

    // ignores WARNING-legacy-renderpass2
    vkt::RenderPass render_pass(*m_device, rp_ci);

    // ignores WARNING-legacy-dynamicrendering
    vkt::Framebuffer framebuffer(*m_device, render_pass, 0, nullptr);
}

TEST_F(PositiveLegacy, GetPhysicalDeviceProperties2Extension) {
    TEST_DESCRIPTION("Show Instance extensions currently only detected if enabled, not if supported");
    const VkLayerSettingEXT layer_setting = {OBJECT_LAYER_NAME, "legacy_detection", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                             &kVkTrue};
    VkLayerSettingsCreateInfoEXT layer_setting_ci = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting};

    RETURN_IF_SKIP(InitFramework(&layer_setting_ci));
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    VkPhysicalDeviceFeatures features{};
    vk::GetPhysicalDeviceFeatures(Gpu(), &features);

    VkFormatProperties format_properties{};
    vk::GetPhysicalDeviceFormatProperties(Gpu(), VK_FORMAT_R8G8B8A8_UNORM, &format_properties);
}