/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/render_pass_helper.h"
#include "generated/vk_extension_helper.h"

void DynamicRenderingTest::InitBasicDynamicRenderingLocalRead() {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::dynamicRenderingLocalRead);
    RETURN_IF_SKIP(Init());
}

TEST_F(PositiveDynamicRenderingLocalRead, BasicUsage) {
    TEST_DESCRIPTION("Most simple way to use dynamic rendering local read");
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDynamicRenderingLocalRead());

    vkt::Image image1(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    vkt::Image image2(*m_device, 32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM,
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    CreatePipelineHelper pipe1(*this);
    CreatePipelineHelper pipe2(*this);
    for (uint32_t i = 0; i < 2; i++)
    {
        CreatePipelineHelper* pipe = (i == 0) ? &pipe1 : &pipe2;
        VkFormat color_formats[] = {VK_FORMAT_UNDEFINED, VK_FORMAT_UNDEFINED};

        // Images mapped differently in pipe1 and pipe2
        uint32_t locations[] = {i, 1 - i};
        uint32_t inputs[] = {i, 1 - i};

        VkRenderingInputAttachmentIndexInfoKHR inputs_info = vku::InitStructHelper();
        inputs_info.colorAttachmentCount = 2;
        inputs_info.pColorAttachmentInputIndices = inputs;

        VkRenderingAttachmentLocationInfoKHR locations_info = vku::InitStructHelper(&inputs_info);
        locations_info.colorAttachmentCount = 2;
        locations_info.pColorAttachmentLocations = locations;

        VkPipelineRenderingCreateInfoKHR pipeline_rendering_info = vku::InitStructHelper(&locations_info);
        pipeline_rendering_info.colorAttachmentCount = 2;
        pipeline_rendering_info.pColorAttachmentFormats = color_formats;

        VkPipelineColorBlendAttachmentState cb_attachments[2];
        memset(cb_attachments, 0, sizeof(VkPipelineColorBlendAttachmentState) * 2);
        pipe->ds_ci_ = vku::InitStructHelper();
        pipe->gp_ci_.pNext = &pipeline_rendering_info;
        pipe->cb_ci_.attachmentCount = 2;
        pipe->cb_ci_.pAttachments = cb_attachments;
        pipe->CreateGraphicsPipeline();
    }

    VkRenderingAttachmentInfoKHR color_attachments[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    color_attachments[0].imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
    color_attachments[1].imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 2;
    begin_rendering_info.pColorAttachments = &color_attachments[0];
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_commandBuffer->begin();

    VkImageMemoryBarrier pre_barriers[2] = {vku::InitStructHelper(), vku::InitStructHelper()};
    pre_barriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pre_barriers[0].srcAccessMask = 0;
    pre_barriers[0].newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
    pre_barriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    pre_barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    pre_barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    pre_barriers[0].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};
    pre_barriers[1] = pre_barriers[0];
    pre_barriers[0].image = image1.handle();
    pre_barriers[1].image = image2.handle();

    VkImageMemoryBarrier post_barriers[2] = {pre_barriers[0], pre_barriers[1]};

    post_barriers[0].oldLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
    post_barriers[0].newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
    post_barriers[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    post_barriers[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    post_barriers[1].oldLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
    post_barriers[1].newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
    post_barriers[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    post_barriers[1].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

    vk::CmdPipelineBarrier(*m_commandBuffer, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2, &pre_barriers[0]);

    m_commandBuffer->BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.Handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdPipelineBarrier(*m_commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2, &post_barriers[0]);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());

    uint32_t locations[] = {1, 0};
    VkRenderingAttachmentLocationInfoKHR location_info = vku::InitStructHelper();
    location_info.colorAttachmentCount = 2;
    location_info.pColorAttachmentLocations = locations;
    vk::CmdSetRenderingAttachmentLocationsKHR(m_commandBuffer->handle(), &location_info);

    VkRenderingInputAttachmentIndexInfoKHR input_info = vku::InitStructHelper();
    input_info.colorAttachmentCount = 2;
    input_info.pColorAttachmentInputIndices = locations;
    vk::CmdSetRenderingInputAttachmentIndicesKHR(m_commandBuffer->handle(), &input_info);

    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}
