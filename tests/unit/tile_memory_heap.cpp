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
#include "../framework/render_pass_helper.h"

class NegativeTileMemoryHeap : public TileMemoryHeapTest {};

TEST_F(NegativeTileMemoryHeap, CreateBufferTest) {
    TEST_DESCRIPTION("Use VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM without enabling the tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-tileMemoryHeap-10762");
}

TEST_F(NegativeTileMemoryHeap, CreateBufferProtectedMemoryFlag) {
    TEST_DESCRIPTION("Test Protected flag in CreateBuffer with tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::protectedMemory);

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM;
    buffer_ci.flags = VK_BUFFER_CREATE_PROTECTED_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10763");
}

TEST_F(NegativeTileMemoryHeap, CreateBufferTestIndexUsageFlags) {
    TEST_DESCRIPTION("Test Index Usage Flag in CreateBuffer with tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_TILE_MEMORY_BIT_QCOM | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    CreateBufferTest(buffer_ci, "VUID-VkBufferCreateInfo-usage-10764");
}

TEST_F(NegativeTileMemoryHeap, AllocateMemory) {
    TEST_DESCRIPTION("Allocate Tile Memory without the Tile Memory feature enabled.");

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 256;

    bool pass = m_device->Physical().SetMemoryType(0xFFFFFFFF, &alloc_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0,
                                                   VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);
    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    m_errorMonitor->SetDesiredError("VUID-VkTileMemoryBindInfoQCOM-memoryTypeIndex-10976");
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, BindBufferMemorySize) {
    TEST_DESCRIPTION("Bind Tile Memory to a Buffer with too small of size");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    // Create a Tile Memory Buffer
    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_2_TILE_MEMORY_BIT_QCOM | VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT),
                       vkt::no_mem);

    // Query Tile Memory Buffer Requirements
    VkBufferMemoryRequirementsInfo2 buffer_info = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    VkMemoryRequirements2 buffer_reqs = vku::InitStructHelper(&tile_mem_reqs);
    buffer_info.buffer = buffer;
    vk::GetBufferMemoryRequirements2(device(), &buffer_info, &buffer_reqs);

    if (tile_mem_reqs.size == 0) {
        GTEST_SKIP() << "Buffer not eligible to be bound with Tile Memory.";
    }

    // Find a memory configuration for the Tile Memory Buffer, otherwise exit
    VkMemoryAllocateInfo bad_alloc_info = vku::InitStructHelper();
    bad_alloc_info.memoryTypeIndex = 0;
    // Purposely subtract 1 from size
    bad_alloc_info.allocationSize = tile_mem_reqs.size - 1;
    bool pass = m_device->Physical().SetMemoryType(buffer_reqs.memoryRequirements.memoryTypeBits, &bad_alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    vkt::DeviceMemory buffer_memory(*m_device, bad_alloc_info);

    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-10742");
    vk::BindBufferMemory(device(), buffer, buffer_memory, 0);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer;
    bind_buffer_info.memory = buffer_memory;
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-memory-10742");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, BindBufferMemoryAlignment) {
    TEST_DESCRIPTION("Bind Tile Memory to a Buffer with an offset that is not a multiple of alignment");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    // Create a Tile Memory Buffer
    vkt::Buffer buffer(*m_device,
                       vkt::Buffer::CreateInfo(4096, VK_BUFFER_USAGE_2_TILE_MEMORY_BIT_QCOM | VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT),
                       vkt::no_mem);

    // Query Tile Memory Buffer Requirements
    VkBufferMemoryRequirementsInfo2 buffer_info = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    VkMemoryRequirements2 buffer_reqs = vku::InitStructHelper(&tile_mem_reqs);
    buffer_info.buffer = buffer;
    vk::GetBufferMemoryRequirements2(device(), &buffer_info, &buffer_reqs);

    if (tile_mem_reqs.size == 0) {
        GTEST_SKIP() << "Buffer not eligible to be bound with Tile Memory.";
    }

    // Find a memory configuration for the Tile Memory Buffer, otherwise exit
    const uint32_t badOffset = 1;
    VkMemoryAllocateInfo bad_alloc_info = vku::InitStructHelper();
    bad_alloc_info.memoryTypeIndex = 0;
    // Purposely add 1 to size for offset
    bad_alloc_info.allocationSize = tile_mem_reqs.size + badOffset;
    bool pass = m_device->Physical().SetMemoryType(buffer_reqs.memoryRequirements.memoryTypeBits, &bad_alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    vkt::DeviceMemory buffer_memory(*m_device, bad_alloc_info);

    m_errorMonitor->SetDesiredError("VUID-vkBindBufferMemory-memory-10740");
    vk::BindBufferMemory(device(), buffer, buffer_memory, badOffset);
    m_errorMonitor->VerifyFound();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper();
    bind_buffer_info.buffer = buffer;
    bind_buffer_info.memory = buffer_memory;
    bind_buffer_info.memoryOffset = badOffset;

    m_errorMonitor->SetDesiredError("VUID-VkBindBufferMemoryInfo-memory-10740");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, CreateImageTest) {
    TEST_DESCRIPTION("Use VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM in CreateImage without enabling the tileMemoryHeap feature");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);

    RETURN_IF_SKIP(Init());

    CreateImageTest(vkt::Image::ImageCreateInfo2D(256, 256, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM),
                    "VUID-VkImageCreateInfo-tileMemoryHeap-10766");
}

TEST_F(NegativeTileMemoryHeap, BindImageMemorySize) {
    TEST_DESCRIPTION("Bind Tile Memory to an Image with too small of size");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    // Create a Tile Memory Image
    auto image_create_info = vkt::Image::ImageCreateInfo2D(
        32u, 32u, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    // Query Tile Memory Image Requirements
    VkImageMemoryRequirementsInfo2 image_info = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    VkMemoryRequirements2 image_reqs = vku::InitStructHelper(&tile_mem_reqs);
    image_info.image = image;
    vk::GetImageMemoryRequirements2(device(), &image_info, &image_reqs);

    if (tile_mem_reqs.size == 0) {
        GTEST_SKIP() << "Image not eligible to be bound with Tile Memory.";
    }

    // Find a memory configuration for the Tile Memory Image, otherwise exit
    VkMemoryAllocateInfo bad_alloc_info = vku::InitStructHelper();
    bad_alloc_info.memoryTypeIndex = 0;
    // Purposely subtract 1 from size
    bad_alloc_info.allocationSize = tile_mem_reqs.size - 1;
    bool pass = m_device->Physical().SetMemoryType(image_reqs.memoryRequirements.memoryTypeBits, &bad_alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    vkt::DeviceMemory image_memory(*m_device, bad_alloc_info);

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-10738");
    vk::BindImageMemory(device(), image, image_memory, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, BindImageMemoryAlignment) {
    TEST_DESCRIPTION("Bind Tile Memory to an Image with an offset that is not a multiple of alignment");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    // Create a Tile Memory Image
    auto image_create_info = vkt::Image::ImageCreateInfo2D(
        32u, 32u, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    // Query Tile Memory Image Requirements
    VkImageMemoryRequirementsInfo2 image_info = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    VkMemoryRequirements2 image_reqs = vku::InitStructHelper(&tile_mem_reqs);
    image_info.image = image;
    vk::GetImageMemoryRequirements2(device(), &image_info, &image_reqs);

    if (tile_mem_reqs.size == 0) {
        GTEST_SKIP() << "Image not eligible to be bound with Tile Memory.";
    }

    // Find a memory configuration for the Tile Memory Image, otherwise exit
    const uint32_t badOffset = 1;
    VkMemoryAllocateInfo bad_alloc_info = vku::InitStructHelper();
    bad_alloc_info.memoryTypeIndex = 0;
    // Purposely add 1 to size for offset
    bad_alloc_info.allocationSize = tile_mem_reqs.size + badOffset;
    bool pass = m_device->Physical().SetMemoryType(image_reqs.memoryRequirements.memoryTypeBits, &bad_alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    vkt::DeviceMemory image_memory(*m_device, bad_alloc_info);

    m_errorMonitor->SetDesiredError("VUID-vkBindImageMemory-memory-10736");
    vk::BindImageMemory(device(), image, image_memory, badOffset);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, BindNonTileMemoryCommandBuffer) {
    TEST_DESCRIPTION("Bind non Tile Memory with vkCmdBindTileMemoryQCOM in Primary/Secondary Command Buffer.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);

    RETURN_IF_SKIP(Init());

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 256;
    mem_alloc.memoryTypeIndex = 0;
    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);

    uint32_t i = 0;
    for (; i < memory_info.memoryTypeCount; i++) {
        // Would require deviceCoherentMemory feature
        if (memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
            continue;
        }
        // Would require protected feature
        if (memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            continue;
        }
        if (!(memory_info.memoryHeaps[memory_info.memoryTypes[i].heapIndex].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM)) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }

    if (i >= memory_info.memoryTypeCount) {
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    vkt::DeviceMemory non_tile_memory(*m_device, mem_alloc);

    VkTileMemoryBindInfoQCOM tile_mem_bind_info = vku::InitStructHelper();
    tile_mem_bind_info.memory = non_tile_memory;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkTileMemoryBindInfoQCOM-memory-10726");
    vk::CmdBindTileMemoryQCOM(m_command_buffer, &tile_mem_bind_info);
    m_errorMonitor->VerifyFound();

    const VkCommandBufferInheritanceInfo cmdbuff_ii = vku::InitStructHelper(&tile_mem_bind_info);
    VkCommandBufferBeginInfo cmdbuff_bi = vku::InitStructHelper();
    cmdbuff_bi.pInheritanceInfo = &cmdbuff_ii;
    vkt::CommandBuffer secondary(*m_device, m_command_pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    m_errorMonitor->SetDesiredError("VUID-VkTileMemoryBindInfoQCOM-memory-10726");
    vk::BeginCommandBuffer(secondary, &cmdbuff_bi);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, TileProperties) {
    TEST_DESCRIPTION("Provide a Tile Memory size that is greater than the largest Tile Memory heap.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_TILE_PROPERTIES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::tileProperties);

    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(Gpu(), &memory_info);
    uint64_t max_tile_memory_heap_size = 0;

    uint32_t i = 0;
    for (; i < memory_info.memoryHeapCount; i++) {
        if (memory_info.memoryHeaps[i].flags & VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM) {
            max_tile_memory_heap_size = std::max(memory_info.memoryHeaps[i].size, max_tile_memory_heap_size);
        }
    }

    if (max_tile_memory_heap_size == 0xFFFFFFFFFFFFFFFF) {
        GTEST_SKIP() << "Tile Memory heap exposes max 64 bit value.";
    }

    VkTilePropertiesQCOM tile_properties = vku::InitStructHelper();
    VkTileMemorySizeInfoQCOM tile_memory_size_info = vku::InitStructHelper();
    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    tile_memory_size_info.size = max_tile_memory_heap_size + 1;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper(&tile_memory_size_info);
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_errorMonitor->SetDesiredError("VUID-VkTileMemorySizeInfoQCOM-size-10729");
    vk::GetDynamicRenderingTilePropertiesQCOM(device(), &begin_rendering_info, &tile_properties);
    m_errorMonitor->VerifyFound();

    VkAttachmentDescriptionStencilLayout attachment_desc_stencil_layout = vku::InitStructHelper();
    attachment_desc_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReferenceStencilLayout attachment_ref_stencil_layout = vku::InitStructHelper();
    attachment_ref_stencil_layout.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;

    RenderPass2SingleSubpass rp2(*this);
    rp2.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp2.AddAttachmentReference(0, VK_IMAGE_LAYOUT_GENERAL);

    m_errorMonitor->SetDesiredError("VUID-VkTileMemorySizeInfoQCOM-size-10729");
    rp2.CreateRenderPass(&tile_memory_size_info);
    m_errorMonitor->VerifyFound();

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(VK_FORMAT_R8_UINT);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_GENERAL});

    m_errorMonitor->SetDesiredError("VUID-VkTileMemorySizeInfoQCOM-size-10729");
    rp.CreateRenderPass(&tile_memory_size_info);
    m_errorMonitor->VerifyFound();
}