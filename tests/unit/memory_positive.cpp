/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 * Copyright (c) 2023 Collabora, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"

TEST_F(PositiveMemory, MapMemory2) {
    TEST_DESCRIPTION("Validate vkMapMemory2 and vkUnmapMemory2");

    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);

    RETURN_IF_SKIP(Init())

    /* Vulkan doesn't have any requirements on what allocationSize can be
     * other than that it must be non-zero.  Pick 64KB because that should
     * work out to an even number of pages on basically any GPU.
     */
    const VkDeviceSize allocation_size = 64 << 10;

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = allocation_size;

    bool pass = m_device->phy().set_memory_type(vvl::kU32Max, &memory_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_TRUE(pass);

    VkDeviceMemory memory;
    VkResult err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &memory);
    ASSERT_EQ(VK_SUCCESS, err);

    VkMemoryMapInfoKHR map_info = vku::InitStructHelper();
    map_info.memory = memory;
    map_info.offset = 0;
    map_info.size = memory_info.allocationSize;

    VkMemoryUnmapInfoKHR unmap_info = vku::InitStructHelper();
    unmap_info.memory = memory;

    uint32_t *pData = NULL;
    err = vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    ASSERT_TRUE(pData != NULL);

    err = vk::UnmapMemory2KHR(m_device->device(), &unmap_info);
    ASSERT_EQ(VK_SUCCESS, err);

    map_info.size = VK_WHOLE_SIZE;

    pData = NULL;
    err = vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    ASSERT_TRUE(pData != NULL);

    err = vk::UnmapMemory2KHR(m_device->device(), &unmap_info);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::FreeMemory(m_device->device(), memory, NULL);
}

TEST_F(PositiveMemory, GetMemoryRequirements2) {
    TEST_DESCRIPTION(
        "Get memory requirements with VK_KHR_get_memory_requirements2 instead of core entry points and verify layers do not emit "
        "errors when objects are bound and used");

    AddRequiredExtensions(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    // Create a test buffer
    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device,
                       vkt::Buffer::create_info(1024, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT));

    // Use extension to get buffer memory requirements
    VkBufferMemoryRequirementsInfo2KHR buffer_info = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr,
                                                      buffer.handle()};
    VkMemoryRequirements2KHR buffer_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR};
    vk::GetBufferMemoryRequirements2KHR(m_device->device(), &buffer_info, &buffer_reqs);

    // Allocate and bind buffer memory
    vkt::DeviceMemory buffer_memory;
    buffer_memory.init(*m_device, vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer_reqs.memoryRequirements, 0));
    vk::BindBufferMemory(m_device->device(), buffer.handle(), buffer_memory.handle(), 0);

    // Create a test image
    auto image_ci = vkt::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image;
    image.init_no_mem(*m_device, image_ci);

    // Use extension to get image memory requirements
    VkImageMemoryRequirementsInfo2KHR image_info = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR, nullptr,
                                                    image.handle()};
    VkMemoryRequirements2KHR image_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR};
    vk::GetImageMemoryRequirements2KHR(m_device->device(), &image_info, &image_reqs);

    // Allocate and bind image memory
    vkt::DeviceMemory image_memory;
    image_memory.init(*m_device, vkt::DeviceMemory::get_resource_alloc_info(*m_device, image_reqs.memoryRequirements, 0));
    vk::BindImageMemory(m_device->device(), image.handle(), image_memory.handle(), 0);

    // Now execute arbitrary commands that use the test buffer and image
    m_commandBuffer->begin();

    // Fill buffer with 0
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_WHOLE_SIZE, 0);

    // Transition and clear image
    const auto subresource_range = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    const auto barrier = image.image_memory_barrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                    VK_IMAGE_LAYOUT_GENERAL, subresource_range);
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    const VkClearColorValue color = {};
    vk::CmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresource_range);

    // Submit and verify no validation errors
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
}

TEST_F(PositiveMemory, BindMemory2) {
    TEST_DESCRIPTION(
        "Bind memory with VK_KHR_bind_memory2 instead of core entry points and verify layers do not emit errors when objects are "
        "used");

    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    // Create a test buffer
    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device, vkt::Buffer::create_info(1024, VK_BUFFER_USAGE_TRANSFER_DST_BIT));

    // Allocate buffer memory
    vkt::DeviceMemory buffer_memory;
    buffer_memory.init(*m_device, vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(), 0));

    // Bind buffer memory with extension
    VkBindBufferMemoryInfoKHR buffer_bind_info = {VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR, nullptr, buffer.handle(),
                                                  buffer_memory.handle(), 0};
    vk::BindBufferMemory2KHR(m_device->device(), 1, &buffer_bind_info);

    // Create a test image
    auto image_ci = vkt::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image;
    image.init_no_mem(*m_device, image_ci);

    // Allocate image memory
    vkt::DeviceMemory image_memory;
    image_memory.init(*m_device, vkt::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), 0));

    // Bind image memory with extension
    VkBindImageMemoryInfoKHR image_bind_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO_KHR, nullptr, image.handle(),
                                                image_memory.handle(), 0};
    vk::BindImageMemory2KHR(m_device->device(), 1, &image_bind_info);

    // Now execute arbitrary commands that use the test buffer and image
    m_commandBuffer->begin();

    // Fill buffer with 0
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer.handle(), 0, VK_WHOLE_SIZE, 0);

    // Transition and clear image
    const auto subresource_range = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    const auto barrier = image.image_memory_barrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                    VK_IMAGE_LAYOUT_GENERAL, subresource_range);
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    const VkClearColorValue color = {};
    vk::CmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color, 1, &subresource_range);

    // Submit and verify no validation errors
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
}

TEST_F(PositiveMemory, NonCoherentMapping) {
    TEST_DESCRIPTION(
        "Ensure that validations handling of non-coherent memory mapping while using VK_WHOLE_SIZE does not cause access "
        "violations");
    VkResult err;
    uint8_t *pData;
    RETURN_IF_SKIP(Init())

    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;
    mem_reqs.memoryTypeBits = 0xFFFFFFFF;
    const VkDeviceSize atom_size = m_device->phy().limits_.nonCoherentAtomSize;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;

    static const VkDeviceSize allocation_size = 32 * atom_size;
    alloc_info.allocationSize = allocation_size;

    // Find a memory configurations WITHOUT a COHERENT bit, otherwise exit
    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!pass) {
        pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (!pass) {
            pass = m_device->phy().set_memory_type(
                mem_reqs.memoryTypeBits, &alloc_info,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            if (!pass) {
                GTEST_SKIP() << "Couldn't find a memory type wihtout a COHERENT bit";
            }
        }
    }

    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_EQ(VK_SUCCESS, err);

    // Map/Flush/Invalidate using WHOLE_SIZE and zero offsets and entire mapped range
    err = vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    VkMappedMemoryRange mmr = vku::InitStructHelper();
    mmr.memory = mem;
    mmr.offset = 0;
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::InvalidateMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(m_device->device(), mem);

    // Map/Flush/Invalidate using WHOLE_SIZE and an offset and entire mapped range
    err = vk::MapMemory(m_device->device(), mem, 5 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.memory = mem;
    mmr.offset = 6 * atom_size;
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::InvalidateMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(m_device->device(), mem);

    // Map with offset and size
    // Flush/Invalidate subrange of mapped area with offset and size
    err = vk::MapMemory(m_device->device(), mem, 3 * atom_size, 9 * atom_size, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.memory = mem;
    mmr.offset = 4 * atom_size;
    mmr.size = 2 * atom_size;
    err = vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::InvalidateMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(m_device->device(), mem);

    // Map without offset and flush WHOLE_SIZE with two separate offsets
    err = vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.memory = mem;
    mmr.offset = allocation_size - (4 * atom_size);
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    mmr.offset = allocation_size - (6 * atom_size);
    mmr.size = VK_WHOLE_SIZE;
    err = vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    ASSERT_EQ(VK_SUCCESS, err);
    vk::UnmapMemory(m_device->device(), mem);

    vk::FreeMemory(m_device->device(), mem, NULL);
}

TEST_F(PositiveMemory, MappingWithMultiInstanceHeapFlag) {
    TEST_DESCRIPTION("Test mapping memory that uses memory heap with VK_MEMORY_HEAP_MULTI_INSTANCE_BIT");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    uint32_t memory_index = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; ++i) {
        if ((memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            if (memory_info.memoryHeaps[memory_info.memoryTypes[i].heapIndex].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) {
                memory_index = i;
                break;
            }
        }
    }

    if (memory_index == std::numeric_limits<uint32_t>::max()) {
        GTEST_SKIP() << "Did not host visible memory from memory heap with VK_MEMORY_HEAP_MULTI_INSTANCE_BIT bit";
    }

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 64;
    mem_alloc.memoryTypeIndex = memory_index;

    VkDeviceMemory memory;
    vk::AllocateMemory(m_device->device(), &mem_alloc, nullptr, &memory);

    uint32_t *pData;
    vk::MapMemory(device(), memory, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    vk::UnmapMemory(device(), memory);
    vk::FreeMemory(m_device->device(), memory, nullptr);
}

TEST_F(PositiveMemory, BindImageMemoryMultiThreaded) {
    RETURN_IF_SKIP(Init())

    if (!IsPlatformMockICD()) {
        GTEST_SKIP() << "This test can crash drivers with threading issues";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    // Create an image object, allocate memory, bind memory, and destroy the object
    auto worker_thread = [&]() {
        for (uint32_t i = 0; i < 1000; ++i) {
            VkImage image;
            VkDeviceMemory mem;
            VkMemoryRequirements mem_reqs;

            VkResult err = vk::CreateImage(m_device->device(), &image_create_info, nullptr, &image);
            ASSERT_EQ(VK_SUCCESS, err);

            vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

            VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
            mem_alloc.memoryTypeIndex = 0;
            mem_alloc.allocationSize = mem_reqs.size;
            const bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
            ASSERT_TRUE(pass);

            err = vk::AllocateMemory(m_device->device(), &mem_alloc, nullptr, &mem);
            ASSERT_EQ(VK_SUCCESS, err);

            err = vk::BindImageMemory(m_device->device(), image, mem, 0);
            ASSERT_EQ(VK_SUCCESS, err);

            vk::DestroyImage(m_device->device(), image, nullptr);

            vk::FreeMemory(m_device->device(), mem, nullptr);
        }
    };

    constexpr int worker_count = 32;
    std::vector<std::thread> workers;
    workers.reserve(worker_count);
    for (int i = 0; i < worker_count; ++i) {
        workers.emplace_back(worker_thread);
    }
    for (auto &worker : workers) {
        worker.join();
    }
}

TEST_F(PositiveMemory, DeviceBufferMemoryRequirements) {
    TEST_DESCRIPTION("Test vkGetDeviceBufferMemoryRequirements");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(Init())

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device, buffer_create_info);
    ASSERT_TRUE(buffer.initialized());

    VkDeviceBufferMemoryRequirements info = vku::InitStructHelper();
    info.pCreateInfo = &buffer_create_info;
    VkMemoryRequirements2 memory_reqs2 = vku::InitStructHelper();
    vk::GetDeviceBufferMemoryRequirements(m_device->device(), &info, &memory_reqs2);

    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = memory_reqs2.memoryRequirements.size;

    const bool pass = m_device->phy().set_memory_type(memory_reqs2.memoryRequirements.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory buffer_memory(*m_device, memory_info);

    VkResult err = vk::BindBufferMemory(m_device->device(), buffer, buffer_memory, 0);
    ASSERT_EQ(VK_SUCCESS, err);
}

TEST_F(PositiveMemory, DeviceImageMemoryRequirements) {
    TEST_DESCRIPTION("Test vkGetDeviceImageMemoryRequirements");

    SetTargetApiVersion(VK_API_VERSION_1_3);

    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    vkt::Image image;
    image.init_no_mem(*m_device, image_create_info);
    ASSERT_TRUE(image.initialized());

    VkDeviceImageMemoryRequirements info = vku::InitStructHelper();
    info.pCreateInfo = &image_create_info;
    VkMemoryRequirements2 mem_reqs = vku::InitStructHelper();
    vk::GetDeviceImageMemoryRequirements(m_device->device(), &info, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = mem_reqs.memoryRequirements.size;
    const bool pass = m_device->phy().set_memory_type(mem_reqs.memoryRequirements.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    vkt::DeviceMemory mem(*m_device, mem_alloc);

    VkResult err = vk::BindImageMemory(m_device->device(), image, mem, 0);
    ASSERT_EQ(VK_SUCCESS, err);
}
