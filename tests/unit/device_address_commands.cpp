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
#include "../framework/render_pass_helper.h"

#include <algorithm>

void DeviceAddressCommands::InitBasicDeviceAddressCommands() {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    RETURN_IF_SKIP(Init());
}

class NegativeDeviceAddressCommands : public DeviceAddressCommands {};

TEST_F(NegativeDeviceAddressCommands, CopyProtectedFlag) {
    TEST_DESCRIPTION("Use addressFlags protected bit in an unprotected command buffer");
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (props11.protectedNoFault) {
        GTEST_SKIP() << "Test requires protectedNoFault to be VK_FALSE";
    }

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_GENERAL);

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-addressRange-13099");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToImageKHR-commandBuffer-13102");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-addressRange-13099");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToMemoryKHR-commandBuffer-13102");
    vk::CmdCopyImageToMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransferQueueAddressRange) {
    TEST_DESCRIPTION("Use invalid address in a transfer queue");

    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    auto transfer_queue = m_device->QueueFamily(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    if (!transfer_queue.has_value()) {
        GTEST_SKIP() << "Need a queue that supports transfer but not graphics or compute";
    }
    vkt::CommandPool pool(*m_device, *transfer_queue);
    vkt::CommandBuffer cb(*m_device, pool);

    vkt::Buffer buffer(*m_device, 32u * 32u * 8u * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange.address = buffer.Address() + 2;
    region.addressRange.size = 32u * 32u * 8u;
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

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToImageKHR-commandBuffer-13104");
    vk::CmdCopyMemoryToImageKHR(cb, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToMemoryKHR-commandBuffer-13104");
    vk::CmdCopyImageToMemoryKHR(cb, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdCopyMemoryToImageUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 8u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange.address = buffer.Address();
    region.addressRange.size = buffer.CreateInfo().size;
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
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToImageKHR-addressRange-13128");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdCopyImageToMemoryUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 8u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange.address = buffer.Address();
    region.addressRange.size = buffer.CreateInfo().size;
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
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToMemoryKHR-addressRange-13129");
    vk::CmdCopyImageToMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyGranularity) {
    TEST_DESCRIPTION("Copy with extent that is not a multiple of the queues minImageTransferGranularity");
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    auto queue_family_properties = m_device->Physical().queue_properties_;
    auto large_granularity_family =
        std::find_if(queue_family_properties.begin(), queue_family_properties.end(), [](VkQueueFamilyProperties family_properties) {
            VkExtent3D family_granularity = family_properties.minImageTransferGranularity;
            // We need a queue family that supports copy operations and has a large enough minImageTransferGranularity for the tests
            // below to make sense.
            return (family_properties.queueFlags & VK_QUEUE_TRANSFER_BIT || family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT ||
                    family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                   family_granularity.width > 1 && family_granularity.height > 1;
        });

    if (large_granularity_family == queue_family_properties.end()) {
        GTEST_SKIP() << "No queue family has a large enough granularity for this test to be meaningful";
    }
    const size_t queue_family_index = std::distance(queue_family_properties.begin(), large_granularity_family);
    vkt::CommandPool command_pool(*m_device, queue_family_index, 0);
    vkt::CommandBuffer cb(*m_device, command_pool);

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {31u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToImageKHR-imageOffset-13105");
    vk::CmdCopyMemoryToImageKHR(cb, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToMemoryKHR-imageOffset-13105");
    vk::CmdCopyImageToMemoryKHR(cb, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeDeviceAddressCommands, ComputeQueueDepthAspect) {
    TEST_DESCRIPTION("Copy image with depth aspect on a compute queue");

    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    auto compute_queue = m_device->QueueFamily(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
    if (!compute_queue.has_value()) {
        GTEST_SKIP() << "Need a queue that supports compute but not graphics";
    }
    vkt::CommandPool pool(*m_device, *compute_queue);
    vkt::CommandBuffer cb(*m_device, pool);

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu_);

    vkt::Buffer buffer(*m_device, 32u * 32u * 8u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, depth_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange.address = buffer.Address();
    region.addressRange.size = buffer.CreateInfo().size;
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToImageKHR-commandBuffer-13106");
    vk::CmdCopyMemoryToImageKHR(cb, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToMemoryKHR-commandBuffer-13106");
    vk::CmdCopyImageToMemoryKHR(cb, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdFillMemoryUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-dstRange-13000");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdFillMemoryAlignment) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 512u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address() + 2;
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13001");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    range.address = buffer.Address();
    range.size = 254u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13002");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdFillMemoryProtectedFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (props11.protectedNoFault) {
        GTEST_SKIP() << "Test requires protectedNoFault to be VK_FALSE";
    }

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-commandBuffer-13003");
    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13099");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdUpdateMemoryUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateMemoryKHR-dstRange-13005");
    vk::CmdUpdateMemoryKHR(m_command_buffer, &range, flags, sizeof(data), &data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdUpdateMemoryAlignmentAndSize) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 131072, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address() + 2;
    range.size = 256u;
    std::vector<uint8_t> data(131073);

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateMemoryKHR-pDstRange-13006");
    vk::CmdUpdateMemoryKHR(m_command_buffer, &range, flags, sizeof(uint32_t), data.data());
    m_errorMonitor->VerifyFound();

    range.address = buffer.Address();
    range.size = 131072;

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateMemoryKHR-pDstRange-13007");
    vk::CmdUpdateMemoryKHR(m_command_buffer, &range, flags, sizeof(uint32_t), data.data());
    m_errorMonitor->VerifyFound();

    range.size = 4u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateMemoryKHR-dataSize-13008");
    vk::CmdUpdateMemoryKHR(m_command_buffer, &range, flags, 8u, data.data());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateMemoryKHR-dataSize-13009");
    vk::CmdUpdateMemoryKHR(m_command_buffer, &range, flags, 3u, data.data());
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdUpdateMemoryProtectedFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props11);
    if (props11.protectedNoFault) {
        GTEST_SKIP() << "Test requires protectedNoFault to be VK_FALSE";
    }

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateMemoryKHR-commandBuffer-13010");
    m_errorMonitor->SetDesiredError("VUID-vkCmdUpdateMemoryKHR-pDstRange-13099");
    vk::CmdUpdateMemoryKHR(m_command_buffer, &range, flags, 4u, &data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMemoryToImageLayout) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToImageKHR-imageLayout-13019");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToMemoryKHR-imageLayout-13023");
    vk::CmdCopyImageToMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMemoryToImageUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

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

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryToImageKHR-pCopyMemoryInfo-13020");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyImageToMemoryKHR-pCopyMemoryInfo-13024");
    vk::CmdCopyImageToMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, AddressCommandFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags =
        VK_ADDRESS_COMMAND_STORAGE_BUFFER_USAGE_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-dstFlags-13100");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, AddressCommandFlagsSync) {
    AddRequiredFeature(vkt::Feature::synchronization2);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    const VkAddressCommandFlagsKHR flags =
        VK_ADDRESS_COMMAND_STORAGE_BUFFER_USAGE_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;

    VkMemoryRangeBarrierKHR memory_range_barrier = vku::InitStructHelper();
    memory_range_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    memory_range_barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    memory_range_barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    memory_range_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memory_range_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memory_range_barrier.addressRange = buffer.AddressRange();
    memory_range_barrier.addressFlags = flags;

    VkMemoryRangeBarriersInfoKHR memory_range_barriers_info = vku::InitStructHelper();
    memory_range_barriers_info.memoryRangeBarrierCount = 1u;
    memory_range_barriers_info.pMemoryRangeBarriers = &memory_range_barrier;

    VkDependencyInfo dependency_info_ = vku::InitStructHelper(&memory_range_barriers_info);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkMemoryRangeBarrierKHR-addressFlags-13100");
    vk::CmdPipelineBarrier2(m_command_buffer, &dependency_info_);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDrawIndexedIndirectCount2Flags) {
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    m_command_buffer.Begin();

    vkt::Buffer buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkDrawIndirectCount2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    info.addressFlags = VK_ADDRESS_COMMAND_STORAGE_BUFFER_USAGE_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;
    info.countAddressRange = count_buffer.AddressRange();
    info.countAddressFlags = 0u;
    info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-addressFlags-13100");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();

    info.addressFlags = 0u;
    info.countAddressFlags =
        VK_ADDRESS_COMMAND_STORAGE_BUFFER_USAGE_BIT_KHR | VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-countAddressFlags-13100");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDrawIndirect2KHR) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDrawIndirectCommand* draw_ptr = static_cast<VkDrawIndirectCommand*>(buffer.Memory().Map());
    draw_ptr->vertexCount = 3u;
    draw_ptr->instanceCount = 1u;
    draw_ptr->firstVertex = 0u;
    draw_ptr->firstInstance = 0u;

    VkDrawIndirect2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndirectCommand));
    info.addressFlags = 0u;
    info.drawCount = 1u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect2KHR-None-08606");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDrawIndexedIndirect2KHR) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDrawIndexedIndirectCommand* draw_ptr = static_cast<VkDrawIndexedIndirectCommand*>(buffer.Memory().Map());
    draw_ptr->indexCount = 3u;
    draw_ptr->instanceCount = 1u;
    draw_ptr->firstIndex = 0u;
    draw_ptr->vertexOffset = 0u;
    draw_ptr->firstInstance = 0u;

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t* indices = static_cast<uint32_t*>(index_buffer.Memory().Map());
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    VkDrawIndirect2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    info.addressFlags = 0u;
    info.drawCount = 1u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0u, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect2KHR-None-08606");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDrawIndirectCount2KHR) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDrawIndexedIndirectCommand* draw_ptr = static_cast<VkDrawIndexedIndirectCommand*>(buffer.Memory().Map());
    draw_ptr->indexCount = 3u;
    draw_ptr->instanceCount = 1u;
    draw_ptr->firstIndex = 0u;
    draw_ptr->vertexOffset = 0u;
    draw_ptr->firstInstance = 0u;

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    uint32_t* count_ptr = static_cast<uint32_t*>(count_buffer.Memory().Map());
    *count_ptr = 1u;

    VkDrawIndirectCount2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    info.addressFlags = 0u;
    info.countAddressRange = count_buffer.AddressRange();
    info.countAddressFlags = 0u;
    info.maxDrawCount = 1u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount2KHR-None-08606");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDrawIndexedIndirectCount2KHR) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDrawIndexedIndirectCommand* draw_ptr = static_cast<VkDrawIndexedIndirectCommand*>(buffer.Memory().Map());
    draw_ptr->indexCount = 3u;
    draw_ptr->instanceCount = 1u;
    draw_ptr->firstIndex = 0u;
    draw_ptr->vertexOffset = 0u;
    draw_ptr->firstInstance = 0u;

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    uint32_t* indices = static_cast<uint32_t*>(index_buffer.Memory().Map());
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    uint32_t* count_ptr = static_cast<uint32_t*>(count_buffer.Memory().Map());
    *count_ptr = 1u;

    VkDrawIndirectCount2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    info.addressFlags = 0u;
    info.countAddressRange = count_buffer.AddressRange();
    info.countAddressFlags = 0u;
    info.maxDrawCount = 1u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindIndexBuffer(m_command_buffer, index_buffer, 0u, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount2KHR-None-08606");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDispatchIndirect2KHR) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, sizeof(VkDispatchIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDispatchIndirectCommand* draw_ptr = static_cast<VkDispatchIndirectCommand*>(buffer.Memory().Map());
    draw_ptr->x = 16u;
    draw_ptr->y = 16u;
    draw_ptr->z = 1u;

    VkDispatchIndirect2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressFlags = 0u;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchIndirect2KHR-None-08606");
    vk::CmdDispatchIndirect2KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDrawMeshTasksIndirect2EXT) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDrawIndirectCommand* draw_ptr = static_cast<VkDrawIndirectCommand*>(buffer.Memory().Map());
    draw_ptr->vertexCount = 3u;
    draw_ptr->instanceCount = 1u;
    draw_ptr->firstVertex = 0u;
    draw_ptr->firstInstance = 0u;

    VkDrawIndirect2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndirectCommand));
    info.addressFlags = 0u;
    info.drawCount = 1u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirect2EXT-None-08606");
    vk::CmdDrawMeshTasksIndirect2EXT(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CmdDrawMeshTasksIndirectCount2EXT) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    VkDrawIndirectCommand* draw_ptr = static_cast<VkDrawIndirectCommand*>(buffer.Memory().Map());
    draw_ptr->vertexCount = 3u;
    draw_ptr->instanceCount = 1u;
    draw_ptr->firstVertex = 0u;
    draw_ptr->firstInstance = 0u;

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    uint32_t* count_ptr = static_cast<uint32_t*>(count_buffer.Memory().Map());
    *count_ptr = 1u;

    VkDrawIndirectCount2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndirectCommand));
    info.addressFlags = 0u;
    info.countAddressRange = count_buffer.AddressRange();
    info.countAddressFlags = 0u;
    info.maxDrawCount = 1u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCount2EXT-None-08606");
    vk::CmdDrawMeshTasksIndirectCount2EXT(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, AddressCommandXfbFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR |
                                     VK_ADDRESS_COMMAND_UNKNOWN_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-dstFlags-13101");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultQueryCount) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-firstQuery-09436");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 1u, 0u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-firstQuery-09437");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 2u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultStirde) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 2u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint64_t));
    range.stride = 0u;

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    vk::CmdBeginQuery(m_command_buffer, query_pool, 1u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 1u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-queryCount-09438");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 2u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryTimestampPartial) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint64_t));
    range.stride = 0u;

    m_command_buffer.Begin();
    vk::CmdWriteTimestamp(m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, query_pool, 0u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-queryType-09439");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, VK_QUERY_RESULT_PARTIAL_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultWithStatus) {
    AddRequiredExtensions(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint64_t));
    range.stride = 0u;

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-queryType-11874");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryUninitialized) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint64_t));
    range.stride = 0u;

    m_command_buffer.Begin();

    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);

    m_command_buffer.End();
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-None-13076");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultsStride) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint64_t));
    range.size = 128u;

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    range.address = buffer.Address() + 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-flags-13077");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    range.address = buffer.Address();
    range.stride = 9;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-flags-13077");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResults64Stride) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, 256, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint64_t));
    range.size = 128u;

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    range.address = buffer.Address() + 1;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-flags-13078");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyFound();

    range.address = buffer.Address();
    range.stride = 9;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-flags-13078");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, VK_QUERY_RESULT_64_BIT);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultsSize) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 2u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);
    vk::CmdBeginQuery(m_command_buffer, query_pool, 1u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 1u);

    range.size = 4u;
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-pDstRange-13079");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 2u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultsUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-pDstRange-13080");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

// Need to fix this test, currently reports 03222 and others
TEST_F(NegativeDeviceAddressCommands, DISABLED_QueryPerformanceCopy) {
    AddRequiredExtensions(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::performanceCounterQueryPools);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkPhysicalDevicePerformanceQueryPropertiesKHR performance_query_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(performance_query_props);

    if (!performance_query_props.allowCommandBufferQueryCopies) {
        GTEST_SKIP() << "allowCommandBufferQueryCopies must be unsupported";
    }

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-queryType-13081");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryPerformanceIntel) {
    AddRequiredExtensions(VK_INTEL_PERFORMANCE_QUERY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_PERFORMANCE_QUERY_INTEL;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);
    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-queryType-13082");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultsActive) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdBeginQuery(m_command_buffer, query_pool, 0u, 0u);

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-None-13083");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(m_command_buffer, query_pool, 0u);

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, QueryResultsAvailable) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer, query_pool, 0u, 1u);
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, 0u, 0u);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-None-13084");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, QueryProtectedFlag) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkQueryPoolCreateInfo query_pool_ci = vku::InitStructHelper();
    query_pool_ci.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_pool_ci.queryCount = 1u;
    vkt::QueryPool query_pool(*m_device, query_pool_ci);

    vkt::Buffer buffer(*m_device, sizeof(uint64_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkStridedDeviceAddressRangeKHR range = buffer.StridedAddressRange(sizeof(uint32_t));

    m_command_buffer.Begin();
    vk::CmdResetQueryPool(m_command_buffer, query_pool, 0u, 1u);
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResultsToMemoryKHR-dstFlags-13085");
    vk::CmdCopyQueryPoolResultsToMemoryKHR(m_command_buffer, query_pool, 0u, 1u, &range, VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR, 0u);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, IndexBufferUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    VkBindIndexBuffer3InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressFlags = 0u;
    info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-addressRange-13051");
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, IndexBufferAlignment) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

    VkBindIndexBuffer3InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressRange.address += 2;
    info.addressFlags = 0u;
    info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-addressRange-13052");
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, IndexBufferIndexTypeNone) {
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

    VkBindIndexBuffer3InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressFlags = 0u;
    info.indexType = VK_INDEX_TYPE_NONE_KHR;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-indexType-13053");
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, IndexBufferIndexTypeUint8) {
    AddRequiredExtensions(VK_KHR_INDEX_TYPE_UINT8_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

    VkBindIndexBuffer3InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressFlags = 0u;
    info.indexType = VK_INDEX_TYPE_UINT8;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-indexType-13054");
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, IndexBufferSizeZero) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

    VkBindIndexBuffer3InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressRange.size = 0;
    info.addressFlags = 0u;
    info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-None-13055");
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, IndexBufferAddressZero) {
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

    VkBindIndexBuffer3InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressRange.size = 0;
    info.addressFlags = 0u;
    info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindIndexBuffer3InfoKHR-addressRange-13056");
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, VertexBufferFirstBinding) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    uint32_t first_binding = m_device->Physical().limits_.maxVertexInputBindings;

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    VkBindVertexBuffer3InfoKHR info = vku::InitStructHelper();
    info.setStride = VK_FALSE;
    info.addressRange = buffer.StridedAddressRange();
    info.addressFlags = 0u;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindVertexBuffers3KHR-firstBinding-13070");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, first_binding, 1u, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, VertexBufferBindingCount) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    uint32_t first_binding = m_device->Physical().limits_.maxVertexInputBindings;

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    VkBindVertexBuffer3InfoKHR infos[2];
    infos[0] = vku::InitStructHelper();
    infos[0].setStride = VK_FALSE;
    infos[0].addressRange = buffer.StridedAddressRange();
    infos[0].addressFlags = 0u;
    infos[1] = vku::InitStructHelper();
    infos[1].setStride = VK_FALSE;
    infos[1].addressRange = buffer.StridedAddressRange();
    infos[1].addressFlags = 0u;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindVertexBuffers3KHR-firstBinding-13071");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, first_binding - 1, 2u, infos);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, VertexBufferBindingSizeZero) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    VkBindVertexBuffer3InfoKHR infos[2];
    infos[0] = vku::InitStructHelper();
    infos[0].setStride = VK_FALSE;
    infos[0].addressRange = buffer.StridedAddressRange();
    infos[0].addressFlags = 0u;
    infos[1] = vku::InitStructHelper();
    infos[1].setStride = VK_FALSE;
    infos[1].addressRange.address = 0u;
    infos[1].addressRange.size = 0u;
    infos[1].addressRange.stride = 0u;
    infos[1].addressFlags = 0u;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindVertexBuffer3InfoKHR-size-13072");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0, 2u, infos);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, VertexBufferBindingStride) {
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
    info.addressRange.stride = 1u;
    info.addressFlags = 0u;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0, 1u, &info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindVertexBuffers3KHR-addressRange-13073");
    vk::CmdDraw(m_command_buffer, 1, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, VertexBufferUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

    VkBindVertexBuffer3InfoKHR info = vku::InitStructHelper();
    info.setStride = VK_FALSE;
    info.addressRange = buffer.StridedAddressRange();
    info.addressFlags = 0u;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindVertexBuffer3InfoKHR-addressRange-13074");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0, 1u, &info);
    m_errorMonitor->VerifyFound();

    info.addressRange.address = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkStridedDeviceAddressRangeKHR-size-11411");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0, 1u, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, VertexBufferSizeZero) {
    AddRequiredFeature(vkt::Feature::nullDescriptor);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    VkBindVertexBuffer3InfoKHR info = vku::InitStructHelper();
    info.setStride = VK_FALSE;
    info.addressRange = buffer.StridedAddressRange();
    info.addressRange.size = 0u;
    info.addressFlags = 0u;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkBindVertexBuffer3InfoKHR-addressRange-13075");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0, 1u, &info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackFeature) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    vkt::Buffer xfb(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    // Limit is not set when feature is not enabled
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-addressRange-13092");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-transformFeedback-02355");
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-transformFeedback-02366");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-transformFeedback-02374");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackAlreadyActive) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();
    info.addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 0u, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-None-02365");
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-None-02367");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 0u, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-None-02375");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackMaxBuffers) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT xfb_properties = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(xfb_properties);

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer xfb(*m_device, 256u, VK_BUFFER_USAGE_2_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT, vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT infos[2];
    infos[0] = vku::InitStructHelper();
    infos[0].addressRange = xfb.AddressRange();
    infos[1] = vku::InitStructHelper();
    infos[1].addressRange = xfb.AddressRange();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-firstBinding-02356");
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, xfb_properties.maxTransformFeedbackBuffers, 1u, infos);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-firstCounter-02368");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, xfb_properties.maxTransformFeedbackBuffers, 0u, nullptr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-firstBinding-02357");
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, xfb_properties.maxTransformFeedbackBuffers - 1, 2u, infos);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-firstCounter-02369");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, xfb_properties.maxTransformFeedbackBuffers - 1u, 2u, infos);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-firstCounterBuffer-02376");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, xfb_properties.maxTransformFeedbackBuffers, 2u, infos);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-firstCounterBuffer-02377");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, xfb_properties.maxTransformFeedbackBuffers - 1u, 2u, infos);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackBeginCount) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT infos[2];
    infos[0] = vku::InitStructHelper();
    infos[0].addressRange = xfb.AddressRange();
    infos[0].addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;
    infos[1] = vku::InitStructHelper();
    infos[1].addressRange = xfb.AddressRange();
    infos[1].addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, infos);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-firstCounter-09630");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0, 2u, infos);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackMissingPipeline) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();
    info.addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-None-06233");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackPipelineMissingXfb) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();
    info.addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-None-04128");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackInMultiview) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    AddRequiredFeature(vkt::Feature::multiview);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    uint32_t viewMask = 0x1u;
    VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo = vku::InitStructHelper();
    renderPassMultiviewCreateInfo.subpassCount = 1;
    renderPassMultiviewCreateInfo.pViewMasks = &viewMask;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED);
    rp.AddColorAttachment(0, VK_IMAGE_LAYOUT_GENERAL);
    rp.CreateRenderPass(&renderPassMultiviewCreateInfo);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    auto image_view_ci = image.BasicViewCreatInfo();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    const vkt::ImageView imageView(*m_device, image_view_ci);

    vkt::Framebuffer framebuffer(*m_device, rp, 1, &imageView.handle());

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.renderPass = rp;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(rp, framebuffer, 32, 32);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-None-02373");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 0u, nullptr);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackCounterInfoAddressSize) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();
    info.addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);

    info.addressRange.size = 3u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-addressRange-13090");
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-pCounterInfos-13093");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-pCounterInfos-13095");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackCounterInfoBufferUsage) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-addressRange-13091");
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    info.addressRange = xfb.AddressRange();
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);

    info.addressRange = buffer.AddressRange();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginTransformFeedback2EXT-pCounterInfos-13094");
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    info.addressRange = xfb.AddressRange();
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);

    info.addressRange = buffer.AddressRange();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-pCounterInfos-13096");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    info.addressRange = xfb.AddressRange();
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackEndCounterInfoAddressSize) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();
    info.addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);

    info.addressRange.size = 3u;
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-pCounterInfos-13095");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    info.addressRange = xfb.AddressRange();
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackEndCounterInfoBufferUsage) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer xfb(*m_device, 256u,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
                    vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();
    info.addressFlags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);
    vk::CmdBeginTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);

    info.addressRange = buffer.AddressRange();
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndTransformFeedback2EXT-pCounterInfos-13096");
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    info.addressRange = xfb.AddressRange();
    vk::CmdEndTransformFeedback2EXT(m_command_buffer, 0u, 1u, &info);

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, TransformFeedbackBindSize) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    VkPhysicalDeviceTransformFeedbackPropertiesEXT xfb_properties = vku::InitStructHelper();
    VkPhysicalDeviceVulkan11Properties props11 = vku::InitStructHelper(&xfb_properties);
    GetPhysicalDeviceProperties2(props11);

    if (xfb_properties.maxTransformFeedbackBufferSize == vvl::kU32Max) {
        GTEST_SKIP() << "maxTransformFeedbackBufferSize is too large";
    } else if (xfb_properties.maxTransformFeedbackBufferSize >= props11.maxMemoryAllocationSize) {
        GTEST_SKIP() << "maxTransformFeedbackBufferSize is too large";
    }

    const char* vert = R"glsl(
        #version 460
        layout(location = 0, xfb_buffer = 0, xfb_offset = 0, xfb_stride = 12) out vec3 outPos;

        void main() {
            vec3 pos = vec3(gl_VertexIndex);
            outPos = pos;
            gl_Position = vec4(pos, 1.0);
        }
    )glsl";

    auto vs = VkShaderObj(*m_device, vert, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.CreateGraphicsPipeline();

    vkt::Buffer xfb(*m_device, xfb_properties.maxTransformFeedbackBufferSize,
                    VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT, vkt::device_address);
    VkBindTransformFeedbackBuffer2InfoEXT info = vku::InitStructHelper();
    info.addressRange = xfb.AddressRange();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBindTransformFeedbackBuffers2EXT-addressRange-13092");
    vk::CmdBindTransformFeedbackBuffers2EXT(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, ConditionalRenderingAlreadyActive) {
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::conditionalRendering);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT, vkt::device_address);

    VkConditionalRenderingBeginInfo2EXT info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressFlags = 0u;
    info.flags = 0u;

    m_command_buffer.Begin();
    vk::CmdBeginConditionalRendering2EXT(m_command_buffer, &info);

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginConditionalRendering2EXT-None-13063");
    vk::CmdBeginConditionalRendering2EXT(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();

    vk::CmdEndConditionalRenderingEXT(m_command_buffer);

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, ConditionalRenderingUsage) {
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::conditionalRendering);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkConditionalRenderingBeginInfo2EXT info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressFlags = 0u;
    info.flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkConditionalRenderingBeginInfo2EXT-addressRange-13064");
    vk::CmdBeginConditionalRendering2EXT(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, ConditionalRenderingAlignment) {
    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::conditionalRendering);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT, vkt::device_address);

    VkConditionalRenderingBeginInfo2EXT info = vku::InitStructHelper();
    info.addressRange = buffer.AddressRange();
    info.addressRange.address += 2;
    info.addressRange.size = 128u;
    info.addressFlags = 0u;
    info.flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkConditionalRenderingBeginInfo2EXT-addressRange-13065");
    vk::CmdBeginConditionalRendering2EXT(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();

    info.addressRange = buffer.AddressRange();
    info.addressRange.size = 2u;
    m_errorMonitor->SetDesiredError("VUID-VkConditionalRenderingBeginInfo2EXT-addressRange-13066");
    vk::CmdBeginConditionalRendering2EXT(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyUnboundImageMemory) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci, vkt::no_mem);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = image_ci.extent;

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07966");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMipLevel) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 1u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-imageSubresource-07967");
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-pRegions-13032");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyLayerCount) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-imageSubresource-07968");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopySubsampledImage) {
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07969");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopySampleCount) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07973");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, Copy1DImage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_1D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32u, 1u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 2u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07979");
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-pRegions-13032");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, Copy3DDepth) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 8, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32u, 32u, 4u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 2u, 5u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-imageOffset-09104");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, Copy2DDepth) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 8, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32u, 32u, 1u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 1};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07980");
    m_errorMonitor->SetUnexpectedError("VUID-VkCopyDeviceMemoryImageInfoKHR-imageOffset-09104");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyOffsetMultiple) {
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_BC3_SRGB_BLOCK,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {1, 0, 0};
    region.imageExtent = {4u, 4u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07274");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    region.imageOffset = {0, 1, 0};
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07275");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyExtentMultiple) {
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_BC3_SRGB_BLOCK,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {5u, 4u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-00207");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    region.imageExtent = {4u, 5u, 1u};
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-00208");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyAspect) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM,
                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-imageSubresource-09105");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, Copy3DLayer) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 16, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent = {32u, 32u, 2u};
    image_ci.mipLevels = 1u;
    image_ci.arrayLayers = 1u;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 2u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-07983");
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-imageSubresource-07968");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyAddressLengthAndHeight) {
    AddRequiredFeature(vkt::Feature::textureCompressionBC);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_BC3_SRGB_BLOCK,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 5u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {4u, 4u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-addressRowLength-09106");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    region.addressRowLength = 0u;
    region.addressImageHeight = 5u;
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-addressImageHeight-09107");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyAddressLengthAndHeight2) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 3u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {4u, 4u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-addressRowLength-09101");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    region.addressRowLength = 0u;
    region.addressImageHeight = 3u;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-addressImageHeight-09102");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMultipleAspects) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkFormat ds_format = FindSupportedDepthStencilFormat(gpu_);
    auto image_ci =
        vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, ds_format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {4u, 4u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-aspectMask-09103");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyZeroExtent) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32 * 32 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    auto image_ci = vkt::Image::ImageCreateInfo2D(32, 32, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
                                                  VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    vkt::Image image(*m_device, image_ci);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {0u, 4u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_info = vku::InitStructHelper();
    copy_info.image = image;
    copy_info.regionCount = 1u;
    copy_info.pRegions = &region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-imageExtent-06659");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    region.imageExtent = {4u, 0u, 1u};
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-imageExtent-06660");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMemoryProtectedFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer src_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceMemoryCopyKHR region = vku::InitStructHelper();
    region.srcRange = src_buffer.AddressRange();
    region.srcFlags = VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR;
    region.dstRange = dst_buffer.AddressRange();
    region.dstFlags = 0u;

    VkCopyDeviceMemoryInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryCopyKHR-srcRange-13099");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryKHR-commandBuffer-13012");
    vk::CmdCopyMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    region.srcFlags = 0u;
    region.dstFlags = VK_ADDRESS_COMMAND_PROTECTED_BIT_KHR;
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryCopyKHR-dstRange-13099");
    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyMemoryKHR-commandBuffer-13013");
    vk::CmdCopyMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMemorySize) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer src_buffer(*m_device, 512u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceMemoryCopyKHR region = vku::InitStructHelper();
    region.srcRange = src_buffer.AddressRange();
    region.srcFlags = 0u;
    region.dstRange = dst_buffer.AddressRange();
    region.dstFlags = 0u;

    VkCopyDeviceMemoryInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryCopyKHR-size-13016");
    vk::CmdCopyMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMemorySrcUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer src_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceMemoryCopyKHR region = vku::InitStructHelper();
    region.srcRange = src_buffer.AddressRange();
    region.srcFlags = 0u;
    region.dstRange = dst_buffer.AddressRange();
    region.dstFlags = 0u;

    VkCopyDeviceMemoryInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryCopyKHR-srcRange-13017");
    vk::CmdCopyMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyMemoryDstUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer src_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Buffer dst_buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);

    VkDeviceMemoryCopyKHR region = vku::InitStructHelper();
    region.srcRange = src_buffer.AddressRange();
    region.srcFlags = 0u;
    region.dstRange = dst_buffer.AddressRange();
    region.dstFlags = 0u;

    VkCopyDeviceMemoryInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryCopyKHR-dstRange-13018");
    vk::CmdCopyMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyDepthStencilAddress) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 16u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);
    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu_);
    vkt::Image image(*m_device, 32u, 32u, ds_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressRange.address += 2u;
    region.addressRange.size = 32 * 32 * 8u;
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_GENERAL);

    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-image-13031");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyImageLayout) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();
    image.SetLayout(m_command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-imageLayout-13028");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyAddressRangeOverlap) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u * 4u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR regions[2];
    regions[0] = vku::InitStructHelper();
    regions[0].addressRange = buffer.AddressRange();
    regions[0].addressFlags = 0u;
    regions[0].addressRowLength = 0u;
    regions[0].addressImageHeight = 0u;
    regions[0].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    regions[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    regions[0].imageOffset = {0, 0, 0};
    regions[0].imageExtent = {32u, 4u, 1u};
    regions[1] = vku::InitStructHelper();
    regions[1].addressRange = buffer.AddressRange();
    regions[1].addressFlags = 0u;
    regions[1].addressRowLength = 0u;
    regions[1].addressImageHeight = 0u;
    regions[1].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    regions[1].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    regions[1].imageOffset = {0, 4, 0};
    regions[1].imageExtent = {32u, 4u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 2u;
    copy_memory_info.pRegions = regions;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryImageInfoKHR-addressRange-13026");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, CopyDeviceMemoryAddressOverlap) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceMemoryCopyKHR region = vku::InitStructHelper();
    region.srcRange = buffer.AddressRange();
    region.srcFlags = 0u;
    region.dstRange = buffer.AddressRange();
    region.dstRange.address += 128u;
    region.dstFlags = 0u;

    VkCopyDeviceMemoryInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkCopyDeviceMemoryInfoKHR-srcRange-13015");
    vk::CmdCopyMemoryKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

// Todo https://gitlab.khronos.org/vulkan/Vulkan-ValidationLayers/-/merge_requests/310#note_589155
TEST_F(NegativeDeviceAddressCommands, DISABLED_CopyDeviceMemorySize) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 32u * 32u, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vkt::device_address);
    vkt::Image image(*m_device, 32u, 32u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    VkDeviceMemoryImageCopyKHR region = vku::InitStructHelper();
    region.addressRange = buffer.AddressRange();
    region.addressFlags = 0u;
    region.addressRowLength = 0u;
    region.addressImageHeight = 0u;
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 0u, 1u};
    region.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {32u, 32u, 1u};

    VkCopyDeviceMemoryImageInfoKHR copy_memory_info = vku::InitStructHelper();
    copy_memory_info.image = image;
    copy_memory_info.regionCount = 1u;
    copy_memory_info.pRegions = &region;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkDeviceMemoryImageCopyKHR-size-13037");
    vk::CmdCopyMemoryToImageKHR(m_command_buffer, &copy_memory_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawCount) {
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 77u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 16;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13110");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13110");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect2KHR-pInfo-13110");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13110");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, StrideZero) {
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(0u);
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 2u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(0u);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 2u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13111");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13111");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect2KHR-pInfo-13111");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13111");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, StrideTooSmall) {
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(4u);
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 2u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(4u);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 2u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13112");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13112");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect2KHR-pInfo-13112");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13112");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, StrideMultiple) {
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand) + 2u;

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 2u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 2u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect2KHR-pInfo-13113");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount2KHR-pInfo-13113");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirect2KHR-pInfo-13113");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndirectCount2KHR-pInfo-13113");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawIndirectUsage) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 1u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-addressRange-13107");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-addressRange-13107");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-addressRange-13107");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-addressRange-13107");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawMeshIndirectUsage) {
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 1u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-addressRange-13107");
    vk::CmdDrawMeshTasksIndirect2EXT(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-addressRange-13107");
    vk::CmdDrawMeshTasksIndirectCount2EXT(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DispatchIndirectUsage) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDispatchIndirect2InfoKHR dispatch_indirect_info = vku::InitStructHelper();
    dispatch_indirect_info.addressRange = buffer.AddressRange();
    dispatch_indirect_info.addressFlags = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkDispatchIndirect2InfoKHR-addressRange-13107");
    vk::CmdDispatchIndirect2KHR(m_command_buffer, &dispatch_indirect_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawIndirectByteCount) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    VkBindTransformFeedbackBuffer2InfoEXT counter_info = vku::InitStructHelper();
    counter_info.addressRange = buffer.AddressRange();
    counter_info.addressFlags = 0u;

    m_errorMonitor->SetDesiredError("UNASSIGNED-VkBindTransformFeedbackBuffer2InfoEXT-addressRange");
    vk::CmdDrawIndirectByteCount2EXT(m_command_buffer, 3u, 0u, &counter_info, 0u, sizeof(uint32_t));
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawIndirectCounterUsage) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13114");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13114");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawIndirectCounterAddressAlignment) {
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, 64, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressRange.address += 2;
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13115");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13115");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawIndirectCounterSize) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressRange.size = 2;
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 0u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13117");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-countAddressRange-13117");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, MissingIndexBuffer) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 1u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirect2KHR-None-07312");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawIndexedIndirectCount2KHR-None-07312");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DispatchAddressRangeSize) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);

    VkDispatchIndirect2InfoKHR dispatch_indirect_info = vku::InitStructHelper();
    dispatch_indirect_info.addressRange = buffer.AddressRange();
    dispatch_indirect_info.addressRange.size = 4u;
    dispatch_indirect_info.addressFlags = 0u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatchIndirect2KHR-pInfo-13050");
    vk::CmdDispatchIndirect2KHR(m_command_buffer, &dispatch_indirect_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawMeshIndirectCountFeature) {
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-vkCmdDrawMeshTasksIndirectCount2EXT-drawIndirectCount-13069");
    vk::CmdDrawMeshTasksIndirectCount2EXT(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, MultiDrawIndirectFeature) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 2u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-drawCount-02718");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-drawCount-02718");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, MultiDrawIndirectCount) {
    AddRequiredFeature(vkt::Feature::multiDrawIndirect);
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    const uint32_t draw_count = m_device->Physical().limits_.maxDrawIndirectCount;
    if (draw_count > 1024) {
        GTEST_SKIP() << "maxDrawIndirectCount is too large";
    }
    vkt::Buffer buffer(*m_device, 1024 * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                       vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 256u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndexedIndirectCommand));
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = draw_count;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-drawCount-02719");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-drawCount-02719");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, MultiDrawMeshIndirectFeature) {
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::meshShader);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    VkDrawIndirect2InfoKHR info = vku::InitStructHelper();
    info.addressRange = buffer.StridedAddressRange(sizeof(VkDrawIndirectCommand));
    info.addressFlags = 0u;
    info.drawCount = 2u;

    const char* mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        void main() {
            SetMeshOutputsEXT(3,1);
        }
    )glsl";

    VkShaderObj ms(*m_device, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(*m_device, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.CreateGraphicsPipeline();

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-drawCount-02718");
    vk::CmdDrawMeshTasksIndirect2EXT(m_command_buffer, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, DrawAddressRangeAlignment) {
    AddRequiredFeature(vkt::Feature::drawIndirectCount);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());
    InitRenderTarget();

    vkt::Buffer buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);
    vkt::Buffer index_buffer(*m_device, 2048u, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vkt::device_address);
    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, vkt::device_address);

    CreatePipelineHelper pipe(*this);
    pipe.CreateGraphicsPipeline();

    VkBindIndexBuffer3InfoKHR index_buffer_info = vku::InitStructHelper();
    index_buffer_info.addressRange = index_buffer.AddressRange();
    index_buffer_info.addressFlags = 0u;
    index_buffer_info.indexType = VK_INDEX_TYPE_UINT32;

    m_command_buffer.Begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    vk::CmdBindIndexBuffer3KHR(m_command_buffer, &index_buffer_info);

    const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand) * sizeof(VkDrawIndirectCommand);

    VkDrawIndirect2InfoKHR draw_indirect_2_info = vku::InitStructHelper();
    draw_indirect_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_2_info.addressRange.address += 2;
    draw_indirect_2_info.addressFlags = 0u;
    draw_indirect_2_info.drawCount = 1u;

    VkDrawIndirectCount2InfoKHR draw_indirect_count_2_info = vku::InitStructHelper();
    draw_indirect_count_2_info.addressRange = buffer.StridedAddressRange(stride);
    draw_indirect_count_2_info.addressRange.address += 2;
    draw_indirect_count_2_info.addressFlags = 0u;
    draw_indirect_count_2_info.countAddressRange = count_buffer.AddressRange();
    draw_indirect_count_2_info.countAddressFlags = 0u;
    draw_indirect_count_2_info.maxDrawCount = 1u;

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-addressRange-13109");
    vk::CmdDrawIndexedIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-addressRange-13109");
    vk::CmdDrawIndexedIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirect2InfoKHR-addressRange-13109");
    vk::CmdDrawIndirect2KHR(m_command_buffer, &draw_indirect_2_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkDrawIndirectCount2InfoKHR-addressRange-13109");
    vk::CmdDrawIndirectCount2KHR(m_command_buffer, &draw_indirect_count_2_info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, AddressRangeBound) {
    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkBufferCreateInfo buffer_ci = vkt::Buffer::CreateInfo(
        65536 * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    vkt::Buffer sparse_buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), sparse_buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_binds[2] = {};
    buffer_memory_binds[0].resourceOffset = 65536 * 2;
    buffer_memory_binds[0].size = 65536;
    buffer_memory_binds[0].memory = buffer_mem;
    buffer_memory_binds[0].memoryOffset = 65536 * 2;
    buffer_memory_binds[1].resourceOffset = 0u;
    buffer_memory_binds[1].size = 65536;
    buffer_memory_binds[1].memory = buffer_mem;
    buffer_memory_binds[1].memoryOffset = 0;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = sparse_buffer;
    buffer_memory_bind_info.bindCount = 2u;
    buffer_memory_bind_info.pBinds = buffer_memory_binds;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1u;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    vkt::Queue* sparse_queue = m_device->QueuesWithSparseCapability()[0];
    vk::QueueBindSparse(sparse_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);
    sparse_queue->Wait();

    VkDeviceAddressRangeKHR range;
    range.address = sparse_buffer.Address();
    range.size = 65536 * 5 / 2;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = VK_ADDRESS_COMMAND_FULLY_BOUND_BIT_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13097");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    range.size = 65536;
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureDeviceAddress) {
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = 0u;
    as_ci.addressRange = {};
    as_ci.addressRange.size = 256u;
    as_ci.addressFlags = VK_ADDRESS_COMMAND_UNKNOWN_STORAGE_BUFFER_USAGE_BIT_KHR;
    as_ci.addressFlags = VK_ADDRESS_COMMAND_STORAGE_BUFFER_USAGE_BIT_KHR;
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11602");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vkt::device_address);
    as_ci.addressRange = buffer.AddressRange();
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11603");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();

    vkt::Buffer buffer2(*m_device, 256u, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);
    as_ci.addressRange = buffer2.AddressRange();
    as_ci.addressRange.address = buffer2.AddressRange().address + 2;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11605");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureDeviceAddressSparseResidency) {
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT | VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
    buffer_ci.size = 256u;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info;
    buffer_memory_bind_info.buffer = buffer;
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1u;
    bind_info.pBufferBinds = &buffer_memory_bind_info;

    vkt::Queue* sparse_queue = m_device->QueuesWithSparseCapability()[0];
    vk::QueueBindSparse(sparse_queue->handle(), 1, &bind_info, VK_NULL_HANDLE);
    sparse_queue->Wait();

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = 0u;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.addressFlags = 0u;
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11604");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureDeviceAddressCaptureReplay) {
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::accelerationStructureCaptureReplay);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.addressFlags = 0u;
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-11607");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureDeviceAddressCaptureReplay2) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::deviceAddressCommands);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddressCaptureReplay);
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    buffer_ci.size = 256u;
    buffer_ci.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer(*m_device, buffer_ci, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       &allocate_flag_info);

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = 0u;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.addressFlags = 0u;
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-11606");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureSize) {
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = 0u;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.addressRange.size = 0u;
    as_ci.addressFlags = 0u;
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-addressRange-11608");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureCreateFlags) {
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-03613");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, DISABLED_AccelerationStructureDescriptorBufferCreateFlags) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfo2KHR-createFlags-08108");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();

    uint32_t data[128];
    const auto ocddci = vku::InitStruct<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(nullptr, &data);

    as_ci.pNext = &ocddci;
    as_ci.createFlags = 0u;
    m_errorMonitor->SetDesiredError("VUID-VkAccelerationStructureCreateInfoKHR-pNext-08109");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureFeature) {
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = 0u;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-vkCreateAccelerationStructure2KHR-accelerationStructure-03611");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, AccelerationStructureDeviceAddressCommandsFeature) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, vkt::device_address);

    VkAccelerationStructureCreateInfo2KHR as_ci = vku::InitStructHelper();
    as_ci.createFlags = 0u;
    as_ci.addressRange = buffer.AddressRange();
    as_ci.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    VkAccelerationStructureKHR as;
    m_errorMonitor->SetDesiredError("VUID-vkCreateAccelerationStructure2KHR-deviceAddressCommands-13086");
    vk::CreateAccelerationStructure2KHR(*m_device, &as_ci, nullptr, &as);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDeviceAddressCommands, StorageBufferMissingAddressFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13122");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, StorageBufferInvalidAddressFlags) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = VK_ADDRESS_COMMAND_STORAGE_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13123");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, VertexBufferStride) {
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vkt::device_address);

    VkBindVertexBuffer3InfoKHR info = vku::InitStructHelper();
    info.setStride = VK_TRUE;
    info.addressRange = buffer.StridedAddressRange(m_device->Physical().limits_.maxVertexInputBindingStride + 1);
    info.addressFlags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkBindVertexBuffer3InfoKHR-setStride-13126");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    info.setStride = VK_FALSE;
    info.addressRange.stride = 4u;
    m_errorMonitor->SetDesiredError("VUID-VkBindVertexBuffer3InfoKHR-setStride-13127");
    vk::CmdBindVertexBuffers3KHR(m_command_buffer, 0u, 1u, &info);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, XfbBufferMissingAddressFlags) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = 0u;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13124");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeDeviceAddressCommands, XfbBufferInvalidAddressFlags) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::transformFeedback);
    RETURN_IF_SKIP(InitBasicDeviceAddressCommands());

    vkt::Buffer buffer(*m_device, 256u, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkt::device_address);

    VkDeviceAddressRangeKHR range;
    range.address = buffer.Address();
    range.size = 256u;
    const uint32_t data = 255;

    VkAddressCommandFlagsKHR flags = VK_ADDRESS_COMMAND_TRANSFORM_FEEDBACK_BUFFER_USAGE_BIT_KHR;

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdFillMemoryKHR-pDstRange-13125");
    vk::CmdFillMemoryKHR(m_command_buffer, &range, flags, data);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}
