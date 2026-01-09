/*
 * Copyright (c) 2023-2026 LunarG, Inc.
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
    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-flags-09641");
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

    m_errorMonitor->SetDesiredError("VUID-vkAllocateMemory-tileMemoryHeap-10976");
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

TEST_F(NegativeTileMemoryHeap, TileMemoryMismatchResourceCommandBuffer) {
    TEST_DESCRIPTION("Mismatch VkDeviceMemory between underlying Resource and Command Buffer");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
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
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = tile_mem_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(image_reqs.memoryRequirements.memoryTypeBits, &alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    vkt::DeviceMemory image_memory(*m_device, alloc_info);
    vk::BindImageMemory(device(), image, image_memory, 0);
    vkt::ImageView image_view = image.CreateView();

    VkFormat color_formats = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo pipeline_rendering_info = vku::InitStructHelper();
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &color_formats;

    CreatePipelineHelper pipe(*this, &pipeline_rendering_info);
    pipe.CreateGraphicsPipeline();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = image_view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {32, 32}};

    // Dynamic Rendering
    m_command_buffer.Begin();
    m_command_buffer.BeginRendering(begin_rendering_info);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-commandBuffer-10746");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRendering();

    VkAttachmentReference2 attachment_ref = vku::InitStructHelper();
    attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    attachment_ref.attachment = 0;

    VkSubpassDescription2 subpass[2] = {};
    subpass[0] = vku::InitStructHelper();
    subpass[0].colorAttachmentCount = 1;
    subpass[0].pColorAttachments = &attachment_ref;
    subpass[1] = vku::InitStructHelper();
    subpass[1].colorAttachmentCount = 1;
    subpass[1].pColorAttachments = &attachment_ref;

    VkAttachmentDescription2 attach_desc = vku::InitStructHelper();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rp = vku::InitStructHelper();
    rp.subpassCount = 2;
    rp.pSubpasses = &subpass[0];
    rp.attachmentCount = 1;
    rp.pAttachments = &attach_desc;
    vkt::RenderPass test_rp(*m_device, rp);

    VkImageView fb_image_view = image_view;
    auto frame_buffer_create_info =
        vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, test_rp.handle(), 1u, &fb_image_view, 32u, 32u, 1u);
    vkt::Framebuffer fb(*m_device, frame_buffer_create_info);
    CreatePipelineHelper render_pass_pipe(*this);
    render_pass_pipe.gp_ci_.renderPass = test_rp;
    render_pass_pipe.gp_ci_.subpass = 0;
    render_pass_pipe.CreateGraphicsPipeline();

    // Render Pass
    m_command_buffer.BeginRenderPass(test_rp, fb, 32u, 32u, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_pass_pipe);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-commandBuffer-10746");
    vk::CmdDraw(m_command_buffer, 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, SecondaryCommandBufferTileMemory) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    RETURN_IF_SKIP(Init());

    auto image_create_info =
        vkt::Image::ImageCreateInfo2D(32u, 32u, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    vkt::Image image(*m_device, image_create_info);

    VkCommandBufferAllocateInfo secondary_cmd_buffer_alloc_info = vku::InitStructHelper();
    secondary_cmd_buffer_alloc_info.commandPool = m_command_pool;
    secondary_cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    secondary_cmd_buffer_alloc_info.commandBufferCount = 1;

    vkt::CommandBuffer secondary_cmd_buffer(*m_device, secondary_cmd_buffer_alloc_info);
    VkPhysicalDeviceMemoryProperties memory_info;
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
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    VkMemoryAllocateInfo tile_mem_alloc_info = vku::InitStructHelper();
    tile_mem_alloc_info.memoryTypeIndex = mem_type_index;
    tile_mem_alloc_info.allocationSize = 1024;
    vkt::DeviceMemory tile_mem1(*m_device, tile_mem_alloc_info);
    tile_mem_alloc_info.allocationSize = 512;
    vkt::DeviceMemory tile_mem2(*m_device, tile_mem_alloc_info);
    VkTileMemoryBindInfoQCOM bind_info = vku::InitStructHelper();
    bind_info.memory = tile_mem1;

    VkTileMemoryBindInfoQCOM tile_memory_bind_info = vku::InitStructHelper();
    tile_memory_bind_info.memory = tile_mem2;

    VkFormat color_formats = VK_FORMAT_R8G8B8A8_UNORM;

    VkCommandBufferInheritanceRenderingInfo inheritance_rendering_info = vku::InitStructHelper(&tile_memory_bind_info);
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    inheritance_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;

    VkCommandBufferInheritanceInfo cmd_buffer_inheritance_info = vku::InitStructHelper(&inheritance_rendering_info);
    VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info = vku::InitStructHelper{};
    secondary_cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary_cmd_buffer_begin_info.pInheritanceInfo = &cmd_buffer_inheritance_info;

    secondary_cmd_buffer.Begin(&secondary_cmd_buffer_begin_info);
    secondary_cmd_buffer.End();

    vkt::ImageView image_view = image.CreateView();
    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = image_view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {32, 32}};

    m_command_buffer.Begin();
    vk::CmdBindTileMemoryQCOM(m_command_buffer, &bind_info);
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredError("VUID-vkCmdExecuteCommands-memory-10724");
    vk::CmdExecuteCommands(m_command_buffer, 1, &secondary_cmd_buffer.handle());
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

TEST_F(NegativeTileMemoryHeap, ResolveAttachment) {
    TEST_DESCRIPTION("Resolve to a Tile Memory Attachment.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_QCOM_TILE_MEMORY_HEAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::tileMemoryHeap);
    AddRequiredFeature(vkt::Feature::dynamicRendering);
    AddRequiredFeature(vkt::Feature::imagelessFramebuffer);

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkt::Image msaa_image(*m_device, image_create_info, vkt::set_layout);
    vkt::ImageView msaa_image_view = msaa_image.CreateView();

    // Create a Tile Memory Image
    auto tile_image_create_info = vkt::Image::ImageCreateInfo2D(
        32u, 32u, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM);
    vkt::Image tile_memory_image(*m_device, tile_image_create_info, vkt::no_mem);

    // Query Tile Memory Image Requirements
    VkImageMemoryRequirementsInfo2 image_info = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    VkMemoryRequirements2 image_reqs = vku::InitStructHelper(&tile_mem_reqs);
    image_info.image = tile_memory_image;
    vk::GetImageMemoryRequirements2(device(), &image_info, &image_reqs);

    if (tile_mem_reqs.size == 0) {
        GTEST_SKIP() << "Image not eligible to be bound with Tile Memory.";
    }

    // Find a memory configuration for the Tile Memory Image, otherwise exit
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = tile_mem_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(image_reqs.memoryRequirements.memoryTypeBits, &alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    vkt::DeviceMemory image_memory(*m_device, alloc_info);
    vk::BindImageMemory(device(), tile_memory_image, image_memory, 0);

    vkt::ImageView resolve_image_view = tile_memory_image.CreateView();

    VkRenderingAttachmentInfo color_attachment = vku::InitStructHelper();
    color_attachment.imageView = msaa_image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment.resolveImageView = resolve_image_view;

    VkRenderingInfo begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {1, 1}};

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkRenderingAttachmentInfo-resolveImageView-10728");
    m_command_buffer.BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();
    m_command_buffer.End();
    m_command_buffer.Reset();

    VkAttachmentReference2 msaa_attachment_ref = vku::InitStructHelper();
    msaa_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    msaa_attachment_ref.attachment = 0;

    VkAttachmentReference2 resolve_attachment_ref = vku::InitStructHelper();
    resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment_ref.attachment = 1;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &msaa_attachment_ref;
    subpass.pResolveAttachments = &resolve_attachment_ref;

    VkAttachmentDescription2 attach_desc[2] = {};
    attach_desc[0] = vku::InitStructHelper();
    attach_desc[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attach_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[1] = vku::InitStructHelper();
    attach_desc[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rp = vku::InitStructHelper();
    rp.subpassCount = 1;
    rp.pSubpasses = &subpass;
    rp.attachmentCount = 2;
    rp.pAttachments = &attach_desc[0];
    vkt::RenderPass test_rp(*m_device, rp);

    VkImageView views[2] = {};
    views[0] = msaa_image_view;
    views[1] = resolve_image_view;
    auto frame_buffer_create_info =
        vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, test_rp.handle(), 2u, &views[0], 32u, 32u, 1u);
    VkFramebuffer fb;

    m_errorMonitor->SetDesiredError("UNASSIGNED-VkFramebufferCreateInfo-attachment");
    vk::CreateFramebuffer(device(), &frame_buffer_create_info, nullptr, &fb);
    m_errorMonitor->VerifyFound();

    // Imageless FBO
    VkFormat framebuffer_attachment_formats = VK_FORMAT_R8G8B8A8_UNORM;
    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info[2] = {};
    framebuffer_attachment_image_info[0] = vku::InitStructHelper();
    ;
    framebuffer_attachment_image_info[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info[0].width = 32;
    framebuffer_attachment_image_info[0].height = 32;
    framebuffer_attachment_image_info[0].layerCount = 1;
    framebuffer_attachment_image_info[0].viewFormatCount = 1;
    framebuffer_attachment_image_info[0].pViewFormats = &framebuffer_attachment_formats;
    framebuffer_attachment_image_info[1] = vku::InitStructHelper();
    ;
    framebuffer_attachment_image_info[1].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TILE_MEMORY_BIT_QCOM;
    framebuffer_attachment_image_info[1].width = 32;
    framebuffer_attachment_image_info[1].height = 32;
    framebuffer_attachment_image_info[1].layerCount = 1;
    framebuffer_attachment_image_info[1].viewFormatCount = 1;
    framebuffer_attachment_image_info[1].pViewFormats = &framebuffer_attachment_formats;
    VkFramebufferAttachmentsCreateInfo framebuffer_attachment_ci = vku::InitStructHelper();
    framebuffer_attachment_ci.attachmentImageInfoCount = 2;
    framebuffer_attachment_ci.pAttachmentImageInfos = &framebuffer_attachment_image_info[0];

    auto imageless_fbci = vku::InitStruct<VkFramebufferCreateInfo>(nullptr, 0u, test_rp.handle(), 2u, nullptr, 32u, 32u, 1u);
    imageless_fbci.pNext = &framebuffer_attachment_ci;
    imageless_fbci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    vkt::Framebuffer imageless_fb(*m_device, imageless_fbci);

    VkImageView image_views[2] = {msaa_image_view, resolve_image_view};
    VkRenderPassAttachmentBeginInfo rp_attachment_begin_info = vku::InitStructHelper();
    rp_attachment_begin_info.attachmentCount = 2;
    rp_attachment_begin_info.pAttachments = &image_views[0];
    VkRenderPassBeginInfo rp_begin_info = vku::InitStructHelper(&rp_attachment_begin_info);
    rp_begin_info.renderPass = test_rp.handle();
    rp_begin_info.renderArea.extent = {32, 32};
    rp_begin_info.framebuffer = imageless_fb;

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("UNASSIGNED-VkRenderPassBeginInfo-framebuffer");
    m_command_buffer.BeginRenderPass(rp_begin_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTileMemoryHeap, BufferMismatchTileMemBound) {
    TEST_DESCRIPTION("Create tile memory storage buffer and use it within a dispatch.");
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
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = tile_mem_reqs.size;
    bool pass = m_device->Physical().SetMemoryType(buffer_reqs.memoryRequirements.memoryTypeBits, &alloc_info,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, VK_MEMORY_HEAP_TILE_MEMORY_BIT_QCOM);

    if (!pass) {
        GTEST_SKIP() << "Could not find an eligible Tile Memory Type.";
    }

    // Bind Tile Memory to Buffer
    vkt::DeviceMemory buffer_memory(*m_device, alloc_info);
    vk::BindBufferMemory(device(), buffer, buffer_memory, 0);

    // Create Compute Shader to write to Tile Memory Buffer
    const char *cs_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer ssbo { float tileMemBuffer; };
        void main() {
           tileMemBuffer = 1.0f;
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.CreateComputePipeline();
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, buffer, 0, 4096, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdDispatch-commandBuffer-10746");
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_errorMonitor->VerifyFound();
}