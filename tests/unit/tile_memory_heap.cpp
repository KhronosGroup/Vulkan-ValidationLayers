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
    VkMemoryRequirements2 buffer_reqs = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    buffer_info.buffer = buffer;
    buffer_reqs.pNext = &tile_mem_reqs;
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
    VkMemoryRequirements2 buffer_reqs = vku::InitStructHelper();
    VkTileMemoryRequirementsQCOM tile_mem_reqs = vku::InitStructHelper();
    buffer_info.buffer = buffer;
    buffer_reqs.pNext = &tile_mem_reqs;
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