/*
 * Copyright (c) 2026 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveDeviceAddressCommands : public DeviceAddressCommands {};

TEST_F(PositiveDeviceAddressCommands, CopyMemoryToImage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_GENERAL);
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDeviceAddressCommands, CopyImageToMemory) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_GENERAL);
    vk::CmdCopyImageToMemoryKHR(m_command_buffer, &copy_memory_info);
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDeviceAddressCommands, CmdFillMemory) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_command_buffer.End();
}

TEST_F(PositiveDeviceAddressCommands, CmdUpdateMemory) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();
    vk::CmdUpdateMemoryKHR(m_command_buffer, &range, flags, sizeof(data), &data);
    m_command_buffer.End();
}

TEST_F(PositiveDeviceAddressCommands, VertexBindingDifferentFunctions) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    const char* vs_source = R"glsl(
        #version 450
        layout(location=0) in vec4 x;
        void main(){}
    )glsl";
    VkShaderObj vs(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    VkVertexInputBindingDescription input_binding = {0, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32_SFLOAT, 0};

    CreatePipelineHelper pipe(*this);
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &attribute;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    CreatePipelineHelper dynamic_pipe(*this);
    dynamic_pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    dynamic_pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    dynamic_pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    dynamic_pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    dynamic_pipe.vi_ci_.pVertexAttributeDescriptions = &attribute;
    dynamic_pipe.shader_stages_ = {vs.GetStageCreateInfo(), dynamic_pipe.fs_->GetStageCreateInfo()};
    dynamic_pipe.CreateGraphicsPipeline();

    VkBuffer buffer_handle = buffer.handle();
    VkDeviceSize offset = 0u;
    VkDeviceSize stride = sizeof(float) * 4;
    VkDeviceSize size = stride * 3;

    VkBindVertexBuffer3InfoKHR binding_info = vku::InitStructHelper();
    binding_info.setStride = VK_TRUE;
    binding_info.addressRange = buffer.StridedAddressRange(stride);
    binding_info.addressFlags = 0u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindVertexBuffers(m_command_buffer, 0u, 1u, &buffer_handle, &offset);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dynamic_pipe);
    vk::CmdBindVertexBuffers2(m_command_buffer, 0u, 1u, &buffer_handle, &offset, &size, &stride);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0u, 1u, &binding_info);
    vk::CmdDraw(m_command_buffer, 3u, 1u, 0u, 0u);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveDeviceAddressCommands, StorageBufferAddressFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer storage_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();

    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);

    range.address = storage_buffer.Address();
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);

    m_command_buffer.End();
}

TEST_F(PositiveDeviceAddressCommands, BindVertexBuffers3Stride) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    const char* vs_source = R"glsl(
        #version 450
        layout(location=0) in vec4 x;
        void main(){}
    )glsl";
    VkShaderObj vs(*m_device, vs_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.AddDynamicState(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    VkVertexInputBindingDescription input_binding = {0, sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX};
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    VkVertexInputAttributeDescription attribute = {0, 0, VK_FORMAT_R32_SFLOAT, 0};
    pipe.vi_ci_.pVertexAttributeDescriptions = &attribute;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.CreateGraphicsPipeline();

    VkBindVertexBuffer3InfoKHR info = vku::InitStructHelper();
    info.setStride = VK_TRUE;
    info.addressRange = buffer.StridedAddressRange(sizeof(uint32_t));
    info.addressRange.stride = 4u;
    info.addressFlags = 0u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0, 1u, &info);
    vk::CmdDraw(m_command_buffer, 1, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(PositiveDeviceAddressCommands, MultipleRegions) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer src_buffer(*m_device, 1024u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 1024u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceMemoryCopyKHR regions[4];
    for (uint32_t i = 0; i < 4; ++i) {
        regions[i] = vku::InitStructHelper();
        regions[i].srcRange = src_buffer.AddressRange();
        regions[i].srcRange.address += i * 256u;
        regions[i].srcRange.size = 256u;
        regions[i].dstRange = dst_buffer.AddressRange();
        regions[i].dstRange.address += i * 256u;
        regions[i].dstRange.size = 256u;
    }
    VkCopyDeviceMemoryInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.regionCount = 4u;
    copy_memory_info.pRegions = regions;

    m_command_buffer.Begin();
    vk::CmdCopyMemoryKHR(m_command_buffer, &copy_memory_info);
    m_command_buffer.End();
}
