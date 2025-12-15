/*
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class NegativeTileMemoryHeap : public TileMemoryHeapTest {};

TEST_F(NegativeTileMemoryHeap, CreateImageTest) {
    TEST_DESCRIPTION("Use VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM in CreateImage without enabling the tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-tileMemoryHeap-10766");
    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format, VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image image(*m_device, image_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, CreateBufferTest) {
    TEST_DESCRIPTION("Use VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM in CreateBuffer without enabling the tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-tileMemoryHeap-10762");
}

TEST_F(NegativeTileMemoryHeap, CreateBufferTestFlags) {
    TEST_DESCRIPTION("Test Flags in CreateBuffer with tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseResidencyAliased);
    AddRequiredFeature(vkt::Feature::protectedMemory);

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM;
    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");

    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");

    buffer_ci.flags = VK_BUFFER_CREATE_SPARSE_ALIASED_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");

    buffer_ci.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");

    buffer_ci.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");

    buffer_ci.flags = VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");

    buffer_ci.flags = VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");
}

TEST_F(NegativeTileMemoryHeap, CreateBufferTestUsageFlags) {
    TEST_DESCRIPTION("Test Usage Flags in CreateBuffer with tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10764");
}

TEST_F(NegativeTileMemoryHeap, TileMemorySizeInfo) {
    TEST_DESCRIPTION("Allocate tile memory size info greater than largest heap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    VkDeviceSize maxHeapSize = 0;
    for (uint32_t i = 0; i < memory_info.memoryHeapCount; i++) {
        if (memory_info.memoryHeaps[i].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM) {
            maxHeapSize = std::max(maxHeapSize, memory_info.memoryHeaps[i].size);
        }
    }

    VkTileMemorySizeInfoQCOM tile_memory_size_info = vku::InitStructHelper();
    tile_memory_size_info.size = maxHeapSize + 1;

    VkRenderingAttachmentInfo color_attachment = {};
    color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment.imageView = VK_NULL_HANDLE;  // Use a valid image view in real test
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo rendering_info = {};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pNext = &tile_memory_size_info;
    rendering_info.renderArea.extent.width = 128;
    rendering_info.renderArea.extent.height = 128;
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkTileMemorySizeInfoQCOM-size-10729");
    m_command_buffer.BeginRendering(rendering_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, AllocateMemory) {
    TEST_DESCRIPTION(
        "Allocate memory with a memory type index that corresponds to a VkmemoryHeap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
    VkMemoryAllocateInfo image_mem_alloc = vku::InitStructHelper();
    image_mem_alloc.allocationSize = image_mem_reqs.size;
    int mem_type_index = -1;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heap_index = memory_info.memoryTypes[i].heapIndex;
        if (memory_info.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM) {
            mem_type_index = i;
        }
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    image_mem_alloc.memoryTypeIndex = mem_type_index;
    VkDeviceMemory image_mem = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredError("VUID-VkTileMemoryBindInfoQCOM-memoryTypeIndex");
    vk::AllocateMemory(device(), &image_mem_alloc, nullptr, &image_mem);
    m_errorMonitor->VerifyFound();
    vk::FreeMemory(device(), image_mem, nullptr);
}

TEST_F(NegativeTileMemoryHeap, CmdBindTileMemoryQCOM) {
    TEST_DESCRIPTION(
        "Allocate memory with a memory type index that corresponds to a VkmemoryHeap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    int mem_type_index = -1;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heap_index = memory_info.memoryTypes[i].heapIndex;
        if (memory_info.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM) {
            continue;
        } else {
            mem_type_index = i;
            break;
        }
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkMemoryAllocateInfo tile_mem_alloc_info = vku::InitStructHelper();
    tile_mem_alloc_info.memoryTypeIndex = mem_type_index;
    tile_mem_alloc_info.allocationSize = 1024;
    VkDeviceMemory tile_mem = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &tile_mem_alloc_info, nullptr, &tile_mem);
    VkTileMemoryBindInfoQCOM bind_info = vku::InitStructHelper();
    bind_info.memory = tile_mem;
    bind_info.sType = VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkTileMemoryBindInfoQCOM-memory-10726");
    vk::CmdBindTileMemoryQCOM(m_command_buffer, &bind_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), tile_mem, NULL);
}

TEST_F(NegativeTileMemoryHeap, RenderPassResolveAttachment) {
    TEST_DESCRIPTION("Allocate tile info size greater than largest heap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    VkDeviceMemory image_mem = VK_NULL_HANDLE;

    if (!AllocateTileImage(image, image_mem)) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkt::ImageView image_view(*m_device, ivci);

    VkRenderingAttachmentInfo resolve_attachment = {};
    resolve_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    resolve_attachment.resolveImageView = image_view;
    resolve_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfo rendering_info = {};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.pNext = nullptr;
    rendering_info.renderArea.extent.width = width;
    rendering_info.renderArea.extent.height = height;
    rendering_info.layerCount = 1;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &resolve_attachment;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-resolveImageView-10728");
    m_command_buffer.BeginRendering(rendering_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), image_mem, nullptr);
}

TEST_F(NegativeTileMemoryHeap, BindBufferMemoryTestsFromTileMemoryHeap) {
    TEST_DESCRIPTION(
        "Allocate memory with a memory type index that corresponds to a VkmemoryHeap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    auto buffer_ci = vkt::Buffer::CreateInfo(256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM);
    VkBuffer buffer[2];
    VkTileMemoryRequirementsQCOM tileMemReqs;
    VkMemoryRequirements2 buffer_mem_reqs2;
    tileMemReqs.sType = VK_STRUCTURE_TYPE_TILE_MEMORY_REQUIREMENTS_QCOM;
    for (int i = 0; i < 2; i++) {
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer[i]);
        buffer_mem_reqs2 = vku::InitStructHelper();
        VkBufferMemoryRequirementsInfo2 bufferMemReqsInfo = vku::InitStructHelper();
        bufferMemReqsInfo.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
        bufferMemReqsInfo.buffer = buffer[i];

        tileMemReqs = vku::InitStructHelper();
        buffer_mem_reqs2.pNext = &tileMemReqs;

        // VkTileMemoryRequirementsQCOM
        vk::GetBufferMemoryRequirements2(device(), &bufferMemReqsInfo, &buffer_mem_reqs2);
    }

    int mem_type_index = -1;
    uint32_t type_mask = buffer_mem_reqs2.memoryRequirements.memoryTypeBits;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heapIndex = memory_info.memoryTypes[i].heapIndex;
        if (((type_mask & 1) == 1) && (memory_info.memoryHeaps[heapIndex].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM)) {
            mem_type_index = i;
            break;
        }
        type_mask >>= 1;
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkMemoryAllocateInfo buffer_mem_alloc = vku::InitStructHelper();
    buffer_mem_alloc.allocationSize = tileMemReqs.size;
    buffer_mem_alloc.memoryTypeIndex = mem_type_index;

    VkDeviceMemory buffer_mem = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &buffer_mem_alloc, nullptr, &buffer_mem);
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-10740");
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-10742");
    vk::BindBufferMemory(device(), buffer[0], buffer_mem, 1);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), buffer_mem, nullptr);

    buffer_mem_alloc.allocationSize = tileMemReqs.size;
    vk::AllocateMemory(device(), &buffer_mem_alloc, nullptr, &buffer_mem);
    VkBindBufferMemoryInfo buffer_bind_info = {VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR, nullptr, buffer[1], buffer_mem, 1};

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-memory-10740");
    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-memory-10742");
    vk::BindBufferMemory2(device(), 1, &buffer_bind_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), buffer_mem, nullptr);

    for (int i = 0; i < 2; i++) {
        vk::DestroyBuffer(*m_device, buffer[i], NULL);
    }
}

TEST_F(NegativeTileMemoryHeap, BindBufferMemoryTestsNotFromTileMemoryHeap) {
    TEST_DESCRIPTION(
        "Allocate memory with a memory type index that does not correspond to a VkmemoryHeap with "
        "VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    auto buffer_ci = vkt::Buffer::CreateInfo(256, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    VkBuffer buffer[2];
    VkMemoryRequirements buffer_mem_reqs[2];
    for (int i = 0; i < 2; i++) {
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer[i]);

        // VkTileMemoryRequirementsQCOM
        vk::GetBufferMemoryRequirements(device(), buffer[i], &buffer_mem_reqs[i]);
    }

    int mem_type_index = -1;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    uint32_t type_mask = buffer_mem_reqs[0].memoryTypeBits;
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heapIndex = memory_info.memoryTypes[i].heapIndex;
        if (((type_mask & 1) == 1) && !(memory_info.memoryHeaps[heapIndex].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM)) {
            mem_type_index = i;
            break;
        }
        type_mask >>= 1;
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkMemoryAllocateInfo buffer_mem_alloc = vku::InitStructHelper();
    buffer_mem_alloc.allocationSize = buffer_mem_reqs[0].size;
    buffer_mem_alloc.memoryTypeIndex = mem_type_index;

    VkDeviceMemory buffer_mem = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &buffer_mem_alloc, nullptr, &buffer_mem);
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-None-10739");
    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-None-10741");
    vk::BindBufferMemory(device(), buffer[0], buffer_mem, 1);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), buffer_mem, nullptr);

    buffer_mem_alloc.allocationSize = buffer_mem_reqs[0].size;
    vk::AllocateMemory(device(), &buffer_mem_alloc, nullptr, &buffer_mem);
    VkBindBufferMemoryInfo buffer_bind_info = {VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR, nullptr, buffer[1], buffer_mem, 1};

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-None-10739");
    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-None-10741");
    vk::BindBufferMemory2(device(), 1, &buffer_bind_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), buffer_mem, nullptr);

    for (int i = 0; i < 2; i++) {
        vk::DestroyBuffer(*m_device, buffer[i], NULL);
    }
}

TEST_F(NegativeTileMemoryHeap, BindImageMemoryTestsFromTileMemoryHeap) {
    TEST_DESCRIPTION(
        "Allocate memory with a memory type index that corresponds to a VkmemoryHeap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format,
                                             VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    VkImage image[2];
    VkTileMemoryRequirementsQCOM tileMemReqs;
    VkMemoryRequirements2 image_mem_reqs2;
    tileMemReqs.sType = VK_STRUCTURE_TYPE_TILE_MEMORY_REQUIREMENTS_QCOM;
    for (int i = 0; i < 2; i++) {
        vk::CreateImage(device(), &image_ci, nullptr, &image[i]);
        image_mem_reqs2 = vku::InitStructHelper();
        VkImageMemoryRequirementsInfo2 imageMemReqsInfo = vku::InitStructHelper();
        imageMemReqsInfo.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
        imageMemReqsInfo.image = image[i];

        tileMemReqs = vku::InitStructHelper();
        image_mem_reqs2.pNext = &tileMemReqs;

        // VkTileMemoryRequirementsQCOM
        vk::GetImageMemoryRequirements2(device(), &imageMemReqsInfo, &image_mem_reqs2);
    }

    int mem_type_index = -1;
    uint32_t type_mask = image_mem_reqs2.memoryRequirements.memoryTypeBits;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heapIndex = memory_info.memoryTypes[i].heapIndex;
        if (((type_mask & 1) == 1) && (memory_info.memoryHeaps[heapIndex].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM)) {
            mem_type_index = i;
            break;
        }
        type_mask >>= 1;
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkMemoryAllocateInfo image_mem_alloc = vku::InitStructHelper();
    image_mem_alloc.allocationSize = tileMemReqs.size;
    image_mem_alloc.memoryTypeIndex = mem_type_index;

    VkDeviceMemory image_mem = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &image_mem_alloc, nullptr, &image_mem);
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-10736");
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-10738");
    vk::BindImageMemory(device(), image[0], image_mem, 1);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), image_mem, nullptr);

    image_mem_alloc.allocationSize = tileMemReqs.size;
    vk::AllocateMemory(device(), &image_mem_alloc, nullptr, &image_mem);
    VkBindImageMemoryInfo image_bind_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO_KHR, nullptr, image[1], image_mem, 1};

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01616");
    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01617");
    vk::BindImageMemory2(device(), 1, &image_bind_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), image_mem, nullptr);

    for (int i = 0; i < 2; i++) {
        vk::DestroyImage(*m_device, image[i], NULL);
    }
}

TEST_F(NegativeTileMemoryHeap, BindImageMemoryTestsNotFromTileMemoryHeap) {
    TEST_DESCRIPTION(
        "Allocate memory with a memory type index that does not correspond to a VkmemoryHeap with "
        "VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT);
    VkImage image[2];
    VkMemoryRequirements image_mem_reqs[2];
    for (int i = 0; i < 2; i++) {
        vk::CreateImage(device(), &image_ci, nullptr, &image[i]);

        // VkTileMemoryRequirementsQCOM
        vk::GetImageMemoryRequirements(device(), image[i], &image_mem_reqs[i]);
    }

    int mem_type_index = -1;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    uint32_t type_mask = image_mem_reqs[0].memoryTypeBits;
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heapIndex = memory_info.memoryTypes[i].heapIndex;
        if (((type_mask & 1) == 1) && !(memory_info.memoryHeaps[heapIndex].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM)) {
            mem_type_index = i;
            break;
        }
        type_mask >>= 1;
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkMemoryAllocateInfo image_mem_alloc = vku::InitStructHelper();
    image_mem_alloc.allocationSize = image_mem_reqs[0].size;
    image_mem_alloc.memoryTypeIndex = mem_type_index;

    VkDeviceMemory image_mem = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &image_mem_alloc, nullptr, &image_mem);
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-None-10735");
    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-None-10737");
    vk::BindImageMemory(device(), image[0], image_mem, 1);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), image_mem, nullptr);

    image_mem_alloc.allocationSize = image_mem_reqs[0].size;
    vk::AllocateMemory(device(), &image_mem_alloc, nullptr, &image_mem);
    VkBindImageMemoryInfo buffer_bind_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO_KHR, nullptr, image[1], image_mem, 1};

    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01616");
    m_errorMonitor->SetDesiredError("VUID-VkBindImageMemoryInfo-pNext-01617");
    vk::BindImageMemory2(device(), 1, &buffer_bind_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), image_mem, nullptr);

    for (int i = 0; i < 2; i++) {
        vk::DestroyImage(*m_device, image[i], NULL);
    }
}

TEST_F(NegativeTileMemoryHeap, DynamicRenderingSubpassDepthStencilAttachment) {
    TEST_DESCRIPTION(
        "Call Begin Rendering with a depth and stencil attachment resolveImageView that is mapped to a VkmemoryHeap with "
        "VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    VkFormat depth_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.depthAttachmentFormat = depth_format;
    pipeline_rendering_info.stencilAttachmentFormat = depth_format;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.ms_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe.ds_ci_ = vku::InitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe.gp_ci_.renderPass = VK_NULL_HANDLE;
    pipe.CreateGraphicsPipeline();

    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, depth_format,
                                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    VkDeviceMemory image_mem = VK_NULL_HANDLE;
    if (!AllocateTileImage(image, image_mem)) {
        GTEST_SKIP() << "Valid memory type index not found";
    }
    vkt::ImageView depth_image_view = image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkRenderingAttachmentInfo depth_attachment = vku::InitStructHelper();
    depth_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment.resolveImageView = depth_image_view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.pDepthAttachment = &depth_attachment;
    begin_rendering_info.pStencilAttachment = &depth_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-resolveImageView-10728");
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-resolveImageView-10728");
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
    vk::FreeMemory(device(), image_mem, NULL);
}

TEST_F(NegativeTileMemoryHeap, SubpassDescriptionResolveAttachment) {
    TEST_DESCRIPTION(
        "Call CreateFrameBuffer with resolve attachment that is mapped to a VkmemoryHeap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachment.samples = VK_SAMPLE_COUNT_4_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription resolveAttachment = {};
    resolveAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0u;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference resolve_attachment_reference;
    resolve_attachment_reference.attachment = 1u;
    resolve_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pResolveAttachments = &resolve_attachment_reference;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.colorAttachmentCount = 1;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, resolveAttachment};

    VkRenderPassCreateInfo rp_ci = vku::InitStructHelper();
    rp_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
    rp_ci.pAttachments = attachments.data();
    rp_ci.subpassCount = 1u;
    rp_ci.pSubpasses = &subpass;
    vkt::RenderPass render_pass(*m_device, rp_ci);

    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    VkDeviceMemory image_mem = VK_NULL_HANDLE;
    if (!AllocateTileImage(image, image_mem)) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent.width = 800;
    imageCreateInfo.extent.height = 600;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_4_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkt::Image color_image(*m_device, imageCreateInfo);
    vkt::ImageView color_image_view = color_image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    vkt::ImageView image_view = image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
    std::array<VkImageView, 2> renderpass_image_views = {color_image_view, image_view};
    VkFramebufferCreateInfo framebuffer_ci = vku::InitStructHelper();
    framebuffer_ci.renderPass = render_pass;
    framebuffer_ci.attachmentCount = 2u;
    framebuffer_ci.pAttachments = renderpass_image_views.data();
    framebuffer_ci.width = 32u;
    framebuffer_ci.height = 32u;
    framebuffer_ci.layers = 1u;

    VkFramebuffer framebuffer;
    m_errorMonitor->SetDesiredError("VUID-VkSubpassDescription-attachment-10755");
    vk::CreateFramebuffer(device(), &framebuffer_ci, nullptr, &framebuffer);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(device(), image_mem, NULL);
    vk::DestroyFramebuffer(device(), framebuffer, NULL);
}

TEST_F(NegativeTileMemoryHeap, DrawDispatchTileImage) {
    TEST_DESCRIPTION(
        "Call cmdDraw with image that is mapped to a VkmemoryHeap with VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM but is not the active "
        "tile memory mapping");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    int mem_type_index = -1;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heap_index = memory_info.memoryTypes[i].heapIndex;
        if (memory_info.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM) {
            mem_type_index = i;
            break;
        }
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkMemoryAllocateInfo tile_mem_alloc_info = vku::InitStructHelper();
    tile_mem_alloc_info.memoryTypeIndex = mem_type_index;
    tile_mem_alloc_info.allocationSize = 1024;
    VkDeviceMemory tile_mem = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &tile_mem_alloc_info, nullptr, &tile_mem);
    VkTileMemoryBindInfoQCOM bind_info = vku::InitStructHelper();
    bind_info.memory = tile_mem;
    bind_info.sType = VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM;

    image_ci = vkt::Image::ImageCreateInfo2D(width, height, 1, 1, format,
                                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image image(*m_device, image_ci, vkt::no_mem);
    VkDeviceMemory image_mem = VK_NULL_HANDLE;
    if (!AllocateTileImage(image, image_mem)) {
        GTEST_SKIP() << "Valid memory type index not found";
    }
    vkt::ImageView depth_image_view = image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT);

    VkFormat color_formats = format;

    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = depth_image_view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindTileMemoryQCOM(m_command_buffer, &bind_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-commandBuffer-10746");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();
    m_command_buffer.End();
    vk::FreeMemory(device(), image_mem, NULL);
    vk::FreeMemory(device(), tile_mem, NULL);
}

TEST_F(NegativeTileMemoryHeap, SecondaryCommandBufferTileMemory) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    RETURN_IF_SKIP(Init());

    VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = vku::InitStructHelper();
    secondary_cmd_buffer_alloc_info.commandPool = m_command_pool;
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);

    int mem_type_index = -1;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; i++) {
        int heap_index = memory_info.memoryTypes[i].heapIndex;
        if (memory_info.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM) {
            mem_type_index = i;
            break;
        }
    }

    if (mem_type_index == -1) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    VkMemoryAllocateInfo tile_mem_alloc_info = vku::InitStructHelper();
    tile_mem_alloc_info.memoryTypeIndex = mem_type_index;
    tile_mem_alloc_info.allocationSize = 1024;
    VkDeviceMemory tile_mem1 = VK_NULL_HANDLE, tile_mem2 = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &tile_mem_alloc_info, nullptr, &tile_mem1);
    tile_mem_alloc_info.allocationSize = 512;
    vk::AllocateMemory(device(), &tile_mem_alloc_info, nullptr, &tile_mem2);
    VkTileMemoryBindInfoQCOM bind_info = vku::InitStructHelper();
    bind_info.memory = tile_mem1;
    bind_info.sType = VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM;

    VkTileMemoryBindInfoQCOM bind_info2 = vku::InitStructHelper();
    bind_info2.memory = tile_mem2;
    bind_info2.sType = VK_STRUCTURE_TYPE_TILE_MEMORY_BIND_INFO_QCOM;

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper();
    cmd_buffer_inheritance_info.pNext = &bind_info2;

    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper{};
    secondary_cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    secondary_cmd_buffer.Begin(&secondary_cmd_buffer_begin_info);
    secondary_cmd_buffer.End();
    m_command_buffer.Begin();

    vk::CmdBindTileMemoryQCOM(m_command_buffer, &bind_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-memory-10724");
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary_cmd_buffer.handle());
    m_errorMonitor->VerifyFound();
    vk::FreeMemory(device(), tile_mem1, NULL);
    vk::FreeMemory(device(), tile_mem2, NULL);
}