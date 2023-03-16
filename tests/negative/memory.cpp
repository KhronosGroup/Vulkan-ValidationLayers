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

#include "cast_utils.h"
#include "enum_flag_bits.h"
#include "../framework/layer_validation_tests.h"
#include "vk_layer_utils.h"

TEST_F(VkLayerTest, InvalidMemoryMapping) {
    TEST_DESCRIPTION("Attempt to map memory in a number of incorrect ways");
    bool pass;
    ASSERT_NO_FATAL_FAILURE(Init());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    const VkDeviceSize atom_size = m_device->props.limits.nonCoherentAtomSize;

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = 256;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    ASSERT_VK_SUCCESS(vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer));

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
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
    ASSERT_VK_SUCCESS(vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem));

    uint8_t *pData;
    // Attempt to map memory size 0 is invalid
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-size-00680");
    vk::MapMemory(m_device->device(), mem, 0, 0, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Map memory twice
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData));
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
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 4 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData));
    VkMappedMemoryRange mmr = LvlInitStruct<VkMappedMemoryRange>();
    mmr.memory = mem;
    mmr.offset = atom_size;  // Error b/c offset less than offset of mapped mem
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range that oversteps mapped range
    vk::UnmapMemory(m_device->device(), mem);
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
    mmr.offset = atom_size;
    mmr.size = 4 * atom_size;  // Flushing bounds exceed mapped bounds
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range with VK_WHOLE_SIZE that oversteps offset
    vk::UnmapMemory(m_device->device(), mem);
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 2 * atom_size, 4 * atom_size, 0, (void **)&pData));
    mmr.offset = atom_size;
    mmr.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00686");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Some platforms have an atomsize of 1 which makes the test meaningless
    if (atom_size > 3) {
        // Now with an offset NOT a multiple of the device limit
        vk::UnmapMemory(m_device->device(), mem);
        ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
        mmr.offset = 3;  // Not a multiple of atom_size
        mmr.size = VK_WHOLE_SIZE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-offset-00687");
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();

        // Now with a size NOT a multiple of the device limit
        vk::UnmapMemory(m_device->device(), mem);
        ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData));
        mmr.offset = atom_size;
        mmr.size = 2 * atom_size + 1;  // Not a multiple of atom_size
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-01390");
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();

        // Now with VK_WHOLE_SIZE and a mapping that does not end at a multiple of atom_size nor at the end of the memory.
        vk::UnmapMemory(m_device->device(), mem);
        ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size + 1, 0, (void **)&pData));
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
    ASSERT_VK_SUCCESS(vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer));
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    ASSERT_VK_SUCCESS(vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem));
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData));
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

TEST_F(VkLayerTest, MapMemWithoutHostVisibleBit) {
    TEST_DESCRIPTION("Allocate memory that is not mappable and then attempt to map it.");
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-memory-00682");
    m_errorMonitor->SetUnexpectedError("VUID-vkMapMemory-memory-00683");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.allocationSize = 1024;

    pass = m_device->phy().set_memory_type(0xFFFFFFFF, &mem_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {  // If we can't find any unmappable memory this test doesn't
                  // make sense
        GTEST_SKIP() << "No unmappable memory types found";
    }

    VkDeviceMemory mem;
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    void *mappedAddress = NULL;
    err = vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, &mappedAddress);
    m_errorMonitor->VerifyFound();

    // Attempt to flush and invalidate non-host memory
    VkMappedMemoryRange memory_range = LvlInitStruct<VkMappedMemoryRange>();
    memory_range.memory = mem;
    memory_range.offset = 0;
    memory_range.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-memory-00684");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &memory_range);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-memory-00684");
    vk::InvalidateMappedMemoryRanges(m_device->device(), 1, &memory_range);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), mem, NULL);
}

TEST_F(VkLayerTest, RebindMemoryMultiObjectDebugUtils) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-07460");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create an image, allocate memory, free it, and then try to bind it
    VkImage image;
    VkDeviceMemory mem1;
    VkDeviceMemory mem2;
    VkMemoryRequirements mem_reqs;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    // Introduce failure, do NOT set memProps to
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    mem_alloc.memoryTypeIndex = 1;
    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    // allocate 2 memory objects
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem1);
    ASSERT_VK_SUCCESS(err);
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem2);
    ASSERT_VK_SUCCESS(err);

    // Bind first memory object to Image object
    err = vk::BindImageMemory(m_device->device(), image, mem1, 0);
    ASSERT_VK_SUCCESS(err);

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

TEST_F(VkLayerTest, QueryMemoryCommitmentWithoutLazyProperty) {
    TEST_DESCRIPTION("Attempt to query memory commitment on memory without lazy allocation");
    ASSERT_NO_FATAL_FAILURE(Init());

    auto image_ci = vk_testing::Image::create_info();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_ci);

    const auto mem_reqs = image.memory_requirements();
    auto image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    image_alloc_info.allocationSize = mem_reqs.size;

    // the last argument is the "forbid" argument for set_memory_type, disallowing
    // that particular memory type rather than requiring it
    if (!m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &image_alloc_info, 0, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)) {
        GTEST_SKIP() << "Failed to set memory type";
    }
    vk_testing::DeviceMemory mem;
    mem.init(*m_device, image_alloc_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceMemoryCommitment-memory-00690");
    VkDeviceSize size;
    vk::GetDeviceMemoryCommitment(m_device->device(), mem.handle(), &size);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BindImageInvalidMemoryType) {
    VkResult err;

    TEST_DESCRIPTION("Test validation check for an invalid memory type index during bind[Buffer|Image]Memory time");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create an image, allocate memory, set a bad typeIndex and then try to
    // bind it
    VkImage image;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    err = vk::CreateImage(m_device->device(), &image_create_info, nullptr, &image);
    ASSERT_VK_SUCCESS(err);

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
        if ((mem_reqs.memoryTypeBits & (1 << i)) == 0) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }
    if (i >= memory_info.memoryTypeCount) {
        vk::DestroyImage(m_device->device(), image, nullptr);
        GTEST_SKIP() << "No invalid memory type index could be found";
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "for this object type are not compatible with the memory");

    err = vk::AllocateMemory(m_device->device(), &mem_alloc, nullptr, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindImageMemory(m_device->device(), image, mem, 0);
    (void)err;

    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), image, nullptr);
    vk::FreeMemory(m_device->device(), mem, nullptr);
}

TEST_F(VkLayerTest, BindInvalidMemory) {
    VkResult err;
    bool pass;

    AddOptionalExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (IsExtensionsEnabled(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) &&
        IsExtensionsEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        auto coherent_mem_features = LvlInitStruct<VkPhysicalDeviceCoherentMemoryFeaturesAMD>();
        GetPhysicalDeviceFeatures2(coherent_mem_features);
        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &coherent_mem_features));
    } else {
        ASSERT_NO_FATAL_FAILURE(InitState());
    }

    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    const int32_t tex_width = 256;
    const int32_t tex_height = 256;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.flags = 0;
    buffer_create_info.size = 4 * 1024 * 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Create an image/buffer, allocate memory, free it, and then try to bind it
    {
        VkImage image = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vk::CreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        err = vk::CreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);

        VkMemoryAllocateInfo image_mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
        VkMemoryAllocateInfo buffer_mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
        image_mem_alloc.allocationSize = image_mem_reqs.size;
        pass = m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_mem_alloc, 0);
        ASSERT_TRUE(pass);
        buffer_mem_alloc.allocationSize = buffer_mem_reqs.size;
        pass = m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_mem_alloc, 0);
        ASSERT_TRUE(pass);

        VkDeviceMemory image_mem = VK_NULL_HANDLE, buffer_mem = VK_NULL_HANDLE;
        err = vk::AllocateMemory(device(), &image_mem_alloc, NULL, &image_mem);
        ASSERT_VK_SUCCESS(err);
        err = vk::AllocateMemory(device(), &buffer_mem_alloc, NULL, &buffer_mem);
        ASSERT_VK_SUCCESS(err);

        vk::FreeMemory(device(), image_mem, NULL);
        vk::FreeMemory(device(), buffer_mem, NULL);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-parameter");
        err = vk::BindImageMemory(device(), image, image_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-parameter");
        err = vk::BindBufferMemory(device(), buffer, buffer_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        vk::DestroyImage(m_device->device(), image, NULL);
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
    }

    // Try to bind memory to an object that already has a memory binding
    {
        VkImage image = VK_NULL_HANDLE;
        err = vk::CreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vk::CreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
        pass = m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        ASSERT_TRUE(pass);
        pass = m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
        ASSERT_TRUE(pass);
        VkDeviceMemory image_mem, buffer_mem;
        err = vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem);
        ASSERT_VK_SUCCESS(err);
        err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
        ASSERT_VK_SUCCESS(err);

        err = vk::BindImageMemory(device(), image, image_mem, 0);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-07460");
        err = vk::BindImageMemory(device(), image, image_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        err = vk::BindBufferMemory(device(), buffer, buffer_mem, 0);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-buffer-07459");
        err = vk::BindBufferMemory(device(), buffer, buffer_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), image_mem, NULL);
        vk::FreeMemory(device(), buffer_mem, NULL);
        vk::DestroyImage(device(), image, NULL);
        vk::DestroyBuffer(device(), buffer, NULL);
    }

    // Try to bind memory to an object with an invalid memoryOffset
    {
        VkImage image = VK_NULL_HANDLE;
        err = vk::CreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vk::CreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        // Leave some extra space for alignment wiggle room
        image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size + buffer_mem_reqs.alignment;
        pass = m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        ASSERT_TRUE(pass);
        pass = m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
        ASSERT_TRUE(pass);
        VkDeviceMemory image_mem, buffer_mem;
        err = vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem);
        ASSERT_VK_SUCCESS(err);
        err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
        ASSERT_VK_SUCCESS(err);

        // Test unaligned memory offset
        {
            if (image_mem_reqs.alignment > 1) {
                VkDeviceSize image_offset = 1;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memoryOffset-01048");
                err = vk::BindImageMemory(device(), image, image_mem, image_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }

            if (buffer_mem_reqs.alignment > 1) {
                VkDeviceSize buffer_offset = 1;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01036");
                err = vk::BindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }
        }

        // Test memory offsets outside the memory allocation
        {
            VkDeviceSize image_offset =
                (image_alloc_info.allocationSize + image_mem_reqs.alignment) & ~(image_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memoryOffset-01046");
            err = vk::BindImageMemory(device(), image, image_mem, image_offset);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();

            VkDeviceSize buffer_offset =
                (buffer_alloc_info.allocationSize + buffer_mem_reqs.alignment) & ~(buffer_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01031");
            err = vk::BindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();
        }

        // Test memory offsets within the memory allocation, but which leave too little memory for
        // the resource.
        {
            VkDeviceSize image_offset = (image_mem_reqs.size - 1) & ~(image_mem_reqs.alignment - 1);
            if ((image_offset > 0) && (image_mem_reqs.size < (image_alloc_info.allocationSize - image_mem_reqs.alignment))) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-size-01049");
                err = vk::BindImageMemory(device(), image, image_mem, image_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }

            VkDeviceSize buffer_offset = (buffer_mem_reqs.size - 1) & ~(buffer_mem_reqs.alignment - 1);
            if (buffer_offset > 0) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-size-01037");
                err = vk::BindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }
        }

        vk::FreeMemory(device(), image_mem, NULL);
        vk::FreeMemory(device(), buffer_mem, NULL);
        vk::DestroyImage(device(), image, NULL);
        vk::DestroyBuffer(device(), buffer, NULL);
    }

    // Try to bind memory to an object with an invalid memory type
    {
        VkImage image = VK_NULL_HANDLE;
        err = vk::CreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vk::CreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
        // Create a mask of available memory types *not* supported by these resources,
        // and try to use one of them.
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);
        VkDeviceMemory image_mem, buffer_mem;

        uint32_t image_unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~image_mem_reqs.memoryTypeBits;
        if (image_unsupported_mem_type_bits != 0) {
            pass = m_device->phy().set_memory_type(image_unsupported_mem_type_bits, &image_alloc_info, 0);
            ASSERT_TRUE(pass);
            err = vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem);
            ASSERT_VK_SUCCESS(err);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-01047");
            err = vk::BindImageMemory(device(), image, image_mem, 0);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();
            vk::FreeMemory(device(), image_mem, NULL);
        }

        uint32_t buffer_unsupported_mem_type_bits =
            ((1 << memory_properties.memoryTypeCount) - 1) & ~buffer_mem_reqs.memoryTypeBits;
        if (buffer_unsupported_mem_type_bits != 0) {
            pass = m_device->phy().set_memory_type(buffer_unsupported_mem_type_bits, &buffer_alloc_info, 0);
            ASSERT_TRUE(pass);
            err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
            ASSERT_VK_SUCCESS(err);
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-01035");
            err = vk::BindBufferMemory(device(), buffer, buffer_mem, 0);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();
            vk::FreeMemory(device(), buffer_mem, NULL);
        }

        vk::DestroyImage(device(), image, NULL);
        vk::DestroyBuffer(device(), buffer, NULL);
    }

    // Try to bind memory to an image created with sparse memory flags
    {
        VkImageCreateInfo sparse_image_create_info = image_create_info;
        sparse_image_create_info.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        VkImageFormatProperties image_format_properties = {};
        err = vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), sparse_image_create_info.format,
                                                         sparse_image_create_info.imageType, sparse_image_create_info.tiling,
                                                         sparse_image_create_info.usage, sparse_image_create_info.flags,
                                                         &image_format_properties);
        if (!m_device->phy().features().sparseResidencyImage2D || err == VK_ERROR_FORMAT_NOT_SUPPORTED) {
            // most likely means sparse formats aren't supported here; skip this test.
        } else {
            ASSERT_VK_SUCCESS(err);
            if (image_format_properties.maxExtent.width == 0) {
                GTEST_SKIP() << "Sparse image format not supported";
            } else {
                VkImage sparse_image = VK_NULL_HANDLE;
                err = vk::CreateImage(m_device->device(), &sparse_image_create_info, NULL, &sparse_image);
                ASSERT_VK_SUCCESS(err);
                VkMemoryRequirements sparse_mem_reqs = {};
                vk::GetImageMemoryRequirements(m_device->device(), sparse_image, &sparse_mem_reqs);
                if (sparse_mem_reqs.memoryTypeBits != 0) {
                    VkMemoryAllocateInfo sparse_mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
                    sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                    sparse_mem_alloc.memoryTypeIndex = 0;
                    pass = m_device->phy().set_memory_type(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                    ASSERT_TRUE(pass);
                    VkDeviceMemory sparse_mem = VK_NULL_HANDLE;
                    err = vk::AllocateMemory(m_device->device(), &sparse_mem_alloc, NULL, &sparse_mem);
                    ASSERT_VK_SUCCESS(err);
                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-01045");
                    err = vk::BindImageMemory(m_device->device(), sparse_image, sparse_mem, 0);
                    // This may very well return an error.
                    (void)err;
                    m_errorMonitor->VerifyFound();
                    vk::FreeMemory(m_device->device(), sparse_mem, NULL);
                }
                vk::DestroyImage(m_device->device(), sparse_image, NULL);
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
            VkBuffer sparse_buffer = VK_NULL_HANDLE;
            err = vk::CreateBuffer(m_device->device(), &sparse_buffer_create_info, NULL, &sparse_buffer);
            ASSERT_VK_SUCCESS(err);
            VkMemoryRequirements sparse_mem_reqs = {};
            vk::GetBufferMemoryRequirements(m_device->device(), sparse_buffer, &sparse_mem_reqs);
            if (sparse_mem_reqs.memoryTypeBits != 0) {
                VkMemoryAllocateInfo sparse_mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
                sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                sparse_mem_alloc.memoryTypeIndex = 0;
                pass = m_device->phy().set_memory_type(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                ASSERT_TRUE(pass);
                VkDeviceMemory sparse_mem = VK_NULL_HANDLE;
                err = vk::AllocateMemory(m_device->device(), &sparse_mem_alloc, NULL, &sparse_mem);
                ASSERT_VK_SUCCESS(err);
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-buffer-01030");
                err = vk::BindBufferMemory(m_device->device(), sparse_buffer, sparse_mem, 0);
                // This may very well return an error.
                (void)err;
                m_errorMonitor->VerifyFound();
                vk::FreeMemory(m_device->device(), sparse_mem, NULL);
            }
            vk::DestroyBuffer(m_device->device(), sparse_buffer, NULL);
        }
    }
}

TEST_F(VkLayerTest, BindInvalidMemoryNoCheck) {
    TEST_DESCRIPTION("Tests case were no call to memory requirements was made prior to binding");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    const bool mp_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    // first test buffer
    {
        VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
        buffer_create_info.flags = 0;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // Create 2 buffers, one that is checked and one that isn't by GetBufferMemoryRequirements
        VkBuffer buffer = VK_NULL_HANDLE;
        VkBuffer unchecked_buffer = VK_NULL_HANDLE;
        VkDeviceMemory buffer_mem = VK_NULL_HANDLE;
        VkDeviceMemory unchecked_buffer_mem = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::CreateBuffer(device(), &buffer_create_info, NULL, &buffer));
        ASSERT_VK_SUCCESS(vk::CreateBuffer(device(), &buffer_create_info, NULL, &unchecked_buffer));

        VkMemoryRequirements buffer_mem_reqs = {};
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        // Leave some extra space for alignment wiggle room
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size + buffer_mem_reqs.alignment;
        ASSERT_TRUE(m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &unchecked_buffer_mem));

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
        VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &image));
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &unchecked_image));

        VkMemoryRequirements image_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        // Leave some extra space for alignment wiggle room
        image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &unchecked_image_mem));

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

        VkImageCreateInfo mp_image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

        // Create aliased function pointers for 1.0 and 1.1 contexts
        PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
        PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2Function = nullptr;

        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vkBindImageMemory2Function = vk::BindImageMemory2;
            vkGetImageMemoryRequirements2Function = vk::GetImageMemoryRequirements2;
        } else {
            vkGetImageMemoryRequirements2Function =
                (PFN_vkGetImageMemoryRequirements2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkGetImageMemoryRequirements2KHR");
            vkBindImageMemory2Function =
                (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
        }

        VkImage mp_image = VK_NULL_HANDLE;
        VkImage mp_unchecked_image = VK_NULL_HANDLE;
        // Array represent planes for disjoint images
        VkDeviceMemory mp_image_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        VkDeviceMemory mp_unchecked_image_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        VkMemoryRequirements2 mp_image_mem_reqs2[2];
        VkMemoryAllocateInfo mp_image_alloc_info[2];

        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_image));
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_unchecked_image));

        VkImagePlaneMemoryRequirementsInfo image_plane_req = LvlInitStruct<VkImagePlaneMemoryRequirementsInfo>();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
        mem_req_info2.image = mp_image;
        mp_image_mem_reqs2[0] = LvlInitStruct<VkMemoryRequirements2>();
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
        mp_image_mem_reqs2[1] = LvlInitStruct<VkMemoryRequirements2>();
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[1]);

        mp_image_alloc_info[0] = LvlInitStruct<VkMemoryAllocateInfo>();
        mp_image_alloc_info[1] = LvlInitStruct<VkMemoryAllocateInfo>();

        mp_image_alloc_info[0].allocationSize = mp_image_mem_reqs2[0].memoryRequirements.size;
        ASSERT_TRUE(
            m_device->phy().set_memory_type(mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[0], 0));
        // Leave some extra space for alignment wiggle room
        mp_image_alloc_info[1].allocationSize =
            mp_image_mem_reqs2[1].memoryRequirements.size + mp_image_mem_reqs2[1].memoryRequirements.alignment;
        ASSERT_TRUE(
            m_device->phy().set_memory_type(mp_image_mem_reqs2[1].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[1], 0));

        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_image_mem[0]));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[1], NULL, &mp_image_mem[1]));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_unchecked_image_mem[0]));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[1], NULL, &mp_unchecked_image_mem[1]));

        // Sets an invalid offset to plane 1
        if (mp_image_mem_reqs2[1].memoryRequirements.alignment > 1) {
            VkBindImagePlaneMemoryInfo plane_memory_info[2];
            plane_memory_info[0] = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
            plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
            plane_memory_info[1] = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
            plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[0]);
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[1]);
            bind_image_info[1].image = mp_image;
            bind_image_info[1].memory = mp_image_mem[1];
            bind_image_info[1].memoryOffset = 1;  // off alignment

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01620");
            vkBindImageMemory2Function(device(), 2, bind_image_info);
            m_errorMonitor->VerifyFound();

            // Should trigger same VUID even when image was never checked
            // this makes an assumption that the driver will return the same image requirements for same createImageInfo where even
            // being close to running out of heap space
            bind_image_info[0].image = mp_unchecked_image;
            bind_image_info[0].memory = mp_unchecked_image_mem[0];
            bind_image_info[1].image = mp_unchecked_image;
            bind_image_info[1].memory = mp_unchecked_image_mem[1];
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01620");
            vkBindImageMemory2Function(device(), 2, bind_image_info);
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

TEST_F(VkLayerTest, BindInvalidMemory2BindInfos) {
    TEST_DESCRIPTION("These tests deal with VK_KHR_bind_memory_2 and invalid VkBindImageMemoryInfo* pBindInfos");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool mp_extensions = IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Create aliased function pointers for 1.0 and 1.1 contexts
    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkBindImageMemory2Function = vk::BindImageMemory2;
    } else {
        vkBindImageMemory2Function = (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
    }

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &image_a));
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &image_b));

        VkMemoryRequirements image_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image_a, &image_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_a_mem));
        vk::GetImageMemoryRequirements(device(), image_b, &image_mem_reqs);
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_b_mem));

        // Try binding same image twice in array
        VkBindImageMemoryInfo bind_image_info[3];
        bind_image_info[0] = LvlInitStruct<VkBindImageMemoryInfo>();
        bind_image_info[0].image = image_a;
        bind_image_info[0].memory = image_a_mem;
        bind_image_info[0].memoryOffset = 0;
        bind_image_info[1] = LvlInitStruct<VkBindImageMemoryInfo>();
        bind_image_info[1].image = image_b;
        bind_image_info[1].memory = image_b_mem;
        bind_image_info[1].memoryOffset = 0;
        bind_image_info[2] = bind_image_info[0];  // duplicate bind

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-04006");
        vkBindImageMemory2Function(device(), 3, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Bind same image to 2 different memory in same array
        bind_image_info[1].image = image_a;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-04006");
        vkBindImageMemory2Function(device(), 2, bind_image_info);
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

        PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2Function = nullptr;
        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vkGetImageMemoryRequirements2Function = vk::GetImageMemoryRequirements2;
        } else {
            vkGetImageMemoryRequirements2Function =
                (PFN_vkGetImageMemoryRequirements2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkGetImageMemoryRequirements2KHR");
        }

        // Creat 1 normal, not disjoint image
        VkImage normal_image = VK_NULL_HANDLE;
        VkDeviceMemory normal_image_mem = VK_NULL_HANDLE;
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &normal_image));
        VkMemoryRequirements image_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), normal_image, &image_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &normal_image_mem));

        // Create 2 disjoint images with memory backing each plane
        VkImageCreateInfo mp_image_create_info = image_create_info;
        mp_image_create_info.format = mp_format;
        mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

        VkImage mp_image_a = VK_NULL_HANDLE;
        VkImage mp_image_b = VK_NULL_HANDLE;
        VkDeviceMemory mp_image_a_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        VkDeviceMemory mp_image_b_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_image_a));
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_image_b));

        AllocateDisjointMemory(m_device, vkGetImageMemoryRequirements2Function, mp_image_a, &mp_image_a_mem[0],
                               VK_IMAGE_ASPECT_PLANE_0_BIT);
        AllocateDisjointMemory(m_device, vkGetImageMemoryRequirements2Function, mp_image_a, &mp_image_a_mem[1],
                               VK_IMAGE_ASPECT_PLANE_1_BIT);
        AllocateDisjointMemory(m_device, vkGetImageMemoryRequirements2Function, mp_image_b, &mp_image_b_mem[0],
                               VK_IMAGE_ASPECT_PLANE_0_BIT);
        AllocateDisjointMemory(m_device, vkGetImageMemoryRequirements2Function, mp_image_b, &mp_image_b_mem[1],
                               VK_IMAGE_ASPECT_PLANE_1_BIT);

        VkBindImagePlaneMemoryInfo plane_memory_info[2];
        plane_memory_info[0] = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
        plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        plane_memory_info[1] = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
        plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

        // set all sType and memoryOffset as they are the same
        VkBindImageMemoryInfo bind_image_info[6];
        for (int i = 0; i < 6; i++) {
            bind_image_info[i] = LvlInitStruct<VkBindImageMemoryInfo>();
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
        vkBindImageMemory2Function(device(), 3, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Same thing, but mix in a non-disjoint image
        bind_image_info[3].pNext = nullptr;
        bind_image_info[3].image = normal_image;
        bind_image_info[3].memory = normal_image_mem;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-02858");
        vkBindImageMemory2Function(device(), 4, bind_image_info);
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
        vkBindImageMemory2Function(device(), 6, bind_image_info);
        m_errorMonitor->VerifyFound();

        // Try binding image_a with no plane specified
        bind_image_info[0].pNext = nullptr;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-image-07736");
        vkBindImageMemory2Function(device(), 1, bind_image_info);
        m_errorMonitor->VerifyFound();
        bind_image_info[0].pNext = (void *)&plane_memory_info[0];

        // Valid case of binding 2 disjoint image and normal image by removing duplicate
        vkBindImageMemory2Function(device(), 5, bind_image_info);

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

TEST_F(VkLayerTest, BindMemoryToDestroyedObject) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-parameter");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create an image object, allocate memory, destroy the object and then try
    // to bind it
    VkImage image;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    // Allocate memory
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    // Introduce validation failure, destroy Image object before binding
    vk::DestroyImage(m_device->device(), image, NULL);
    ASSERT_VK_SUCCESS(err);

    // Now Try to bind memory to this destroyed object
    err = vk::BindImageMemory(m_device->device(), image, mem, 0);
    // This may very well return an error.
    (void)err;

    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), mem, NULL);
}

TEST_F(VkLayerTest, ExceedMemoryAllocationCount) {
    VkResult err = VK_SUCCESS;
    const int max_mems = 32;
    VkDeviceMemory mems[max_mems + 1];

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

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
    ASSERT_NO_FATAL_FAILURE(InitState());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "Number of currently valid memory objects is not less than the maximum allowed");

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
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

TEST_F(VkLayerTest, ImageMemoryNotBound) {
    TEST_DESCRIPTION("Attempt to draw with an image which has not had memory bound to it.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkImage image;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
    ASSERT_VK_SUCCESS(err);
    // Have to bind memory to image before recording cmd in cmd buffer using it
    VkMemoryRequirements mem_reqs;
    VkDeviceMemory image_mem;
    bool pass;
    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &image_mem);
    ASSERT_VK_SUCCESS(err);

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

TEST_F(VkLayerTest, BufferMemoryNotBound) {
    TEST_DESCRIPTION("Attempt to copy from a buffer which has not had memory bound to it.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
               VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buf_info.size = 1024;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = 1024;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        GTEST_SKIP() << "Failed to set memory type";
    }
    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

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

TEST_F(VkLayerTest, DedicatedAllocationBinding) {
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize resource_size = 1024;
    auto buffer_info = VkBufferObj::create_info(resource_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_info);
    auto buffer_alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(), mem_flags);
    auto buffer_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfoKHR>();
    buffer_dedicated_info.buffer = buffer.handle();
    buffer_alloc_info.pNext = &buffer_dedicated_info;
    vk_testing::DeviceMemory dedicated_buffer_memory;
    dedicated_buffer_memory.init(*m_device, buffer_alloc_info);

    VkBufferObj wrong_buffer;
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

    auto image_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfoKHR>();
    image_dedicated_info.image = image.handle();
    auto image_alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vk_testing::DeviceMemory dedicated_image_memory;
    dedicated_image_memory.init(*m_device, image_alloc_info);

    // Bind with wrong image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-01509");
    vk::BindImageMemory(m_device->handle(), wrong_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset (same VUID)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-memory-01509");  // offset must be zero
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-size-01049");  // offset pushes us past size
    auto image_offset = image.memory_requirements().alignment;
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), image_offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);
}

TEST_F(VkLayerTest, DedicatedAllocationImageAliasing) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto aliasing_features = LvlInitStruct<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>();
    GetPhysicalDeviceFeatures2(aliasing_features);
    if (aliasing_features.dedicatedAllocationImageAliasing != VK_TRUE) {
        GTEST_SKIP() << "dedicatedAllocationImageAliasing feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &aliasing_features));

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

    auto image_dedicated_info = LvlInitStruct<VkMemoryDedicatedAllocateInfoKHR>();
    image_dedicated_info.image = image->handle();
    auto image_alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image->memory_requirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vk_testing::DeviceMemory dedicated_image_memory;
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

TEST_F(VkLayerTest, BufferDeviceAddressEXT) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferAddressFeaturesEXT>();
    GetPhysicalDeviceFeatures2(buffer_device_address_features);
    if (buffer_device_address_features.bufferDeviceAddressCaptureReplay != VK_TRUE) {
        GTEST_SKIP() << "bufferDeviceAddressCaptureReplay feature not supported";
    }

    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &buffer_device_address_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressEXT vkGetBufferDeviceAddressEXT =
        (PFN_vkGetBufferDeviceAddressEXT)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressEXT");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferDeviceAddressCreateInfoEXT addr_ci = LvlInitStruct<VkBufferDeviceAddressCreateInfoEXT>();
    addr_ci.deviceAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-deviceAddress-02604");

    buffer_create_info.pNext = nullptr;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoEXT info = LvlInitStruct<VkBufferDeviceAddressInfoEXT>();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vkGetBufferDeviceAddressEXT(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(m_device->device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);

    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, BufferDeviceAddressEXTDisabled) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    AddRequiredExtensions(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferAddressFeaturesEXT>();
    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &buffer_device_address_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressEXT vkGetBufferDeviceAddressEXT =
        (PFN_vkGetBufferDeviceAddressEXT)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressEXT");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoEXT info = LvlInitStruct<VkBufferDeviceAddressInfoEXT>();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02601");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vkGetBufferDeviceAddressEXT(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, BufferDeviceAddressKHR) {
    TEST_DESCRIPTION("Test VK_KHR_buffer_device_address.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    GetPhysicalDeviceFeatures2(buffer_device_address_features);
    if (buffer_device_address_features.bufferDeviceAddress != VK_TRUE) {
        GTEST_SKIP() << "bufferDeviceAddress feature not supported";
    }

    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &buffer_device_address_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");
    PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR vkGetDeviceMemoryOpaqueCaptureAddressKHR =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetDeviceMemoryOpaqueCaptureAddressKHR");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferOpaqueCaptureAddressCreateInfoKHR addr_ci = LvlInitStruct<VkBufferOpaqueCaptureAddressCreateInfoKHR>();
    addr_ci.opaqueCaptureAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337");

    buffer_create_info.pNext = nullptr;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoKHR info = LvlInitStruct<VkBufferDeviceAddressInfoKHR>();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vkGetBufferDeviceAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
        auto fpGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddress");
        if (nullptr == fpGetBufferDeviceAddress) {
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkGetBufferDeviceAddress");
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
            fpGetBufferDeviceAddress(m_device->device(), &info);
            m_errorMonitor->VerifyFound();
        }
    }

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-bufferDeviceAddress-03339");
    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);
    m_errorMonitor->VerifyFound();

    VkDeviceMemoryOpaqueCaptureAddressInfoKHR mem_opaque_addr_info = LvlInitStruct<VkDeviceMemoryOpaqueCaptureAddressInfoKHR>();
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vkGetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
        auto fpGetDeviceMemoryOpaqueCaptureAddress =
            (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)vk::GetDeviceProcAddr(device(), "vkGetDeviceMemoryOpaqueCaptureAddress");
        if (nullptr == fpGetDeviceMemoryOpaqueCaptureAddress) {
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkGetDeviceMemoryOpaqueCaptureAddress");
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
            fpGetDeviceMemoryOpaqueCaptureAddress(m_device->device(), &mem_opaque_addr_info);
            m_errorMonitor->VerifyFound();
        }
    }

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);

    mem_opaque_addr_info.memory = buffer_mem;
    vkGetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);

    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);

    vkGetBufferDeviceAddressKHR(m_device->device(), &info);

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, BufferDeviceAddressKHRDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_buffer_device_address.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &buffer_device_address_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");
    PFN_vkGetBufferOpaqueCaptureAddressKHR vkGetBufferOpaqueCaptureAddressKHR =
        (PFN_vkGetBufferOpaqueCaptureAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferOpaqueCaptureAddressKHR");
    PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR vkGetDeviceMemoryOpaqueCaptureAddressKHR =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetDeviceMemoryOpaqueCaptureAddressKHR");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoKHR info = LvlInitStruct<VkBufferDeviceAddressInfoKHR>();
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferDeviceAddress-bufferDeviceAddress-03324");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02601");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vkGetBufferDeviceAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferOpaqueCaptureAddress-None-03326");
    vkGetBufferOpaqueCaptureAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
        auto fpGetBufferOpaqueCaptureAddress =
            (PFN_vkGetBufferOpaqueCaptureAddress)vk::GetDeviceProcAddr(device(), "vkGetBufferOpaqueCaptureAddress");
        if (nullptr == fpGetBufferOpaqueCaptureAddress) {
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkGetBufferOpaqueCaptureAddress");
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferOpaqueCaptureAddress-None-03326");
            fpGetBufferOpaqueCaptureAddress(m_device->device(), &info);
            m_errorMonitor->VerifyFound();
        }
    }

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);

    VkDeviceMemoryOpaqueCaptureAddressInfoKHR mem_opaque_addr_info = LvlInitStruct<VkDeviceMemoryOpaqueCaptureAddressInfoKHR>();
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceMemoryOpaqueCaptureAddress-None-03334");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vkGetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-flags-03330");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-flags-03331");
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, InvalidMemoryType) {
    // Attempts to allocate from a memory type that doesn't exist

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-pAllocateInfo-01714");

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.memoryTypeIndex = memory_info.memoryTypeCount;
    mem_alloc.allocationSize = 4;

    VkDeviceMemory mem;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AllocationBeyondHeapSize) {
    // Attempts to allocate a single piece of memory that's larger than the heap size

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-pAllocateInfo-01713");

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = memory_info.memoryHeaps[memory_info.memoryTypes[0].heapIndex].size + 1;

    VkDeviceMemory mem;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceCoherentMemoryDisabledAMD) {
    // Attempts to allocate device coherent memory without enabling the extension/feature
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (IsPlatform(kMockICD)) {
        GTEST_SKIP() << "Test not supported by MockICD, does not support the necessary memory type";
    }

    auto coherent_memory_features_amd = LvlInitStruct<VkPhysicalDeviceCoherentMemoryFeaturesAMD>();
    coherent_memory_features_amd.deviceCoherentMemory = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &coherent_memory_features_amd));

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

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.memoryTypeIndex = deviceCoherentMemoryTypeIndex;
    mem_alloc.allocationSize = 4;

    VkDeviceMemory mem;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DedicatedAllocation) {
    TEST_DESCRIPTION("Create invalid requests to dedicated allocation of memory");

    // Both VK_KHR_dedicated_allocation and VK_KHR_sampler_ycbcr_conversion supported in 1.1
    // Quicke to set 1.1 then check all extensions in 1.0
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    const VkFormat disjoint_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    const VkFormat normal_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), disjoint_format, &format_properties);

    bool sparse_support = (m_device->phy().features().sparseBinding == VK_TRUE);
    bool disjoint_support = ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) != 0);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.size = 2048;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = NULL;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
    VkMemoryDedicatedAllocateInfo dedicated_allocate_info = LvlInitStruct<VkMemoryDedicatedAllocateInfo>();
    VkMemoryAllocateInfo memory_allocate_info = LvlInitStruct<VkMemoryAllocateInfo>(&dedicated_allocate_info);
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
        VkImagePlaneMemoryRequirementsInfo image_plane_req = LvlInitStruct<VkImagePlaneMemoryRequirementsInfo>();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
        VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
        mem_req_info2.image = disjoint_image;
        VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();

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

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    const char *image_vuid = IsExtensionsEnabled(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)
                                 ? "VUID-VkMemoryDedicatedAllocateInfo-image-02964"
                                 : "VUID-VkMemoryDedicatedAllocateInfo-image-01433";
    const char *buffer_vuid = IsExtensionsEnabled(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)
                                  ? "VUID-VkMemoryDedicatedAllocateInfo-buffer-02965"
                                  : "VUID-VkMemoryDedicatedAllocateInfo-buffer-01435";
#else
    const char *image_vuid = "VUID-VkMemoryDedicatedAllocateInfo-image-01433";
    const char *buffer_vuid = "VUID-VkMemoryDedicatedAllocateInfo-buffer-01435";
#endif
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, image_vuid);
    vk::AllocateMemory(m_device->device(), &memory_allocate_info, NULL, &device_memory);
    m_errorMonitor->VerifyFound();

    memory_allocate_info.allocationSize = normal_buffer_memory_req.size - 1;
    dedicated_allocate_info.image = VK_NULL_HANDLE;
    dedicated_allocate_info.buffer = normal_buffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, buffer_vuid);
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

TEST_F(VkLayerTest, InvalidMemoryRequirements) {
    TEST_DESCRIPTION("Create invalid requests to image and buffer memory requirments.");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool drm_format_modifier = IsExtensionsEnabled(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Need to make sure disjoint is supported for format
    // Also need to support an arbitrary image usage feature
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &format_properties);
    if (!((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
          (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
        GTEST_SKIP() << "test requires disjoint/sampled feature bit on format";
    } else {
        VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
        ASSERT_VK_SUCCESS(err);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-01588");
        VkMemoryRequirements memory_requirements;
        vk::GetImageMemoryRequirements(m_device->device(), image, &memory_requirements);
        m_errorMonitor->VerifyFound();

        PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2Function =
            (PFN_vkGetImageMemoryRequirements2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkGetImageMemoryRequirements2KHR");
        ASSERT_TRUE(vkGetImageMemoryRequirements2Function != nullptr);

        VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>();
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01589");
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        // Point to a 3rd plane for a 2-plane format
        VkImagePlaneMemoryRequirementsInfo image_plane_req = LvlInitStruct<VkImagePlaneMemoryRequirementsInfo>();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281");
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        // Test with a non planar image aspect also
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImagePlaneMemoryRequirementsInfo-planeAspect-02281");
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        vk::DestroyImage(m_device->device(), image, nullptr);

        // Recreate image without Disjoint bit
        image_create_info.flags = 0;
        err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01590");
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        vk::DestroyImage(m_device->device(), image, nullptr);

        // Recreate image with single plane format and with Disjoint bit
        image_create_info.flags = 0;
        image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;

        // Disjoint bit isn't set as likely not even supported by non-planar format
        const char *vuid = drm_format_modifier ? "VUID-VkImageMemoryRequirementsInfo2-image-02280"
                                               : "VUID-VkImageMemoryRequirementsInfo2-image-01591";
        m_errorMonitor->SetUnexpectedError("VUID-VkImageMemoryRequirementsInfo2-image-01590");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
        m_errorMonitor->VerifyFound();

        vk::DestroyImage(m_device->device(), image, nullptr);

        // TODO - Test needs to use a real drm format and not try using ALIAS_BIT
        // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4793#issuecomment-1312724486
    }
}

TEST_F(VkLayerTest, InvalidMemoryAllocatepNextChain) {
    // Attempts to allocate from a memory type that doesn't exist

    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    // Check for external memory device extensions
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    VkExportMemoryAllocateInfoNV export_memory_info_nv = LvlInitStruct<VkExportMemoryAllocateInfoNV>();
    export_memory_info_nv.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_NV;

    VkExportMemoryAllocateInfo export_memory_info = LvlInitStruct<VkExportMemoryAllocateInfo>(&export_memory_info_nv);
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>(&export_memory_info);
    mem_alloc.memoryTypeIndex = memory_info.memoryTypeCount;
    mem_alloc.allocationSize = 4;

    VkDeviceMemory mem;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-00640");
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    m_errorMonitor->VerifyFound();

#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkExportMemoryWin32HandleInfoNV export_memory_info_win32_nv = LvlInitStruct<VkExportMemoryWin32HandleInfoNV>();
    export_memory_info_win32_nv.pAttributes = nullptr;
    export_memory_info_win32_nv.dwAccess = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-pNext-00640");
    export_memory_info.pNext = &export_memory_info_win32_nv;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    m_errorMonitor->VerifyFound();
#endif
}

TEST_F(VkLayerTest, DeviceImageMemoryRequirementsSwapchain) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirementsKHR =
        reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirementsKHR>(
            vk::GetInstanceProcAddr(instance(), "vkGetDeviceImageMemoryRequirementsKHR"));
    assert(vkGetDeviceImageMemoryRequirementsKHR != nullptr);

    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>(&image_swapchain_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    auto device_image_memory_requirements = LvlInitStruct<VkDeviceImageMemoryRequirementsKHR>();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;

    VkMemoryRequirements2 memory_requirements = LvlInitStruct<VkMemoryRequirements2>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirementsKHR-pCreateInfo-06416");
    vkGetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceImageMemoryRequirementsDrmFormatModifier) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirementsKHR =
        reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirementsKHR>(
            vk::GetInstanceProcAddr(instance(), "vkGetDeviceImageMemoryRequirementsKHR"));
    assert(vkGetDeviceImageMemoryRequirementsKHR != nullptr);

    VkSubresourceLayout planeLayout = {0, 0, 0, 0, 0};
    auto drm_format_modifier_create_info = LvlInitStruct<VkImageDrmFormatModifierExplicitCreateInfoEXT>();
    drm_format_modifier_create_info.drmFormatModifierPlaneCount = 1;
    drm_format_modifier_create_info.pPlaneLayouts = &planeLayout;

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>(&drm_format_modifier_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    auto device_image_memory_requirements = LvlInitStruct<VkDeviceImageMemoryRequirementsKHR>();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_COLOR_BIT;

    VkMemoryRequirements2 memory_requirements = LvlInitStruct<VkMemoryRequirements2>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirements-pCreateInfo-06776");
    vkGetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceImageMemoryRequirementsDisjoint) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    const VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) == 0) {
        GTEST_SKIP() << "Test requires disjoint support extensions";
    }

    PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirementsKHR =
        reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirementsKHR>(
            vk::GetInstanceProcAddr(instance(), "vkGetDeviceImageMemoryRequirementsKHR"));
    assert(vkGetDeviceImageMemoryRequirementsKHR != nullptr);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;

    auto device_image_memory_requirements = LvlInitStruct<VkDeviceImageMemoryRequirementsKHR>();
    device_image_memory_requirements.pCreateInfo = &image_create_info;
    device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_NONE_KHR;

    VkMemoryRequirements2 memory_requirements = LvlInitStruct<VkMemoryRequirements2>();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceImageMemoryRequirementsKHR-pCreateInfo-06417");
    vkGetDeviceImageMemoryRequirementsKHR(device(), &device_image_memory_requirements, &memory_requirements);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestBindBufferMemoryDeviceGroup) {
    TEST_DESCRIPTION("Test VkBindBufferMemoryDeviceGroupInfo.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    auto create_device_pnext = LvlInitStruct<VkDeviceGroupDeviceCreateInfo>();
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
        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext));
    } else {
        GTEST_SKIP() << "Test requires a physical device group with more than 1 device";
    }

    VkBufferObj buffer;
    auto buffer_info = VkBufferObj::create_info(4096, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buffer.init_no_mem(*m_device, buffer_info);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    buffer_alloc_info.memoryTypeIndex = 0;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;

    VkDeviceMemory buffer_memory;
    vk::AllocateMemory(m_device->device(), &buffer_alloc_info, nullptr, &buffer_memory);

    std::vector<uint32_t> device_indices(create_device_pnext.physicalDeviceCount);

    VkBindBufferMemoryDeviceGroupInfo bind_buffer_memory_device_group = LvlInitStruct<VkBindBufferMemoryDeviceGroupInfo>();
    bind_buffer_memory_device_group.deviceIndexCount = 1;
    bind_buffer_memory_device_group.pDeviceIndices = device_indices.data();

    VkBindBufferMemoryInfo bind_buffer_info = LvlInitStruct<VkBindBufferMemoryInfo>(&bind_buffer_memory_device_group);
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
    VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    image_alloc_info.memoryTypeIndex = 0;
    image_alloc_info.allocationSize = image_mem_reqs.size;

    VkDeviceMemory image_memory;
    vk::AllocateMemory(m_device->device(), &image_alloc_info, nullptr, &image_memory);

    VkBindImageMemoryDeviceGroupInfo bind_image_memory_device_group = LvlInitStruct<VkBindImageMemoryDeviceGroupInfo>();
    bind_image_memory_device_group.deviceIndexCount = 1;
    bind_image_memory_device_group.pDeviceIndices = device_indices.data();

    VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>(&bind_image_memory_device_group);
    bind_image_info.image = image.handle();
    bind_image_info.memory = image_memory;
    bind_image_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01634");
    vk::BindImageMemory2(device(), 1, &bind_image_info);
    m_errorMonitor->VerifyFound();
}
