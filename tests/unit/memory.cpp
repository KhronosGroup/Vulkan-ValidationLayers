/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "generated/enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"

TEST_F(NegativeMemory, MapMemory) {
    TEST_DESCRIPTION("Attempt to map memory in a number of incorrect ways");
    bool pass;
    RETURN_IF_SKIP(Init())

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    const VkDeviceSize atom_size = m_device->phy().limits_.nonCoherentAtomSize;

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = 256;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer));

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.memoryTypeIndex = 0;

    // Ensure memory is big enough for both bindings
    // Want to make sure entire allocation is aligned to atom size
    static const VkDeviceSize allocation_size = atom_size * 64;
    alloc_info.allocationSize = allocation_size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem));

    uint8_t *pData;
    // Attempt to map memory size 0 is invalid
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-size-00680");
    vk::MapMemory(m_device->device(), mem, 0, 0, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Map memory twice
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-memory-00678");
    vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();

    // Unmap the memory to avoid re-map error
    vk::UnmapMemory(m_device->device(), mem);
    // overstep offset with VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-offset-00679");
    vk::MapMemory(m_device->device(), mem, allocation_size + 1, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep offset w/o VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-offset-00679");
    vk::MapMemory(m_device->device(), mem, allocation_size + 1, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep allocation w/o VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-size-00681");
    vk::MapMemory(m_device->device(), mem, 1, allocation_size, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Now error due to unmapping memory that's not mapped
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUnmapMemory-memory-00689");
    vk::UnmapMemory(m_device->device(), mem);
    m_errorMonitor->VerifyFound();

    // Now map memory and cause errors due to flushing invalid ranges
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 4 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData));
    VkMappedMemoryRange mmr = vku::InitStructHelper();
    mmr.memory = mem;
    mmr.offset = atom_size;  // Error b/c offset less than offset of mapped mem
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range that oversteps mapped range
    vk::UnmapMemory(m_device->device(), mem);
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
    mmr.offset = atom_size;
    mmr.size = 4 * atom_size;  // Flushing bounds exceed mapped bounds
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range with VK_WHOLE_SIZE that oversteps offset
    vk::UnmapMemory(m_device->device(), mem);
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 2 * atom_size, 4 * atom_size, 0, (void **)&pData));
    mmr.offset = atom_size;
    mmr.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00686");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Some platforms have an atomsize of 1 which makes the test meaningless
    if (atom_size > 3) {
        // Now with an offset NOT a multiple of the device limit
        vk::UnmapMemory(m_device->device(), mem);
        ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
        mmr.offset = 3;  // Not a multiple of atom_size
        mmr.size = VK_WHOLE_SIZE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-offset-00687");
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();

        // Now with a size NOT a multiple of the device limit
        vk::UnmapMemory(m_device->device(), mem);
        ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
        mmr.offset = atom_size;
        mmr.size = 2 * atom_size + 1;  // Not a multiple of atom_size
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-01390");
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();

        // Now with VK_WHOLE_SIZE and a mapping that does not end at a multiple of atom_size nor at the end of the memory.
        vk::UnmapMemory(m_device->device(), mem);
        ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size + 1, 0, (void **)&pData));
        mmr.offset = atom_size;
        mmr.size = VK_WHOLE_SIZE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-01389");
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();
    }

    // Try flushing and invalidating host memory not mapped
    vk::UnmapMemory(m_device->device(), mem);
    mmr.offset = 0;
    mmr.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-memory-00684");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-memory-00684");
    vk::InvalidateMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    vk::FreeMemory(m_device->device(), mem, NULL);

    // device memory not atom size aligned
    alloc_info.allocationSize = (atom_size * 4) + 1;
    ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer));
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem));
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData));
    // Some platforms have an atomsize of 1 which makes the test meaningless
    if (atom_size > 1) {
        // Offset is atom size, but total memory range is not atom size
        mmr.memory = mem;
        mmr.offset = atom_size;
        mmr.size = VK_WHOLE_SIZE;
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    }

    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vk::UnmapMemory(m_device->device(), mem);
    if (!pass) {
        vk::FreeMemory(m_device->device(), mem, NULL);
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    // TODO : If we can get HOST_VISIBLE w/o HOST_COHERENT we can test cases of
    //  kVUID_Core_MemTrack_InvalidMap in validateAndCopyNoncoherentMemoryToDriver()

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    vk::FreeMemory(m_device->device(), mem, NULL);
}

TEST_F(NegativeMemory, MapMemory2) {
    TEST_DESCRIPTION("Attempt to map memory in a number of incorrect ways");

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

    vkt::DeviceMemory memory(*m_device, memory_info);

    VkMemoryMapInfoKHR map_info = vku::InitStructHelper();
    map_info.memory = memory;

    VkMemoryUnmapInfoKHR unmap_info = vku::InitStructHelper();
    unmap_info.memory = memory;

    uint8_t *pData;
    // Attempt to map memory size 0 is invalid
    map_info.offset = 0;
    map_info.size = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryMapInfoKHR-size-07960");
    vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Map memory twice
    map_info.offset = 0;
    map_info.size = VK_WHOLE_SIZE;
    ASSERT_EQ(VK_SUCCESS, vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData));
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryMapInfoKHR-memory-07958");
    vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();

    // Unmap the memory to avoid re-map error
    vk::UnmapMemory2KHR(m_device->device(), &unmap_info);
    // overstep offset with VK_WHOLE_SIZE
    map_info.offset = allocation_size + 1;
    map_info.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryMapInfoKHR-offset-07959");
    vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep allocation w/o VK_WHOLE_SIZE
    map_info.offset = 1,
    map_info.size = allocation_size;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryMapInfoKHR-size-07961");
    vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Now error due to unmapping memory that's not mapped
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryUnmapInfoKHR-memory-07964");
    vk::UnmapMemory2KHR(m_device->device(), &unmap_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MapMemWithoutHostVisibleBit) {
    TEST_DESCRIPTION("Allocate memory that is not mappable and then attempt to map it.");

    RETURN_IF_SKIP(Init())

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 1024;

    if (!m_device->phy().set_memory_type(0xFFFFFFFF, &mem_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
        // If we can't find any unmappable memory this test doesn't make sense
        GTEST_SKIP() << "No unmappable memory types found";
    }

    vkt::DeviceMemory memory(*m_device, mem_alloc);
    void *mapped_address = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-memory-00682");
    m_errorMonitor->SetUnexpectedError("VUID-vkMapMemory-memory-00683");
    vk::MapMemory(m_device->device(), memory.handle(), 0, VK_WHOLE_SIZE, 0, &mapped_address);
    m_errorMonitor->VerifyFound();

    // Attempt to flush and invalidate non-host memory
    VkMappedMemoryRange memory_range = vku::InitStructHelper();
    memory_range.memory = memory.handle();
    memory_range.offset = 0;
    memory_range.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-memory-00684");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &memory_range);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-memory-00684");
    vk::InvalidateMappedMemoryRanges(m_device->device(), 1, &memory_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MapMemory2WithoutHostVisibleBit) {
    TEST_DESCRIPTION("Allocate memory that is not mappable and then attempt to map it.");
    AddRequiredExtensions(VK_KHR_MAP_MEMORY_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 1024;
    if (!m_device->phy().set_memory_type(
            0xFFFFFFFF, &mem_alloc, 0,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {  // If we can't find any unmappable memory this test doesn't make sense
        GTEST_SKIP() << "No unmappable memory types found";
    }

    vkt::DeviceMemory memory(*m_device, mem_alloc);

    VkMemoryMapInfoKHR map_info = vku::InitStructHelper();
    map_info.memory = memory.handle();
    map_info.offset = 0;
    map_info.size = 32;
    uint8_t *pData;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryMapInfoKHR-memory-07962");
    vk::MapMemory2KHR(m_device->device(), &map_info, (void **)&pData);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, RebindMemoryMultiObjectDebugUtils) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-07460");

    RETURN_IF_SKIP(Init())

    // Create an image, allocate memory, free it, and then try to bind it
    VkImage image;
    VkDeviceMemory mem1;
    VkDeviceMemory mem2;
    VkMemoryRequirements mem_reqs;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    // Introduce failure, do NOT set memProps to
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    mem_alloc.memoryTypeIndex = 1;
    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    // allocate 2 memory objects
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem1);
    ASSERT_EQ(VK_SUCCESS, err);
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem2);
    ASSERT_EQ(VK_SUCCESS, err);

    // Bind first memory object to Image object
    err = vk::BindImageMemory(m_device->device(), image, mem1, 0);
    ASSERT_EQ(VK_SUCCESS, err);

    // Introduce validation failure, try to bind a different memory object to
    // the same image object
    err = vk::BindImageMemory(m_device->device(), image, mem2, 0);
    m_errorMonitor->VerifyFound();

    // This particular VU should output three objects in its error message. Verify this works correctly.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VK_OBJECT_TYPE_IMAGE");
    err = vk::BindImageMemory(m_device->device(), image, mem2, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), image, NULL);
    vk::FreeMemory(m_device->device(), mem1, NULL);
    vk::FreeMemory(m_device->device(), mem2, NULL);
}

TEST_F(NegativeMemory, QueryMemoryCommitmentWithoutLazyProperty) {
    TEST_DESCRIPTION("Attempt to query memory commitment on memory without lazy allocation");
    RETURN_IF_SKIP(Init())

    auto image_ci = vkt::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_ci);

    const auto mem_reqs = image.memory_requirements();
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    image_alloc_info.allocationSize = mem_reqs.size;

    // the last argument is the "forbid" argument for set_memory_type, disallowing
    // that particular memory type rather than requiring it
    if (!m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &image_alloc_info, 0, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)) {
        GTEST_SKIP() << "Failed to set memory type";
    }
    vkt::DeviceMemory mem;
    mem.init(*m_device, image_alloc_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceMemoryCommitment-memory-00690");
    VkDeviceSize size;
    vk::GetDeviceMemoryCommitment(m_device->device(), mem.handle(), &size);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, BindImageMemoryType) {
    TEST_DESCRIPTION("Create an image, allocate memory, set a bad typeIndex and then try to bind it");
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
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;

    // Introduce Failure, select invalid TypeIndex
    VkPhysicalDeviceMemoryProperties memory_info;

    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);
    uint32_t i = 0;
    for (; i < memory_info.memoryTypeCount; i++) {
        // Would require deviceCoherentMemory feature
        if (memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
            continue;
        }
        // would require protected feature
        if (memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            continue;
        }
        if ((mem_reqs.memoryTypeBits & (1 << i)) == 0) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }
    if (i >= memory_info.memoryTypeCount) {
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-01047");
    vkt::DeviceMemory mem(*m_device, mem_alloc);
    vk::BindImageMemory(m_device->device(), image.handle(), mem.handle(), 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, BindMemory) {
    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    auto buffer_create_info = vkt::Buffer::create_info(4 * 1024 * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    // Create an image/buffer, allocate memory, free it, and then try to bind it
    {
        VkImageObj image(m_device);
        image.init_no_mem(*m_device, image_create_info);

        vkt::Buffer buffer;
        buffer.init_no_mem(*m_device, buffer_create_info);

        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);

        VkMemoryAllocateInfo image_mem_alloc = vku::InitStructHelper();
        VkMemoryAllocateInfo buffer_mem_alloc = vku::InitStructHelper();
        image_mem_alloc.allocationSize = image_mem_reqs.size;
        m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_mem_alloc, 0);
        buffer_mem_alloc.allocationSize = buffer_mem_reqs.size;
        m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_mem_alloc, 0);

        VkDeviceMemory image_mem = VK_NULL_HANDLE, buffer_mem = VK_NULL_HANDLE;
        vk::AllocateMemory(device(), &image_mem_alloc, nullptr, &image_mem);
        vk::AllocateMemory(device(), &buffer_mem_alloc, nullptr, &buffer_mem);

        vk::FreeMemory(device(), image_mem, nullptr);
        vk::FreeMemory(device(), buffer_mem, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-parameter");
        vk::BindImageMemory(device(), image.handle(), image_mem, 0);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-parameter");
        vk::BindBufferMemory(device(), buffer.handle(), buffer_mem, 0);
        m_errorMonitor->VerifyFound();
    }

    // Try to bind memory to an object that already has a memory binding
    {
        VkImageObj image(m_device);
        image.init_no_mem(*m_device, image_create_info);

        vkt::Buffer buffer;
        buffer.init_no_mem(*m_device, buffer_create_info);

        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
        VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
        m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);

        vkt::DeviceMemory image_mem(*m_device, image_alloc_info);
        vkt::DeviceMemory buffer_mem(*m_device, buffer_alloc_info);

        vk::BindImageMemory(device(), image.handle(), image_mem.handle(), 0);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-07460");
        vk::BindImageMemory(device(), image.handle(), image_mem.handle(), 0);
        m_errorMonitor->VerifyFound();

        vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), 0);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-buffer-07459");
        vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), 0);
        m_errorMonitor->VerifyFound();
    }

    // Try to bind memory to an object with an invalid memoryOffset
    {
        VkImageObj image(m_device);
        image.init_no_mem(*m_device, image_create_info);

        vkt::Buffer buffer;
        buffer.init_no_mem(*m_device, buffer_create_info);

        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
        VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        // Leave some extra space for alignment wiggle room
        image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size + buffer_mem_reqs.alignment;
        m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
        vkt::DeviceMemory image_mem(*m_device, image_alloc_info);
        vkt::DeviceMemory buffer_mem(*m_device, buffer_alloc_info);

        // Test unaligned memory offset
        {
            if (image_mem_reqs.alignment > 1) {
                VkDeviceSize image_offset = 1;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memoryOffset-01048");
                vk::BindImageMemory(device(), image.handle(), image_mem.handle(), image_offset);
                m_errorMonitor->VerifyFound();
            }

            if (buffer_mem_reqs.alignment > 1) {
                VkDeviceSize buffer_offset = 1;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01036");
                vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), buffer_offset);
                m_errorMonitor->VerifyFound();
            }
        }

        // Test memory offsets outside the memory allocation
        {
            VkDeviceSize image_offset =
                (image_alloc_info.allocationSize + image_mem_reqs.alignment) & ~(image_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memoryOffset-01046");
            vk::BindImageMemory(device(), image.handle(), image_mem.handle(), image_offset);
            m_errorMonitor->VerifyFound();

            VkDeviceSize buffer_offset =
                (buffer_alloc_info.allocationSize + buffer_mem_reqs.alignment) & ~(buffer_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01031");
            vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), buffer_offset);
            m_errorMonitor->VerifyFound();
        }

        // Test memory offsets within the memory allocation, but which leave too little memory for
        // the resource.
        {
            VkDeviceSize image_offset = (image_mem_reqs.size - 1) & ~(image_mem_reqs.alignment - 1);
            if ((image_offset > 0) && (image_mem_reqs.size < (image_alloc_info.allocationSize - image_mem_reqs.alignment))) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-size-01049");
                vk::BindImageMemory(device(), image.handle(), image_mem.handle(), image_offset);
                m_errorMonitor->VerifyFound();
            }

            VkDeviceSize buffer_offset = (buffer_mem_reqs.size - 1) & ~(buffer_mem_reqs.alignment - 1);
            if (buffer_offset > 0) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-size-01037");
                vk::BindBufferMemory(device(), buffer.handle(), buffer_mem.handle(), buffer_offset);
                m_errorMonitor->VerifyFound();
            }
        }
    }

    // Try to bind memory to an image created with sparse memory flags
    {
        VkImageCreateInfo sparse_image_create_info = image_create_info;
        sparse_image_create_info.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        VkImageFormatProperties image_format_properties = {};
        VkResult err = vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), sparse_image_create_info.format,
                                                                  sparse_image_create_info.imageType,
                                                                  sparse_image_create_info.tiling, sparse_image_create_info.usage,
                                                                  sparse_image_create_info.flags, &image_format_properties);
        if (!m_device->phy().features().sparseResidencyImage2D || err == VK_ERROR_FORMAT_NOT_SUPPORTED) {
            // most likely means sparse formats aren't supported here; skip this test.
        } else {
            if (image_format_properties.maxExtent.width == 0) {
                GTEST_SKIP() << "Sparse image format not supported";
            } else {
                VkImageObj sparse_image(m_device);
                sparse_image.init_no_mem(*m_device, sparse_image_create_info);
                VkMemoryRequirements sparse_mem_reqs = {};
                vk::GetImageMemoryRequirements(m_device->device(), sparse_image.handle(), &sparse_mem_reqs);
                if (sparse_mem_reqs.memoryTypeBits != 0) {
                    VkMemoryAllocateInfo sparse_mem_alloc = vku::InitStructHelper();
                    sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                    sparse_mem_alloc.memoryTypeIndex = 0;
                    m_device->phy().set_memory_type(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                    vkt::DeviceMemory memory(*m_device, sparse_mem_alloc);
                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-01045");
                    vk::BindImageMemory(m_device->device(), sparse_image.handle(), memory.handle(), 0);
                    m_errorMonitor->VerifyFound();
                }
            }
        }
    }

    // Try to bind memory to a buffer created with sparse memory flags
    {
        VkBufferCreateInfo sparse_buffer_create_info = buffer_create_info;
        sparse_buffer_create_info.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        if (!m_device->phy().features().sparseResidencyBuffer) {
            // most likely means sparse formats aren't supported here; skip this test.
        } else {
            vkt::Buffer sparse_buffer;
            sparse_buffer.init_no_mem(*m_device, sparse_buffer_create_info);
            VkMemoryRequirements sparse_mem_reqs = {};
            vk::GetBufferMemoryRequirements(m_device->device(), sparse_buffer.handle(), &sparse_mem_reqs);
            if (sparse_mem_reqs.memoryTypeBits != 0) {
                VkMemoryAllocateInfo sparse_mem_alloc = vku::InitStructHelper();
                sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                sparse_mem_alloc.memoryTypeIndex = 0;
                m_device->phy().set_memory_type(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                vkt::DeviceMemory memory(*m_device, sparse_mem_alloc);
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-buffer-01030");
                vk::BindBufferMemory(m_device->device(), sparse_buffer.handle(), memory.handle(), 0);
                m_errorMonitor->VerifyFound();
            }
        }
    }
}

TEST_F(NegativeMemory, BindMemoryUnsupported) {
    RETURN_IF_SKIP(Init())

    VkImageObj image(m_device);
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image.init_no_mem(*m_device, image_create_info);

    auto buffer_info = vkt::Buffer::create_info(4 * 1024 * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device, buffer_info);

    VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    image_alloc_info.allocationSize = image_mem_reqs.size;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    // Create a mask of available memory types *not* supported by these resources,
    // and try to use one of them.
    VkPhysicalDeviceMemoryProperties memory_properties = {};
    vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);

    uint32_t image_unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~image_mem_reqs.memoryTypeBits;
    // can't have protected bit because feature bit is not added
    bool found_type =
        m_device->phy().set_memory_type(image_unsupported_mem_type_bits, &image_alloc_info, 0,
                                        VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
    if (image_unsupported_mem_type_bits != 0 && found_type) {
        vkt::DeviceMemory memory(*m_device, image_alloc_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-01047");
        vk::BindImageMemory(device(), image.handle(), memory.handle(), 0);
        m_errorMonitor->VerifyFound();
    }

    uint32_t buffer_unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~buffer_mem_reqs.memoryTypeBits;
    found_type = m_device->phy().set_memory_type(buffer_unsupported_mem_type_bits, &buffer_alloc_info, 0,
                                                 VK_MEMORY_PROPERTY_PROTECTED_BIT | VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD);
    if (buffer_unsupported_mem_type_bits != 0 && found_type) {
        vkt::DeviceMemory memory(*m_device, buffer_alloc_info);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-01035");
        vk::BindBufferMemory(device(), buffer.handle(), memory.handle(), 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeMemory, BindMemoryNoCheck) {
    TEST_DESCRIPTION("Tests case were no call to memory requirements was made prior to binding");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())
    const bool mp_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    // first test buffer
    {
        VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
        buffer_create_info.flags = 0;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // Create 2 buffers, one that is checked and one that isn't by GetBufferMemoryRequirements
        VkBuffer buffer = VK_NULL_HANDLE;
        VkBuffer unchecked_buffer = VK_NULL_HANDLE;
        VkDeviceMemory buffer_mem = VK_NULL_HANDLE;
        VkDeviceMemory unchecked_buffer_mem = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(device(), &buffer_create_info, NULL, &buffer));
        ASSERT_EQ(VK_SUCCESS, vk::CreateBuffer(device(), &buffer_create_info, NULL, &unchecked_buffer));

        VkMemoryRequirements buffer_mem_reqs = {};
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
        // Leave some extra space for alignment wiggle room
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size + buffer_mem_reqs.alignment;
        ASSERT_TRUE(m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &unchecked_buffer_mem));

        if (buffer_mem_reqs.alignment > 1) {
            VkDeviceSize buffer_offset = 1;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01036");
            vk::BindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
            m_errorMonitor->VerifyFound();

            // Should trigger same VUID even when image was never checked
            // this makes an assumption that the driver will return the same image requirements for same createImageInfo where even
            // being close to running out of heap space
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01036");
            vk::BindBufferMemory(device(), unchecked_buffer, unchecked_buffer_mem, buffer_offset);
            m_errorMonitor->VerifyFound();
        }

        vk::DestroyBuffer(device(), buffer, NULL);
        vk::DestroyBuffer(device(), unchecked_buffer, NULL);
        vk::FreeMemory(device(), buffer_mem, NULL);
        vk::FreeMemory(device(), unchecked_buffer_mem, NULL);
    }

    // Next test is a single-plane image
    {
        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_create_info.extent.width = 256;
        image_create_info.extent.height = 256;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.flags = 0;

        // Create 2 images, one that is checked and one that isn't by GetImageMemoryRequirements
        VkImage image = VK_NULL_HANDLE;
        VkImage unchecked_image = VK_NULL_HANDLE;
        VkDeviceMemory image_mem = VK_NULL_HANDLE;
        VkDeviceMemory unchecked_image_mem = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &image));
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &unchecked_image));

        VkMemoryRequirements image_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
        // Leave some extra space for alignment wiggle room
        image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &image_alloc_info, NULL, &unchecked_image_mem));

        // single-plane image
        if (image_mem_reqs.alignment > 1) {
            VkDeviceSize image_offset = 1;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memoryOffset-01048");
            vk::BindImageMemory(device(), image, image_mem, image_offset);
            m_errorMonitor->VerifyFound();

            // Should trigger same VUID even when image was never checked
            // this makes an assumption that the driver will return the same image requirements for same createImageInfo where even
            // being close to running out of heap space
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memoryOffset-01048");
            vk::BindImageMemory(device(), unchecked_image, unchecked_image_mem, image_offset);
            m_errorMonitor->VerifyFound();
        }

        vk::DestroyImage(device(), image, NULL);
        vk::DestroyImage(device(), unchecked_image, NULL);
        vk::FreeMemory(device(), image_mem, NULL);
        vk::FreeMemory(device(), unchecked_image_mem, NULL);
    }

    // Same style test but with a multi-planar disjoint image
    // Test doesn't check either of the planes for the unchecked image
    if (mp_extensions == false) {
        GTEST_SKIP() << "Rest of test rely on YCbCr Multi-planar support";
    } else {
        // Check for support of format used by all multi-planar tests
        const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        VkFormatProperties mp_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &mp_format_properties);
        if (!((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
              (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            GTEST_SKIP() << "test rely on a supported disjoint format";
        }

        VkImageCreateInfo mp_image_create_info = vku::InitStructHelper();
        mp_image_create_info.imageType = VK_IMAGE_TYPE_2D;
        mp_image_create_info.format = mp_format;
        mp_image_create_info.extent.width = 256;
        mp_image_create_info.extent.height = 256;
        mp_image_create_info.extent.depth = 1;
        mp_image_create_info.mipLevels = 1;
        mp_image_create_info.arrayLayers = 1;
        mp_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        mp_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        mp_image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

        VkImage mp_image = VK_NULL_HANDLE;
        VkImage mp_unchecked_image = VK_NULL_HANDLE;
        // Array represent planes for disjoint images
        VkDeviceMemory mp_image_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        VkDeviceMemory mp_unchecked_image_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        VkMemoryRequirements2 mp_image_mem_reqs2[2];
        VkMemoryAllocateInfo mp_image_alloc_info[2];

        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_image));
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_unchecked_image));

        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = mp_image;
        mp_image_mem_reqs2[0] = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
        mp_image_mem_reqs2[1] = vku::InitStructHelper();
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mp_image_mem_reqs2[1]);

        mp_image_alloc_info[0] = vku::InitStructHelper();
        mp_image_alloc_info[1] = vku::InitStructHelper();

        mp_image_alloc_info[0].allocationSize = mp_image_mem_reqs2[0].memoryRequirements.size;
        ASSERT_TRUE(
            m_device->phy().set_memory_type(mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[0], 0));
        // Leave some extra space for alignment wiggle room
        mp_image_alloc_info[1].allocationSize =
            mp_image_mem_reqs2[1].memoryRequirements.size + mp_image_mem_reqs2[1].memoryRequirements.alignment;
        ASSERT_TRUE(
            m_device->phy().set_memory_type(mp_image_mem_reqs2[1].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[1], 0));

        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_image_mem[0]));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[1], NULL, &mp_image_mem[1]));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_unchecked_image_mem[0]));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &mp_image_alloc_info[1], NULL, &mp_unchecked_image_mem[1]));

        // Sets an invalid offset to plane 1
        if (mp_image_mem_reqs2[1].memoryRequirements.alignment > 1) {
            VkBindImagePlaneMemoryInfo plane_memory_info[2];
            plane_memory_info[0] = vku::InitStructHelper();
            plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
            plane_memory_info[1] = vku::InitStructHelper();
            plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0] = vku::InitStructHelper(&plane_memory_info[0]);
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1] = vku::InitStructHelper(&plane_memory_info[1]);
            bind_image_info[1].image = mp_image;
            bind_image_info[1].memory = mp_image_mem[1];
            bind_image_info[1].memoryOffset = 1;  // off alignment

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01620");
            vk::BindImageMemory2KHR(device(), 2, bind_image_info);
            m_errorMonitor->VerifyFound();

            // Should trigger same VUID even when image was never checked
            // this makes an assumption that the driver will return the same image requirements for same createImageInfo where even
            // being close to running out of heap space
            bind_image_info[0].image = mp_unchecked_image;
            bind_image_info[0].memory = mp_unchecked_image_mem[0];
            bind_image_info[1].image = mp_unchecked_image;
            bind_image_info[1].memory = mp_unchecked_image_mem[1];
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01620");
            vk::BindImageMemory2KHR(device(), 2, bind_image_info);
            m_errorMonitor->VerifyFound();
        }

        vk::DestroyImage(device(), mp_image, NULL);
        vk::DestroyImage(device(), mp_unchecked_image, NULL);
        vk::FreeMemory(device(), mp_image_mem[0], NULL);
        vk::FreeMemory(device(), mp_image_mem[1], NULL);
        vk::FreeMemory(device(), mp_unchecked_image_mem[0], NULL);
        vk::FreeMemory(device(), mp_unchecked_image_mem[1], NULL);
    }
}

TEST_F(NegativeMemory, BindMemory2BindInfos) {
    TEST_DESCRIPTION("These tests deal with VK_KHR_bind_memory_2 and invalid VkBindImageMemoryInfo* pBindInfos");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())
    const bool mp_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    RETURN_IF_SKIP(InitState())

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    {
        // Create 2 image with 2 memory objects
        VkImage image_a = VK_NULL_HANDLE;
        VkImage image_b = VK_NULL_HANDLE;
        VkDeviceMemory image_a_mem = VK_NULL_HANDLE;
        VkDeviceMemory image_b_mem = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &image_a));
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &image_b));

        VkMemoryRequirements image_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image_a, &image_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_a_mem));
        vk::GetImageMemoryRequirements(device(), image_b, &image_mem_reqs);
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_b_mem));

        // Try binding same image twice in array
        VkBindImageMemoryInfo bind_image_info[3];
        bind_image_info[0] = vku::InitStructHelper();
        bind_image_info[0].image = image_a;
        bind_image_info[0].memory = image_a_mem;
        bind_image_info[0].memoryOffset = 0;
        bind_image_info[1] = vku::InitStructHelper();
        bind_image_info[1].image = image_b;
        bind_image_info[1].memory = image_b_mem;
        bind_image_info[1].memoryOffset = 0;
        bind_image_info[2] = bind_image_info[0];  // duplicate bind

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-04006");
        vk::BindImageMemory2KHR(device(), 3, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Bind same image to 2 different memory in same array
        bind_image_info[1].image = image_a;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-04006");
        vk::BindImageMemory2KHR(device(), 2, bind_image_info);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), image_a_mem, NULL);
        vk::FreeMemory(device(), image_b_mem, NULL);
        vk::DestroyImage(device(), image_a, NULL);
        vk::DestroyImage(device(), image_b, NULL);
    }

    if (mp_extensions) {
        const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

        // Check for support of format used by all multi-planar tests
        VkFormatProperties mp_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &mp_format_properties);
        if (!((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
              (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            GTEST_SKIP() << "test rely on a supported disjoint format";
        }

        // Creat 1 normal, not disjoint image
        VkImage normal_image = VK_NULL_HANDLE;
        VkDeviceMemory normal_image_mem = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &image_create_info, NULL, &normal_image));
        VkMemoryRequirements image_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), normal_image, &image_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &image_alloc_info, NULL, &normal_image_mem));

        // Create 2 disjoint images with memory backing each plane
        VkImageCreateInfo mp_image_create_info = image_create_info;
        mp_image_create_info.format = mp_format;
        mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

        VkImage mp_image_a = VK_NULL_HANDLE;
        VkImage mp_image_b = VK_NULL_HANDLE;
        VkDeviceMemory mp_image_a_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        VkDeviceMemory mp_image_b_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_image_a));
        ASSERT_EQ(VK_SUCCESS, vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_image_b));

        AllocateDisjointMemory(m_device, vk::GetImageMemoryRequirements2KHR, mp_image_a, &mp_image_a_mem[0],
                               VK_IMAGE_ASPECT_PLANE_0_BIT);
        AllocateDisjointMemory(m_device, vk::GetImageMemoryRequirements2KHR, mp_image_a, &mp_image_a_mem[1],
                               VK_IMAGE_ASPECT_PLANE_1_BIT);
        AllocateDisjointMemory(m_device, vk::GetImageMemoryRequirements2KHR, mp_image_b, &mp_image_b_mem[0],
                               VK_IMAGE_ASPECT_PLANE_0_BIT);
        AllocateDisjointMemory(m_device, vk::GetImageMemoryRequirements2KHR, mp_image_b, &mp_image_b_mem[1],
                               VK_IMAGE_ASPECT_PLANE_1_BIT);

        VkBindImagePlaneMemoryInfo plane_memory_info[2];
        plane_memory_info[0] = vku::InitStructHelper();
        plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        plane_memory_info[1] = vku::InitStructHelper();
        plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

        // set all sType and memoryOffset as they are the same
        VkBindImageMemoryInfo bind_image_info[6];
        for (int i = 0; i < 6; i++) {
            bind_image_info[i] = vku::InitStructHelper();
            bind_image_info[i].memoryOffset = 0;
        }

        // Try only binding part of image_b
        bind_image_info[0].pNext = (void *)&plane_memory_info[0];
        bind_image_info[0].image = mp_image_a;
        bind_image_info[0].memory = mp_image_a_mem[0];
        bind_image_info[1].pNext = (void *)&plane_memory_info[1];
        bind_image_info[1].image = mp_image_a;
        bind_image_info[1].memory = mp_image_a_mem[1];
        bind_image_info[2].pNext = (void *)&plane_memory_info[0];
        bind_image_info[2].image = mp_image_b;
        bind_image_info[2].memory = mp_image_b_mem[0];
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-02858");
        vk::BindImageMemory2KHR(device(), 3, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Same thing, but mix in a non-disjoint image
        bind_image_info[3].pNext = nullptr;
        bind_image_info[3].image = normal_image;
        bind_image_info[3].memory = normal_image_mem;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-02858");
        vk::BindImageMemory2KHR(device(), 4, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Try binding image_b plane 1 twice
        // Valid case where binding disjoint and non-disjoint
        bind_image_info[4].pNext = (void *)&plane_memory_info[1];
        bind_image_info[4].image = mp_image_b;
        bind_image_info[4].memory = mp_image_b_mem[1];
        bind_image_info[5].pNext = (void *)&plane_memory_info[1];
        bind_image_info[5].image = mp_image_b;
        bind_image_info[5].memory = mp_image_b_mem[1];
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-04006");
        vk::BindImageMemory2KHR(device(), 6, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Try binding image_a with no plane specified
        bind_image_info[0].pNext = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-image-07736");
        vk::BindImageMemory2KHR(device(), 1, bind_image_info);
        m_errorMonitor->VerifyFound();
        bind_image_info[0].pNext = (void *)&plane_memory_info[0];

        // Valid case of binding 2 disjoint image and normal image by removing duplicate
        vk::BindImageMemory2KHR(device(), 5, bind_image_info);

        vk::FreeMemory(device(), normal_image_mem, NULL);
        vk::FreeMemory(device(), mp_image_a_mem[0], NULL);
        vk::FreeMemory(device(), mp_image_a_mem[1], NULL);
        vk::FreeMemory(device(), mp_image_b_mem[0], NULL);
        vk::FreeMemory(device(), mp_image_b_mem[1], NULL);
        vk::DestroyImage(device(), normal_image, NULL);
        vk::DestroyImage(device(), mp_image_a, NULL);
        vk::DestroyImage(device(), mp_image_b, NULL);
    }
}

TEST_F(NegativeMemory, BindMemoryToDestroyedObject) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-parameter");

    RETURN_IF_SKIP(Init())

    // Create an image object, allocate memory, destroy the object and then try
    // to bind it
    VkImage image;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    // Allocate memory
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_EQ(VK_SUCCESS, err);

    // Introduce validation failure, destroy Image object before binding
    vk::DestroyImage(m_device->device(), image, NULL);
    ASSERT_EQ(VK_SUCCESS, err);

    // Now Try to bind memory to this destroyed object
    err = vk::BindImageMemory(m_device->device(), image, mem, 0);
    // This may very well return an error.
    (void)err;

    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), mem, NULL);
}

TEST_F(NegativeMemory, AllocationCount) {
    VkResult err = VK_SUCCESS;
    const int max_mems = 32;
    VkDeviceMemory mems[max_mems + 1];

    RETURN_IF_SKIP(InitFramework())

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceLimitsEXT, fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    if (props.limits.maxMemoryAllocationCount > max_mems) {
        props.limits.maxMemoryAllocationCount = max_mems;
        fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    }
    RETURN_IF_SKIP(InitState())
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-maxMemoryAllocationCount-04101");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = 4;

    int i;
    for (i = 0; i <= max_mems; i++) {
        err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mems[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }
    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vk::FreeMemory(m_device->device(), mems[j], NULL);
    }
}

TEST_F(NegativeMemory, ImageMemoryNotBound) {
    TEST_DESCRIPTION("Attempt to draw with an image which has not had memory bound to it.");
    RETURN_IF_SKIP(Init())

    VkImage image;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = 0;
    VkResult err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_EQ(VK_SUCCESS, err);
    // Have to bind memory to image before recording cmd in cmd buffer using it
    VkMemoryRequirements mem_reqs;
    VkDeviceMemory image_mem;
    bool pass;
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &image_mem);
    ASSERT_EQ(VK_SUCCESS, err);

    // Introduce error, do not call vk::BindImageMemory(m_device->device(), image, image_mem, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         " used with no memory bound. Memory should be bound by calling vkBindImageMemory().");

    m_commandBuffer->begin();
    VkClearColorValue ccv;
    ccv.float32[0] = 1.0f;
    ccv.float32[1] = 1.0f;
    ccv.float32[2] = 1.0f;
    ccv.float32[3] = 1.0f;
    VkImageSubresourceRange isr = {};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.baseArrayLayer = 0;
    isr.baseMipLevel = 0;
    isr.layerCount = 1;
    isr.levelCount = 1;
    vk::CmdClearColorImage(m_commandBuffer->handle(), image, VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &isr);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
    vk::DestroyImage(m_device->device(), image, NULL);
    vk::FreeMemory(m_device->device(), image_mem, nullptr);
}

TEST_F(NegativeMemory, BufferMemoryNotBound) {
    TEST_DESCRIPTION("Attempt to copy from a buffer which has not had memory bound to it.");
    RETURN_IF_SKIP(Init())

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buf_info.size = 1024;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = 1024;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_EQ(VK_SUCCESS, err);

    // Introduce failure by not calling vkBindBufferMemory(m_device->device(), buffer, mem, 0);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         " used with no memory bound. Memory should be bound by calling vkBindBufferMemory().");
    VkBufferImageCopy region = {};
    region.bufferRowLength = 16;
    region.bufferImageHeight = 16;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    region.imageSubresource.layerCount = 1;
    region.imageExtent.height = 4;
    region.imageExtent.width = 4;
    region.imageExtent.depth = 1;
    m_commandBuffer->begin();
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer, image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    vk::FreeMemory(m_device->handle(), mem, NULL);
}

TEST_F(NegativeMemory, DedicatedAllocationBinding) {
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize resource_size = 1024;
    auto buffer_info = vkt::Buffer::create_info(resource_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vkt::Buffer buffer;
    buffer.init_no_mem(*m_device, buffer_info);
    auto buffer_alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(), mem_flags);
    VkMemoryDedicatedAllocateInfoKHR buffer_dedicated_info = vku::InitStructHelper();
    buffer_dedicated_info.buffer = buffer.handle();
    buffer_alloc_info.pNext = &buffer_dedicated_info;
    vkt::DeviceMemory dedicated_buffer_memory;
    dedicated_buffer_memory.init(*m_device, buffer_alloc_info);

    vkt::Buffer wrong_buffer;
    wrong_buffer.init_no_mem(*m_device, buffer_info);

    // Bind with wrong buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-01508");
    vk::BindBufferMemory(m_device->handle(), wrong_buffer.handle(), dedicated_buffer_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset (same VUID)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindBufferMemory-memory-01508");  // offset must be zero
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindBufferMemory-size-01037");  // offset pushes us past size
    auto offset = buffer.memory_requirements().alignment;
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), dedicated_buffer_memory.handle(), offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), dedicated_buffer_memory.handle(), 0);

    // And for images...
    VkImageObj image(m_device);
    VkImageObj wrong_image(m_device);
    auto image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image.init_no_mem(*m_device, image_info);
    wrong_image.init_no_mem(*m_device, image_info);

    VkMemoryDedicatedAllocateInfoKHR image_dedicated_info = vku::InitStructHelper();
    image_dedicated_info.image = image.handle();
    auto image_alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vkt::DeviceMemory dedicated_image_memory;
    dedicated_image_memory.init(*m_device, image_alloc_info);

    // Bind with wrong image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02628");
    vk::BindImageMemory(m_device->handle(), wrong_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset (same VUID)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-memory-02628");  // offset must be zero
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-size-01049");  // offset pushes us past size
    auto image_offset = image.memory_requirements().alignment;
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), image_offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);
}

TEST_F(NegativeMemory, DedicatedAllocationImageAliasing) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV aliasing_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(aliasing_features);
    if (aliasing_features.dedicatedAllocationImageAliasing != VK_TRUE) {
        GTEST_SKIP() << "dedicatedAllocationImageAliasing feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &aliasing_features));

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize resource_size = 1024;

    std::unique_ptr<VkImageObj> image(new VkImageObj(m_device));  // in a pointer so it can be easily destroyed.
    VkImageObj identical_image(m_device);
    VkImageObj post_delete_image(m_device);

    auto image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image->init_no_mem(*m_device, image_info);
    identical_image.init_no_mem(*m_device, image_info);
    post_delete_image.init_no_mem(*m_device, image_info);

    VkMemoryDedicatedAllocateInfoKHR image_dedicated_info = vku::InitStructHelper();
    image_dedicated_info.image = image->handle();
    auto image_alloc_info = vkt::DeviceMemory::get_resource_alloc_info(*m_device, image->memory_requirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vkt::DeviceMemory dedicated_image_memory;
    dedicated_image_memory.init(*m_device, image_alloc_info);

    // Bind with different but identical image
    vk::BindImageMemory(m_device->handle(), identical_image.handle(), dedicated_image_memory.handle(), 0);

    VkImageObj smaller_image(m_device);
    image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size - 1;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    smaller_image.init_no_mem(*m_device, image_info);

    // Bind with a smaller image
    vk::BindImageMemory(m_device->handle(), smaller_image.handle(), dedicated_image_memory.handle(), 0);

    VkImageObj larger_image(m_device);
    image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size + 1;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    larger_image.init_no_mem(*m_device, image_info);

    // Bind with a larger image (not supported, and not enough memory)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02629");
    if (larger_image.memory_requirements().size > image->memory_requirements().size) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-size-01049");
    }
    vk::BindImageMemory(m_device->handle(), larger_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-memory-02629");  // offset must be zero
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-size-01049");  // offset pushes us past size
    auto image_offset = image->memory_requirements().alignment;
    vk::BindImageMemory(m_device->handle(), image->handle(), dedicated_image_memory.handle(), image_offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    vk::BindImageMemory(m_device->handle(), image->handle(), dedicated_image_memory.handle(), 0);

    image.reset();  // destroy the original image
    vk::BindImageMemory(m_device->handle(), post_delete_image.handle(), dedicated_image_memory.handle(), 0);
}

TEST_F(NegativeMemory, BufferDeviceAddressEXT) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceBufferAddressFeaturesEXT buffer_device_address_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(buffer_device_address_features);
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &buffer_device_address_features));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferDeviceAddressCreateInfoEXT addr_ci = vku::InitStructHelper();
    addr_ci.deviceAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-deviceAddress-02604");

    buffer_create_info.pNext = nullptr;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkBufferDeviceAddressInfoEXT info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressEXT(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(m_device->device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_EQ(VK_SUCCESS, err);

    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(NegativeMemory, BufferDeviceAddressEXTDisabled) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceBufferAddressFeaturesEXT buffer_device_address_features = vku::InitStructHelper();
    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &buffer_device_address_features));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkBufferDeviceAddressInfoEXT info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02601");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressEXT(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(NegativeMemory, BufferDeviceAddressKHR) {
    TEST_DESCRIPTION("Test VK_KHR_buffer_device_address.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_device_address_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(buffer_device_address_features);
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &buffer_device_address_features));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferOpaqueCaptureAddressCreateInfoKHR addr_ci = vku::InitStructHelper();
    addr_ci.opaqueCaptureAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337");

    buffer_create_info.pNext = nullptr;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkBufferDeviceAddressInfoKHR info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_EQ(VK_SUCCESS, err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-bufferDeviceAddress-03339");
    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);
    m_errorMonitor->VerifyFound();

    VkDeviceMemoryOpaqueCaptureAddressInfoKHR mem_opaque_addr_info = vku::InitStructHelper();
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);

    mem_opaque_addr_info.memory = buffer_mem;
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);

    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);

    vk::GetBufferDeviceAddressKHR(m_device->device(), &info);

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(NegativeMemory, BufferDeviceAddressKHRDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_buffer_device_address.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR buffer_device_address_features = vku::InitStructHelper();
    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &buffer_device_address_features));
    InitRenderTarget();

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_EQ(VK_SUCCESS, err);

    VkBufferDeviceAddressInfoKHR info = vku::InitStructHelper();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02601");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vk::GetBufferDeviceAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferOpaqueCaptureAddress-None-03326");
    vk::GetBufferOpaqueCaptureAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_EQ(VK_SUCCESS, err);

    VkDeviceMemoryOpaqueCaptureAddressInfoKHR mem_opaque_addr_info = vku::InitStructHelper();
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceMemoryOpaqueCaptureAddress-None-03334");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vk::GetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = vku::InitStructHelper();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-flags-03330");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-flags-03331");
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(NegativeMemory, MemoryType) {
    // Attempts to allocate from a memory type that doesn't exist

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-pAllocateInfo-01714");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = memory_info.memoryTypeCount;
    mem_alloc.allocationSize = 4;

    VkDeviceMemory mem;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, AllocationBeyondHeapSize) {
    // Attempts to allocate a single piece of memory that's larger than the heap size

    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-pAllocateInfo-01713");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = memory_info.memoryHeaps[memory_info.memoryTypes[0].heapIndex].size + 1;

    VkDeviceMemory mem;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, DeviceCoherentMemoryDisabledAMD) {
    // Attempts to allocate device coherent memory without enabling the extension/feature
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD, does not support the necessary memory type";
    }

    VkPhysicalDeviceCoherentMemoryFeaturesAMD coherent_memory_features_amd = vku::InitStructHelper();
    coherent_memory_features_amd.deviceCoherentMemory = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &coherent_memory_features_amd));

    // Find a memory type that includes the device coherent memory property
    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);
    uint32_t deviceCoherentMemoryTypeIndex = memory_info.memoryTypeCount;  // Set to an invalid value just in case

    for (uint32_t i = 0; i < memory_info.memoryTypeCount; ++i) {
        if ((memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) != 0) {
            deviceCoherentMemoryTypeIndex = i;
            break;
        }
    }

    if (deviceCoherentMemoryTypeIndex == memory_info.memoryTypeCount) {
        GTEST_SKIP() << "Valid memory type index not found";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-deviceCoherentMemory-02790");

    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = deviceCoherentMemoryTypeIndex;
    mem_alloc.allocationSize = 4;

    VkDeviceMemory mem;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, DedicatedAllocation) {
    TEST_DESCRIPTION("Create invalid requests to dedicated allocation of memory");

    // Both VK_KHR_dedicated_allocation and VK_KHR_sampler_ycbcr_conversion supported in 1.1
    // Quicke to set 1.1 then check all extensions in 1.0
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init())

    const VkFormat disjoint_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    const VkFormat normal_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), disjoint_format, &format_properties);

    bool sparse_support = (m_device->phy().features().sparseBinding == VK_TRUE);
    bool disjoint_support = ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) != 0);

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.size = 2048;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = NULL;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = normal_format;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;

    // Create Images and Buffers without any memory backing
    VkImage normal_image = VK_NULL_HANDLE;
    vk::CreateImage(device(), &image_create_info, nullptr, &normal_image);

    VkBuffer normal_buffer = VK_NULL_HANDLE;
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &normal_buffer);

    VkImage sparse_image = VK_NULL_HANDLE;
    VkBuffer sparse_buffer = VK_NULL_HANDLE;
    if (sparse_support == true) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        vk::CreateImage(device(), &image_create_info, nullptr, &sparse_image);
        buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
        vk::CreateBuffer(device(), &buffer_create_info, nullptr, &sparse_buffer);
    }

    VkImage disjoint_image = VK_NULL_HANDLE;
    if (disjoint_support == true) {
        image_create_info.format = disjoint_format;
        image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
        vk::CreateImage(device(), &image_create_info, nullptr, &disjoint_image);
    }

    VkDeviceMemory device_memory;
    VkMemoryDedicatedAllocateInfo dedicated_allocate_info = vku::InitStructHelper();
    VkMemoryAllocateInfo memory_allocate_info = vku::InitStructHelper(&dedicated_allocate_info);
    memory_allocate_info.memoryTypeIndex = 0;
    memory_allocate_info.allocationSize = 64;

    // Both image and buffer set in dedicated allocation
    dedicated_allocate_info.image = normal_image;
    dedicated_allocate_info.buffer = normal_buffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryDedicatedAllocateInfo-image-01432");
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &device_memory);
    m_errorMonitor->VerifyFound();

    if (sparse_support == true) {
        VkMemoryRequirements sparse_image_memory_req;
        vk::GetImageMemoryRequirements(device(), sparse_image, &sparse_image_memory_req);
        VkMemoryRequirements sparse_buffer_memory_req;
        vk::GetBufferMemoryRequirements(device(), sparse_buffer, &sparse_buffer_memory_req);

        dedicated_allocate_info.image = sparse_image;
        dedicated_allocate_info.buffer = VK_NULL_HANDLE;
        memory_allocate_info.allocationSize = sparse_image_memory_req.size;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryDedicatedAllocateInfo-image-01434");
        vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &device_memory);
        m_errorMonitor->VerifyFound();

        dedicated_allocate_info.image = VK_NULL_HANDLE;
        dedicated_allocate_info.buffer = sparse_buffer;
        memory_allocate_info.allocationSize = sparse_buffer_memory_req.size;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryDedicatedAllocateInfo-buffer-01436");
        vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &device_memory);
        m_errorMonitor->VerifyFound();
    }

    if (disjoint_support == true) {
        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper(&image_plane_req);
        mem_req_info2.image = disjoint_image;
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();

        vk::GetImageMemoryRequirements2(m_device->device(), &mem_req_info2, &mem_req2);

        dedicated_allocate_info.image = disjoint_image;
        dedicated_allocate_info.buffer = VK_NULL_HANDLE;
        memory_allocate_info.allocationSize = mem_req2.memoryRequirements.size;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryDedicatedAllocateInfo-image-01797");
        vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &device_memory);
        m_errorMonitor->VerifyFound();
    }

    VkMemoryRequirements normal_image_memory_req;
    vk::GetImageMemoryRequirements(device(), normal_image, &normal_image_memory_req);
    VkMemoryRequirements normal_buffer_memory_req;
    vk::GetBufferMemoryRequirements(device(), normal_buffer, &normal_buffer_memory_req);

    // Set allocation size to be not equal to memory requirement
    memory_allocate_info.allocationSize = normal_image_memory_req.size - 1;
    dedicated_allocate_info.image = normal_image;
    dedicated_allocate_info.buffer = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryDedicatedAllocateInfo-image-02964");
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &device_memory);
    m_errorMonitor->VerifyFound();

    memory_allocate_info.allocationSize = normal_buffer_memory_req.size - 1;
    dedicated_allocate_info.image = VK_NULL_HANDLE;
    dedicated_allocate_info.buffer = normal_buffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryDedicatedAllocateInfo-buffer-02965");
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &device_memory);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(device(), normal_image, nullptr);
    vk::DestroyBuffer(device(), normal_buffer, nullptr);
    if (sparse_support == true) {
        vk::DestroyImage(device(), sparse_image, nullptr);
        vk::DestroyBuffer(device(), sparse_buffer, nullptr);
    }
    if (disjoint_support == true) {
        vk::DestroyImage(device(), disjoint_image, nullptr);
    }
}

TEST_F(NegativeMemory, MemoryRequirements) {
    TEST_DESCRIPTION("Create invalid requests to image and buffer memory requirments.");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    RETURN_IF_SKIP(InitState())

    // Need to make sure disjoint is supported for format
    // Also need to support an arbitrary image usage feature
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &format_properties);
    if (!((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
          (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
        GTEST_SKIP() << "test requires disjoint/sampled feature bit on format";
    } else {
        VkImageCreateInfo image_create_info = vku::InitStructHelper();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        image_create_info.extent.width = 64;
        image_create_info.extent.height = 64;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

        VkImage image;
        VkResult err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
        ASSERT_EQ(VK_SUCCESS, err);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-01588");
        VkMemoryRequirements memory_requirements;
        vk::GetImageMemoryRequirements(m_device->device(), image, &memory_requirements);
        m_errorMonitor->VerifyFound();

        VkImageMemoryRequirementsInfo2 mem_req_info2 = vku::InitStructHelper();
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = vku::InitStructHelper();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01589");
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        // Point to a 3rd plane for a 2-plane format
        VkImagePlaneMemoryRequirementsInfo image_plane_req = vku::InitStructHelper();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281");
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        // Test with a non planar image aspect also
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281");
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        vk::DestroyImage(m_device->device(), image, nullptr);

        // Recreate image without Disjoint bit
        image_create_info.flags = 0;
        err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
        ASSERT_EQ(VK_SUCCESS, err);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01590");
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        vk::DestroyImage(m_device->device(), image, nullptr);

        // Recreate image with single plane format and with Disjoint bit
        image_create_info.flags = 0;
        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
        ASSERT_EQ(VK_SUCCESS, err);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        // Disjoint bit isn't set as likely not even supported by non-planar format
        m_errorMonitor->SetUnexpectedError("VUID-VkImageMemoryRequirementsInfo2-image-01590");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-02280");
        vk::GetImageMemoryRequirements2KHR(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        vk::DestroyImage(m_device->device(), image, nullptr);
    }
}

TEST_F(NegativeMemory, MemoryAllocatepNextChain) {
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())
    RETURN_IF_SKIP(InitState())

    VkDeviceMemory mem;
    VkMemoryAllocateInfo mem_alloc = vku::InitStructHelper();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = 4;

    // pNext chain includes both VkExportMemoryAllocateInfo and VkExportMemoryAllocateInfoNV
    {
        VkExportMemoryAllocateInfoNV export_memory_info_nv = vku::InitStructHelper();
        export_memory_info_nv.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_NV;

        VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&export_memory_info_nv);
        export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-00640");
        mem_alloc.pNext = &export_memory_info;
        vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
        m_errorMonitor->VerifyFound();
    }
#ifdef VK_USE_PLATFORM_WIN32_KHR
    // pNext chain includes both VkExportMemoryAllocateInfo and VkExportMemoryWin32HandleInfoNV
    {
        VkExportMemoryWin32HandleInfoNV export_memory_info_win32_nv = vku::InitStructHelper();
        export_memory_info_win32_nv.pAttributes = nullptr;
        export_memory_info_win32_nv.dwAccess = 0;

        VkExportMemoryAllocateInfo export_memory_info = vku::InitStructHelper(&export_memory_info_win32_nv);
        export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-00640");
        mem_alloc.pNext = &export_memory_info;
        vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
        m_errorMonitor->VerifyFound();
    }
    // pNext chain includes both VkImportMemoryWin32HandleInfoKHR and VkImportMemoryWin32HandleInfoNV
    {
        VkImportMemoryWin32HandleInfoKHR import_memory_info_win32_khr = vku::InitStructHelper();
        import_memory_info_win32_khr.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

        VkImportMemoryWin32HandleInfoNV import_memory_info_win32_nv = vku::InitStructHelper(&import_memory_info_win32_khr);
        import_memory_info_win32_nv.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_NV;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-00641");
        mem_alloc.pNext = &import_memory_info_win32_nv;
        vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
        m_errorMonitor->VerifyFound();
    }
#endif  // VK_USE_PLATFORM_WIN32_KHR
}

TEST_F(NegativeMemory, DeviceImageMemoryRequirementsSwapchain) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_swapchain_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    VkDeviceImageMemoryRequirementsKHR device_image_memory_requirements = vku::InitStructHelper();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;

    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06416");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, DeviceImageMemoryRequirementsDisjoint) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    const VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) == 0) {
        GTEST_SKIP() << "Test requires disjoint support extensions";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    VkDeviceImageMemoryRequirementsKHR device_image_memory_requirements = vku::InitStructHelper();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_NONE_KHR;

    VkMemoryRequirements2 memory_requirements = vku::InitStructHelper();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06417");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();

    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06419");
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();

    // valid
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
    vk::GetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
}

TEST_F(NegativeMemory, BindBufferMemoryDeviceGroup) {
    TEST_DESCRIPTION("Test VkBindBufferMemoryDeviceGroupInfo.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitFramework())

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = 0;
    create_device_pnext.pPhysicalDevices = nullptr;
    for (const auto &dg : physical_device_group) {
        if (dg.physicalDeviceCount > 1) {
            create_device_pnext.physicalDeviceCount = dg.physicalDeviceCount;
            create_device_pnext.pPhysicalDevices = dg.physicalDevices;
            break;
        }
    }
    if (create_device_pnext.pPhysicalDevices) {
        RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    } else {
        GTEST_SKIP() << "Test requires a physical device group with more than 1 device";
    }

    vkt::Buffer buffer;
    auto buffer_info = vkt::Buffer::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer.init_no_mem(*m_device, buffer_info);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = vku::InitStructHelper();
    buffer_alloc_info.memoryTypeIndex = 0;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;

    VkDeviceMemory buffer_memory;
    vk::AllocateMemory(m_device->device(), &buffer_alloc_info, nullptr, &buffer_memory);

    std::vector<uint32_t> device_indices(create_device_pnext.physicalDeviceCount);

    VkBindBufferMemoryDeviceGroupInfo bind_buffer_memory_device_group = vku::InitStructHelper();
    bind_buffer_memory_device_group.deviceIndexCount = 1;
    bind_buffer_memory_device_group.pDeviceIndices = device_indices.data();

    VkBindBufferMemoryInfo bind_buffer_info = vku::InitStructHelper(&bind_buffer_memory_device_group);
    bind_buffer_info.buffer = buffer.handle();
    bind_buffer_info.memory = buffer_memory;
    bind_buffer_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryDeviceGroupInfo-deviceIndexCount-01606");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();

    bind_buffer_memory_device_group.deviceIndexCount = create_device_pnext.physicalDeviceCount;
    device_indices[0] = create_device_pnext.physicalDeviceCount;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryDeviceGroupInfo-pDeviceIndices-01607");
    vk::BindBufferMemory2(device(), 1, &bind_buffer_info);
    m_errorMonitor->VerifyFound();
    device_indices[0] = 0;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryRequirements image_mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = vku::InitStructHelper();
    image_alloc_info.memoryTypeIndex = 0;
    image_alloc_info.allocationSize = image_mem_reqs.size;

    VkDeviceMemory image_memory;
    vk::AllocateMemory(m_device->device(), &image_alloc_info, nullptr, &image_memory);

    VkBindImageMemoryDeviceGroupInfo bind_image_memory_device_group = vku::InitStructHelper();
    bind_image_memory_device_group.deviceIndexCount = 1;
    bind_image_memory_device_group.pDeviceIndices = device_indices.data();

    VkBindImageMemoryInfo bind_image_info = vku::InitStructHelper(&bind_image_memory_device_group);
    bind_image_info.image = image.handle();
    bind_image_info.memory = image_memory;
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01634");
    vk::BindImageMemory2(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeMemory, MemoryPriorityOutOfRange) {
    TEST_DESCRIPTION("Allocate memory with invalid priority.");

    AddRequiredExtensions(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    RETURN_IF_SKIP(Init())

    VkMemoryPriorityAllocateInfoEXT priority = vku::InitStructHelper();
    priority.priority = 2.0f;

    VkMemoryAllocateInfo memory_ai = vku::InitStructHelper(&priority);
    memory_ai.allocationSize = 0x100000;
    memory_ai.memoryTypeIndex = 0;

    VkDeviceMemory memory;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryPriorityAllocateInfoEXT-priority-02602");
    vk::AllocateMemory(*m_device, &memory_ai, nullptr, &memory);
    m_errorMonitor->VerifyFound();
}
