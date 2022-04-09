/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "../layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

TEST_F(VkPositiveLayerTest, DynamicRenderingDraw) {
    TEST_DESCRIPTION("Draw with Dynamic Rendering.");

    m_errorMonitor->ExpectSuccess();

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    char const* fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        void main() {
           color = vec4(1.0f);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.pNext = &pipeline_rendering_info;

    pipe.CreateVKPipeline(pl.handle(), VK_NULL_HANDLE, &create_info);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->ExpectSuccess();
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, CmdClearAttachmentTestsDynamicRendering) {
    TEST_DESCRIPTION("Various tests for validating usage of vkCmdClearAttachments with Dynamic Rendering");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();

    // Main thing we care about for this test is that the VkImage obj we're
    // clearing matches Color Attachment of FB
    //  Also pass down other dummy params to keep driver and paramchecker happy
    VkClearAttachment color_attachment;
    color_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_attachment.clearValue.color.float32[0] = 1.0;
    color_attachment.clearValue.color.float32[1] = 1.0;
    color_attachment.clearValue.color.float32[2] = 1.0;
    color_attachment.clearValue.color.float32[3] = 1.0;
    color_attachment.colorAttachment = 0;
    VkClearRect clear_rect = {{{0, 0}, {(uint32_t)m_width, (uint32_t)m_height}}, 0, 1};

    clear_rect.rect.extent.width = renderPassBeginInfo().renderArea.extent.width + 4;
    clear_rect.rect.extent.height = clear_rect.rect.extent.height / 2;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.renderArea = clear_rect.rect;
    begin_rendering_info.layerCount = 1;
    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);

    // baseLayer >= view layers
    clear_rect.rect.extent.width = (uint32_t)m_width;
    clear_rect.baseArrayLayer = 1;
    clear_rect.layerCount = 1;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);

    // baseLayer + layerCount > view layers
    clear_rect.rect.extent.width = (uint32_t)m_width;
    clear_rect.baseArrayLayer = 0;
    clear_rect.layerCount = 2;
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &color_attachment, 1, &clear_rect);

    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, TestBeginQueryInDynamicRendering) {
    TEST_DESCRIPTION("Test calling vkCmdBeginQuery with a dynamic render pass.");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        printf("%s Tests requires Vulkan 1.3+, skipping test\n", kSkipPrefix);
        return;
    }

    auto vk13features = LvlInitStruct<VkPhysicalDeviceVulkan13Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vk13features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!vk13features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering, skipping\n", kSkipPrefix);
        return;
    }
    if (!vk13features.synchronization2) {
        printf("%s Test requires (unsupported) synchronization2, skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto vkCmdBeginRendering =
        reinterpret_cast<PFN_vkCmdBeginRendering>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginRendering"));
    assert(vkCmdBeginRendering != nullptr);
    auto vkCmdEndRendering =
        reinterpret_cast<PFN_vkCmdEndRendering>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndRendering"));
    assert(vkCmdEndRendering != nullptr);

    m_errorMonitor->ExpectSuccess();

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.layerCount = 1;

    VkQueryPoolCreateInfo qpci = LvlInitStruct<VkQueryPoolCreateInfo>();
    qpci.queryType = VK_QUERY_TYPE_OCCLUSION;
    qpci.queryCount = 2;

    vk_testing::QueryPool query_pool;
    query_pool.init(*m_device, qpci);

    m_commandBuffer->begin();

    vkCmdBeginRendering(m_commandBuffer->handle(), &begin_rendering_info);
    vk::CmdBeginQuery(m_commandBuffer->handle(), query_pool.handle(), 0, 0);
    vk::CmdEndQuery(m_commandBuffer->handle(), query_pool.handle(), 0);
    vkCmdEndRendering(m_commandBuffer->handle());

    m_commandBuffer->end();

    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkPositiveLayerTest, DynamicRenderingPipeWithDiscard) {
    TEST_DESCRIPTION("Create dynamic rendering pipeline with rasterizer discard.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Tests requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->ExpectSuccess();

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineRasterizationStateCreateInfo rs_ci = LvlInitStruct<VkPipelineRasterizationStateCreateInfo>();
    rs_ci.rasterizerDiscardEnable = VK_TRUE;
    rs_ci.lineWidth = 1.0f;

    VkPipelineDepthStencilStateCreateInfo ds_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.SetDepthStencil(&ds_ci);
    pipe.SetRasterization(&rs_ci);

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    auto create_info = LvlInitStruct<VkGraphicsPipelineCreateInfo>();
    pipe.InitGraphicsPipelineCreateInfo(&create_info);
    create_info.layout = pl.handle();
    create_info.renderPass = VK_NULL_HANDLE;

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};
    auto pipeline_rendering_info = LvlInitStruct<VkPipelineRenderingCreateInfoKHR>();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;
    pipeline_rendering_info.depthAttachmentFormat = VK_FORMAT_D16_UNORM;  // D16_UNORM has guaranteed support
    pipeline_rendering_info.pNext = create_info.pNext;
    create_info.pNext = &pipeline_rendering_info;

    const VkRenderPass render_pass = VK_NULL_HANDLE;
    pipe.CreateVKPipeline(pl.handle(), render_pass, &create_info);

    m_errorMonitor->VerifyNotFound();
}
