/*
 * Copyright (c) 2015-2019 The Khronos Group Inc.
 * Copyright (c) 2015-2019 Valve Corporation
 * Copyright (c) 2015-2019 LunarG, Inc.
 * Copyright (c) 2015-2019 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Chris Forbes <chrisf@ijw.co.nz>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mike Stroyan <mike@LunarG.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Tony Barbour <tony@LunarG.com>
 * Author: Cody Northrop <cnorthrop@google.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 * Author: Jeremy Kniager <jeremyk@lunarg.com>
 * Author: Shannon McPherson <shannon@lunarg.com>
 * Author: John Zulauf <jzulauf@lunarg.com>
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, MirrorClampToEdgeNotEnabled) {
    TEST_DESCRIPTION("Validation should catch using CLAMP_TO_EDGE addressing mode if the extension is not enabled.");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkSamplerCreateInfo-addressModeU-01079");
    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    // Set the modes to cause the error
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

    vkCreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AnisotropyFeatureDisabled) {
    TEST_DESCRIPTION("Validation should check anisotropy parameters are correct with samplerAnisotropy disabled.");

    // Determine if required device features are available
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    device_features.samplerAnisotropy = VK_FALSE;  // force anisotropy off
    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkSamplerCreateInfo-anisotropyEnable-01070");
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    // With the samplerAnisotropy disable, the sampler must not enable it.
    sampler_info.anisotropyEnable = VK_TRUE;
    VkSampler sampler = VK_NULL_HANDLE;

    VkResult err;
    err = vkCreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == err) {
        vkDestroySampler(m_device->device(), sampler, NULL);
    }
    sampler = VK_NULL_HANDLE;
}

TEST_F(VkLayerTest, AnisotropyFeatureEnabled) {
    TEST_DESCRIPTION("Validation must check several conditions that apply only when Anisotropy is enabled.");

    // Determine if required device features are available
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    // These tests require that the device support anisotropic filtering
    if (VK_TRUE != device_features.samplerAnisotropy) {
        printf("%s Test requires unsupported samplerAnisotropy feature. Skipped.\n", kSkipPrefix);
        return;
    }

    bool cubic_support = false;
    if (DeviceExtensionSupported(gpu(), nullptr, "VK_IMG_filter_cubic")) {
        m_device_extension_names.push_back("VK_IMG_filter_cubic");
        cubic_support = true;
    }

    VkSamplerCreateInfo sampler_info_ref = SafeSaneSamplerCreateInfo();
    sampler_info_ref.anisotropyEnable = VK_TRUE;
    VkSamplerCreateInfo sampler_info = sampler_info_ref;
    ASSERT_NO_FATAL_FAILURE(InitState());

    // maxAnisotropy out-of-bounds low.
    sampler_info.maxAnisotropy = NearestSmaller(1.0F);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-anisotropyEnable-01071");
    sampler_info.maxAnisotropy = sampler_info_ref.maxAnisotropy;

    // maxAnisotropy out-of-bounds high.
    sampler_info.maxAnisotropy = NearestGreater(m_device->phy().properties().limits.maxSamplerAnisotropy);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-anisotropyEnable-01071");
    sampler_info.maxAnisotropy = sampler_info_ref.maxAnisotropy;

    // Both anisotropy and unnormalized coords enabled
    sampler_info.unnormalizedCoordinates = VK_TRUE;
    // If unnormalizedCoordinates is VK_TRUE, minLod and maxLod must be zero
    sampler_info.minLod = 0;
    sampler_info.maxLod = 0;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076");
    sampler_info.unnormalizedCoordinates = sampler_info_ref.unnormalizedCoordinates;

    // Both anisotropy and cubic filtering enabled
    if (cubic_support) {
        sampler_info.minFilter = VK_FILTER_CUBIC_IMG;
        CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-magFilter-01081");
        sampler_info.minFilter = sampler_info_ref.minFilter;

        sampler_info.magFilter = VK_FILTER_CUBIC_IMG;
        CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-magFilter-01081");
        sampler_info.magFilter = sampler_info_ref.magFilter;
    } else {
        printf("%s Test requires unsupported extension \"VK_IMG_filter_cubic\". Skipped.\n", kSkipPrefix);
    }
}

TEST_F(VkLayerTest, UnnormalizedCoordinatesEnabled) {
    TEST_DESCRIPTION("Validate restrictions on sampler parameters when unnormalizedCoordinates is true.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    VkSamplerCreateInfo sampler_info_ref = SafeSaneSamplerCreateInfo();
    sampler_info_ref.unnormalizedCoordinates = VK_TRUE;
    sampler_info_ref.minLod = 0.0f;
    sampler_info_ref.maxLod = 0.0f;
    VkSamplerCreateInfo sampler_info = sampler_info_ref;
    ASSERT_NO_FATAL_FAILURE(InitState());

    // min and mag filters must be the same
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072");
    std::swap(sampler_info.minFilter, sampler_info.magFilter);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072");
    sampler_info = sampler_info_ref;

    // mipmapMode must be NEAREST
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01073");
    sampler_info = sampler_info_ref;

    // minlod and maxlod must be zero
    sampler_info.maxLod = 3.14159f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074");
    sampler_info.minLod = 2.71828f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074");
    sampler_info = sampler_info_ref;

    // addressModeU and addressModeV must both be CLAMP_TO_EDGE or CLAMP_TO_BORDER
    // checks all 12 invalid combinations out of 16 total combinations
    const std::array<VkSamplerAddressMode, 4> kAddressModes = {{
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    }};
    for (const auto umode : kAddressModes) {
        for (const auto vmode : kAddressModes) {
            if ((umode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE && umode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
                (vmode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE && vmode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
                sampler_info.addressModeU = umode;
                sampler_info.addressModeV = vmode;
                CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01075");
            }
        }
    }
    sampler_info = sampler_info_ref;

    // VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076 is tested in AnisotropyFeatureEnabled above
    // Since it requires checking/enabling the anisotropic filtering feature, it's easier to do it
    // with the other anisotropic tests.

    // compareEnable must be VK_FALSE
    sampler_info.compareEnable = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01077");
    sampler_info = sampler_info_ref;
}

TEST_F(VkLayerTest, UpdateBufferAlignment) {
    TEST_DESCRIPTION("Check alignment parameters for vkCmdUpdateBuffer");
    uint32_t updateData[] = {1, 2, 3, 4, 5, 6, 7, 8};

    ASSERT_NO_FATAL_FAILURE(Init());

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkBufferObj buffer;
    buffer.init_as_dst(*m_device, (VkDeviceSize)20, reqs);

    m_commandBuffer->begin();
    // Introduce failure by using dstOffset that is not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " is not a multiple of 4");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 1, 4, updateData);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using dataSize that is not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " is not a multiple of 4");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 0, 6, updateData);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using dataSize that is < 0
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "must be greater than zero and less than or equal to 65536");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 0, (VkDeviceSize)-44, updateData);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using dataSize that is > 65536
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "must be greater than zero and less than or equal to 65536");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 0, (VkDeviceSize)80000, updateData);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, FillBufferAlignment) {
    TEST_DESCRIPTION("Check alignment parameters for vkCmdFillBuffer");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkBufferObj buffer;
    buffer.init_as_dst(*m_device, (VkDeviceSize)20, reqs);

    m_commandBuffer->begin();

    // Introduce failure by using dstOffset that is not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " is not a multiple of 4");
    m_commandBuffer->FillBuffer(buffer.handle(), 1, 4, 0x11111111);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using size that is not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " is not a multiple of 4");
    m_commandBuffer->FillBuffer(buffer.handle(), 0, 6, 0x11111111);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using size that is zero
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "must be greater than zero");
    m_commandBuffer->FillBuffer(buffer.handle(), 0, 0, 0x11111111);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SparseBindingImageBufferCreate) {
    TEST_DESCRIPTION("Create buffer/image with sparse attributes but without the sparse_binding bit set");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buf_info.size = 2048;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (m_device->phy().features().sparseResidencyBuffer) {
        buf_info.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
        CreateBufferTest(*this, &buf_info, "VUID-VkBufferCreateInfo-flags-00918");
    } else {
        printf("%s Test requires unsupported sparseResidencyBuffer feature. Skipped.\n", kSkipPrefix);
        return;
    }

    if (m_device->phy().features().sparseResidencyAliased) {
        buf_info.flags = VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
        CreateBufferTest(*this, &buf_info, "VUID-VkBufferCreateInfo-flags-00918");
    } else {
        printf("%s Test requires unsupported sparseResidencyAliased feature. Skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (m_device->phy().features().sparseResidencyImage2D) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-00987");
    } else {
        printf("%s Test requires unsupported sparseResidencyImage2D feature. Skipped.\n", kSkipPrefix);
        return;
    }

    if (m_device->phy().features().sparseResidencyAliased) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-00987");
    } else {
        printf("%s Test requires unsupported sparseResidencyAliased feature. Skipped.\n", kSkipPrefix);
        return;
    }
}

TEST_F(VkLayerTest, SparseResidencyImageCreateUnsupportedTypes) {
    TEST_DESCRIPTION("Create images with sparse residency with unsupported types");

    // Determine which device feature are available
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    // Mask out device features we don't want and initialize device state
    device_features.sparseResidencyImage2D = VK_FALSE;
    device_features.sparseResidencyImage3D = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

    if (!m_device->phy().features().sparseBinding) {
        printf("%s Test requires unsupported sparseBinding feature. Skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;

    // 1D image w/ sparse residency is an error
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00970");

    // 2D image w/ sparse residency when feature isn't available
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 64;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00971");

    // 3D image w/ sparse residency when feature isn't available
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent.depth = 8;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00972");
}

TEST_F(VkLayerTest, SparseResidencyImageCreateUnsupportedSamples) {
    TEST_DESCRIPTION("Create images with sparse residency with unsupported tiling or sample counts");

    // Determine which device feature are available
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    // These tests require that the device support sparse residency for 2D images
    if (VK_TRUE != device_features.sparseResidencyImage2D) {
        printf("%s Test requires unsupported SparseResidencyImage2D feature. Skipped.\n", kSkipPrefix);
        return;
    }

    // Mask out device features we don't want and initialize device state
    device_features.sparseResidency2Samples = VK_FALSE;
    device_features.sparseResidency4Samples = VK_FALSE;
    device_features.sparseResidency8Samples = VK_FALSE;
    device_features.sparseResidency16Samples = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;

    // 2D image w/ sparse residency and linear tiling is an error
    CreateImageTest(*this, &image_create_info,
                    "VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT then image tiling of VK_IMAGE_TILING_LINEAR is not supported");
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Multi-sample image w/ sparse residency when feature isn't available (4 flavors)
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00973");

    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00974");

    image_create_info.samples = VK_SAMPLE_COUNT_8_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00975");

    image_create_info.samples = VK_SAMPLE_COUNT_16_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00976");
}

TEST_F(VkLayerTest, InvalidMemoryMapping) {
    TEST_DESCRIPTION("Attempt to map memory in a number of incorrect ways");
    VkResult err;
    bool pass;
    ASSERT_NO_FATAL_FAILURE(Init());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    const VkDeviceSize atom_size = m_device->props.limits.nonCoherentAtomSize;

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = 256;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    err = vkCreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vkGetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    // Ensure memory is big enough for both bindings
    static const VkDeviceSize allocation_size = 0x10000;
    alloc_info.allocationSize = allocation_size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    err = vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    uint8_t *pData;
    // Attempt to map memory size 0 is invalid
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VkMapMemory: Attempting to map memory range of size zero");
    err = vkMapMemory(m_device->device(), mem, 0, 0, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Map memory twice
    err = vkMapMemory(m_device->device(), mem, 0, mem_reqs.size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-MemTrack-InvalidMap");
    err = vkMapMemory(m_device->device(), mem, 0, mem_reqs.size, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();

    // Unmap the memory to avoid re-map error
    vkUnmapMemory(m_device->device(), mem);
    // overstep allocation with VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " with size of VK_WHOLE_SIZE oversteps total array size 0x");
    err = vkMapMemory(m_device->device(), mem, allocation_size + 1, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep allocation w/o VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, " oversteps total array size 0x");
    err = vkMapMemory(m_device->device(), mem, 1, allocation_size, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Now error due to unmapping memory that's not mapped
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "Unmapping Memory without memory being mapped: ");
    vkUnmapMemory(m_device->device(), mem);
    m_errorMonitor->VerifyFound();

    // Now map memory and cause errors due to flushing invalid ranges
    err = vkMapMemory(m_device->device(), mem, 4 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    VkMappedMemoryRange mmr = {};
    mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mmr.memory = mem;
    mmr.offset = atom_size;  // Error b/c offset less than offset of mapped mem
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMappedMemoryRange-size-00685");
    vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range that oversteps mapped range
    vkUnmapMemory(m_device->device(), mem);
    err = vkMapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    mmr.offset = atom_size;
    mmr.size = 4 * atom_size;  // Flushing bounds exceed mapped bounds
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMappedMemoryRange-size-00685");
    vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range with VK_WHOLE_SIZE that oversteps offset
    vkUnmapMemory(m_device->device(), mem);
    err = vkMapMemory(m_device->device(), mem, 2 * atom_size, 4 * atom_size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    mmr.offset = atom_size;
    mmr.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMappedMemoryRange-size-00686");
    vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Some platforms have an atomsize of 1 which makes the test meaningless
    if (atom_size > 3) {
        // Now with an offset NOT a multiple of the device limit
        vkUnmapMemory(m_device->device(), mem);
        err = vkMapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData);
        ASSERT_VK_SUCCESS(err);
        mmr.offset = 3;  // Not a multiple of atom_size
        mmr.size = VK_WHOLE_SIZE;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMappedMemoryRange-offset-00687");
        vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();

        // Now with a size NOT a multiple of the device limit
        vkUnmapMemory(m_device->device(), mem);
        err = vkMapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData);
        ASSERT_VK_SUCCESS(err);
        mmr.offset = atom_size;
        mmr.size = 2 * atom_size + 1;  // Not a multiple of atom_size
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkMappedMemoryRange-size-01390");
        vkFlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();
    }

    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vkFreeMemory(m_device->device(), mem, NULL);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    // TODO : If we can get HOST_VISIBLE w/o HOST_COHERENT we can test cases of
    //  kVUID_Core_MemTrack_InvalidMap in validateAndCopyNoncoherentMemoryToDriver()

    vkDestroyBuffer(m_device->device(), buffer, NULL);
    vkFreeMemory(m_device->device(), mem, NULL);
}

TEST_F(VkLayerTest, MapMemWithoutHostVisibleBit) {
    TEST_DESCRIPTION("Allocate memory that is not mappable and then attempt to map it.");
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkMapMemory-memory-00682");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 1024;

    pass = m_device->phy().set_memory_type(0xFFFFFFFF, &mem_alloc, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {  // If we can't find any unmappable memory this test doesn't
                  // make sense
        printf("%s No unmappable memory types found, skipping test\n", kSkipPrefix);
        return;
    }

    VkDeviceMemory mem;
    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    void *mappedAddress = NULL;
    err = vkMapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, &mappedAddress);
    m_errorMonitor->VerifyFound();

    vkFreeMemory(m_device->device(), mem, NULL);
}

TEST_F(VkLayerTest, RebindMemory) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-image-01044");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create an image, allocate memory, free it, and then try to bind it
    VkImage image;
    VkDeviceMemory mem1;
    VkDeviceMemory mem2;
    VkMemoryRequirements mem_reqs;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    // Introduce failure, do NOT set memProps to
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    mem_alloc.memoryTypeIndex = 1;
    err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    vkGetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    // allocate 2 memory objects
    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem1);
    ASSERT_VK_SUCCESS(err);
    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem2);
    ASSERT_VK_SUCCESS(err);

    // Bind first memory object to Image object
    err = vkBindImageMemory(m_device->device(), image, mem1, 0);
    ASSERT_VK_SUCCESS(err);

    // Introduce validation failure, try to bind a different memory object to
    // the same image object
    err = vkBindImageMemory(m_device->device(), image, mem2, 0);

    m_errorMonitor->VerifyFound();

    vkDestroyImage(m_device->device(), image, NULL);
    vkFreeMemory(m_device->device(), mem1, NULL);
    vkFreeMemory(m_device->device(), mem2, NULL);
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

    auto mem_reqs = image.memory_requirements();
    // memory_type_index is set to 0 here, but is set properly below
    auto image_alloc_info = vk_testing::DeviceMemory::alloc_info(mem_reqs.size, 0);

    bool pass;
    // the last argument is the "forbid" argument for set_memory_type, disallowing
    // that particular memory type rather than requiring it
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &image_alloc_info, 0, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        return;
    }
    vk_testing::DeviceMemory mem;
    mem.init(*m_device, image_alloc_info);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetDeviceMemoryCommitment-memory-00690");
    VkDeviceSize size;
    vkGetDeviceMemoryCommitment(m_device->device(), mem.handle(), &size);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidUsageBits) {
    TEST_DESCRIPTION(
        "Specify wrong usage for image then create conflicting view of image Initialize buffer with wrong usage then perform copy "
        "expecting errors from both the image and the buffer (2 calls)");

    ASSERT_NO_FATAL_FAILURE(Init());
    auto format = FindSupportedDepthStencilFormat(gpu());
    if (!format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    // Initialize image with transfer source usage
    image.Init(128, 128, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView dsv;
    VkImageViewCreateInfo dsvci = {};
    dsvci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    dsvci.format = format;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    // Create a view with depth / stencil aspect for image with different usage
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-MemTrack-InvalidUsageFlag");
    vkCreateImageView(m_device->device(), &dsvci, NULL, &dsv);
    m_errorMonitor->VerifyFound();

    // Initialize buffer with TRANSFER_DST usage
    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_dst(*m_device, 128 * 128, reqs);
    VkBufferImageCopy region = {};
    region.bufferRowLength = 128;
    region.bufferImageHeight = 128;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.height = 16;
    region.imageExtent.width = 16;
    region.imageExtent.depth = 1;

    // Buffer usage not set to TRANSFER_SRC and image usage not set to TRANSFER_DST
    m_commandBuffer->begin();

    // two separate errors from this call:
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyBufferToImage-dstImage-00177");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyBufferToImage-srcBuffer-00174");

    vkCmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CopyBufferToCompressedImage) {
    TEST_DESCRIPTION("Copy buffer to compressed image when buffer is larger than image.");
    ASSERT_NO_FATAL_FAILURE(Init());

    // Verify format support
    if (!ImageFormatAndFeaturesSupported(gpu(), VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR)) {
        printf("%s Required formats/features not supported - CopyBufferToCompressedImage skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj width_image(m_device);
    VkImageObj height_image(m_device);
    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src(*m_device, 8 * 4 * 2, reqs);
    VkBufferImageCopy region = {};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = 8;
    region.imageExtent.height = 4;
    region.imageExtent.depth = 1;

    width_image.Init(5, 4, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    height_image.Init(8, 3, 1, VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    if (!width_image.initialized() || (!height_image.initialized())) {
        printf("%s Unable to initialize surfaces - UncompressedToCompressedImageCopy skipped.\n", kSkipPrefix);
        return;
    }
    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBufferImageCopy-imageOffset-00197");
    vkCmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), width_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBufferImageCopy-imageOffset-00200");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-pRegions-00172");

    VkResult err;
    VkImageCreateInfo depth_image_create_info = {};
    depth_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_image_create_info.pNext = NULL;
    depth_image_create_info.imageType = VK_IMAGE_TYPE_3D;
    depth_image_create_info.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    depth_image_create_info.extent.width = 8;
    depth_image_create_info.extent.height = 4;
    depth_image_create_info.extent.depth = 1;
    depth_image_create_info.mipLevels = 1;
    depth_image_create_info.arrayLayers = 1;
    depth_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    depth_image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    depth_image_create_info.queueFamilyIndexCount = 0;
    depth_image_create_info.pQueueFamilyIndices = NULL;

    VkImage depth_image = VK_NULL_HANDLE;
    err = vkCreateImage(m_device->handle(), &depth_image_create_info, NULL, &depth_image);
    ASSERT_VK_SUCCESS(err);

    VkDeviceMemory mem1;
    VkMemoryRequirements mem_reqs;
    mem_reqs.memoryTypeBits = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.memoryTypeIndex = 1;
    vkGetImageMemoryRequirements(m_device->device(), depth_image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem1);
    ASSERT_VK_SUCCESS(err);
    err = vkBindImageMemory(m_device->device(), depth_image, mem1, 0);

    region.imageExtent.depth = 2;
    vkCmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), depth_image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    vkDestroyImage(m_device->device(), depth_image, NULL);
    vkFreeMemory(m_device->device(), mem1, NULL);
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CreateUnknownObject) {
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageMemoryRequirements-image-parameter");

    TEST_DESCRIPTION("Pass an invalid image object handle into a Vulkan API call.");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Pass bogus handle into GetImageMemoryRequirements
    VkMemoryRequirements mem_reqs;
    uint64_t fakeImageHandle = 0xCADECADE;
    VkImage fauxImage = reinterpret_cast<VkImage &>(fakeImageHandle);

    vkGetImageMemoryRequirements(m_device->device(), fauxImage, &mem_reqs);

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

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    vkGetImageMemoryRequirements(m_device->device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;

    // Introduce Failure, select invalid TypeIndex
    VkPhysicalDeviceMemoryProperties memory_info;

    vkGetPhysicalDeviceMemoryProperties(gpu(), &memory_info);
    unsigned int i;
    for (i = 0; i < memory_info.memoryTypeCount; i++) {
        if ((mem_reqs.memoryTypeBits & (1 << i)) == 0) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }
    if (i >= memory_info.memoryTypeCount) {
        printf("%s No invalid memory type index could be found; skipped.\n", kSkipPrefix);
        vkDestroyImage(m_device->device(), image, NULL);
        return;
    }

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "for this object type are not compatible with the memory");

    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vkBindImageMemory(m_device->device(), image, mem, 0);
    (void)err;

    m_errorMonitor->VerifyFound();

    vkDestroyImage(m_device->device(), image, NULL);
    vkFreeMemory(m_device->device(), mem, NULL);
}

TEST_F(VkLayerTest, BindInvalidMemory) {
    VkResult err;
    bool pass;

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    const int32_t tex_width = 256;
    const int32_t tex_height = 256;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = NULL;
    buffer_create_info.flags = 0;
    buffer_create_info.size = 4 * 1024 * 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Create an image/buffer, allocate memory, free it, and then try to bind it
    {
        VkImage image = VK_NULL_HANDLE;
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vkCreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        err = vkCreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vkGetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vkGetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);

        VkMemoryAllocateInfo image_mem_alloc = {}, buffer_mem_alloc = {};
        image_mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_mem_alloc.allocationSize = image_mem_reqs.size;
        pass = m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_mem_alloc, 0);
        ASSERT_TRUE(pass);
        buffer_mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        buffer_mem_alloc.allocationSize = buffer_mem_reqs.size;
        pass = m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_mem_alloc, 0);
        ASSERT_TRUE(pass);

        VkDeviceMemory image_mem = VK_NULL_HANDLE, buffer_mem = VK_NULL_HANDLE;
        err = vkAllocateMemory(device(), &image_mem_alloc, NULL, &image_mem);
        ASSERT_VK_SUCCESS(err);
        err = vkAllocateMemory(device(), &buffer_mem_alloc, NULL, &buffer_mem);
        ASSERT_VK_SUCCESS(err);

        vkFreeMemory(device(), image_mem, NULL);
        vkFreeMemory(device(), buffer_mem, NULL);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-memory-parameter");
        err = vkBindImageMemory(device(), image, image_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-memory-parameter");
        err = vkBindBufferMemory(device(), buffer, buffer_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        vkDestroyImage(m_device->device(), image, NULL);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
    }

    // Try to bind memory to an object that already has a memory binding
    {
        VkImage image = VK_NULL_HANDLE;
        err = vkCreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vkCreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vkGetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vkGetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = {}, buffer_alloc_info = {};
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_alloc_info.allocationSize = image_mem_reqs.size;
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
        pass = m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        ASSERT_TRUE(pass);
        pass = m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
        ASSERT_TRUE(pass);
        VkDeviceMemory image_mem, buffer_mem;
        err = vkAllocateMemory(device(), &image_alloc_info, NULL, &image_mem);
        ASSERT_VK_SUCCESS(err);
        err = vkAllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
        ASSERT_VK_SUCCESS(err);

        err = vkBindImageMemory(device(), image, image_mem, 0);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-image-01044");
        err = vkBindImageMemory(device(), image, image_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        err = vkBindBufferMemory(device(), buffer, buffer_mem, 0);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-buffer-01029");
        err = vkBindBufferMemory(device(), buffer, buffer_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        vkFreeMemory(device(), image_mem, NULL);
        vkFreeMemory(device(), buffer_mem, NULL);
        vkDestroyImage(device(), image, NULL);
        vkDestroyBuffer(device(), buffer, NULL);
    }

    // Try to bind memory to an object with an invalid memoryOffset
    {
        VkImage image = VK_NULL_HANDLE;
        err = vkCreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vkCreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vkGetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vkGetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = {}, buffer_alloc_info = {};
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        // Leave some extra space for alignment wiggle room
        image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size + buffer_mem_reqs.alignment;
        pass = m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0);
        ASSERT_TRUE(pass);
        pass = m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
        ASSERT_TRUE(pass);
        VkDeviceMemory image_mem, buffer_mem;
        err = vkAllocateMemory(device(), &image_alloc_info, NULL, &image_mem);
        ASSERT_VK_SUCCESS(err);
        err = vkAllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
        ASSERT_VK_SUCCESS(err);

        // Test unaligned memory offset
        {
            if (image_mem_reqs.alignment > 1) {
                VkDeviceSize image_offset = 1;
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-memoryOffset-01048");
                err = vkBindImageMemory(device(), image, image_mem, image_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }

            if (buffer_mem_reqs.alignment > 1) {
                VkDeviceSize buffer_offset = 1;
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-memoryOffset-01036");
                err = vkBindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }
        }

        // Test memory offsets outside the memory allocation
        {
            VkDeviceSize image_offset =
                (image_alloc_info.allocationSize + image_mem_reqs.alignment) & ~(image_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-memoryOffset-01046");
            err = vkBindImageMemory(device(), image, image_mem, image_offset);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();

            VkDeviceSize buffer_offset =
                (buffer_alloc_info.allocationSize + buffer_mem_reqs.alignment) & ~(buffer_mem_reqs.alignment - 1);
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-memoryOffset-01031");
            err = vkBindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();
        }

        // Test memory offsets within the memory allocation, but which leave too little memory for
        // the resource.
        {
            VkDeviceSize image_offset = (image_mem_reqs.size - 1) & ~(image_mem_reqs.alignment - 1);
            if ((image_offset > 0) && (image_mem_reqs.size < (image_alloc_info.allocationSize - image_mem_reqs.alignment))) {
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-size-01049");
                err = vkBindImageMemory(device(), image, image_mem, image_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }

            VkDeviceSize buffer_offset = (buffer_mem_reqs.size - 1) & ~(buffer_mem_reqs.alignment - 1);
            if (buffer_offset > 0) {
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-size-01037");
                err = vkBindBufferMemory(device(), buffer, buffer_mem, buffer_offset);
                (void)err;  // This may very well return an error.
                m_errorMonitor->VerifyFound();
            }
        }

        vkFreeMemory(device(), image_mem, NULL);
        vkFreeMemory(device(), buffer_mem, NULL);
        vkDestroyImage(device(), image, NULL);
        vkDestroyBuffer(device(), buffer, NULL);
    }

    // Try to bind memory to an object with an invalid memory type
    {
        VkImage image = VK_NULL_HANDLE;
        err = vkCreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        VkBuffer buffer = VK_NULL_HANDLE;
        err = vkCreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vkGetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vkGetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
        VkMemoryAllocateInfo image_alloc_info = {}, buffer_alloc_info = {};
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_alloc_info.allocationSize = image_mem_reqs.size;
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
        // Create a mask of available memory types *not* supported by these resources,
        // and try to use one of them.
        VkPhysicalDeviceMemoryProperties memory_properties = {};
        vkGetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);
        VkDeviceMemory image_mem, buffer_mem;

        uint32_t image_unsupported_mem_type_bits = ((1 << memory_properties.memoryTypeCount) - 1) & ~image_mem_reqs.memoryTypeBits;
        if (image_unsupported_mem_type_bits != 0) {
            pass = m_device->phy().set_memory_type(image_unsupported_mem_type_bits, &image_alloc_info, 0);
            ASSERT_TRUE(pass);
            err = vkAllocateMemory(device(), &image_alloc_info, NULL, &image_mem);
            ASSERT_VK_SUCCESS(err);
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-memory-01047");
            err = vkBindImageMemory(device(), image, image_mem, 0);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();
            vkFreeMemory(device(), image_mem, NULL);
        }

        uint32_t buffer_unsupported_mem_type_bits =
            ((1 << memory_properties.memoryTypeCount) - 1) & ~buffer_mem_reqs.memoryTypeBits;
        if (buffer_unsupported_mem_type_bits != 0) {
            pass = m_device->phy().set_memory_type(buffer_unsupported_mem_type_bits, &buffer_alloc_info, 0);
            ASSERT_TRUE(pass);
            err = vkAllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
            ASSERT_VK_SUCCESS(err);
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-memory-01035");
            err = vkBindBufferMemory(device(), buffer, buffer_mem, 0);
            (void)err;  // This may very well return an error.
            m_errorMonitor->VerifyFound();
            vkFreeMemory(device(), buffer_mem, NULL);
        }

        vkDestroyImage(device(), image, NULL);
        vkDestroyBuffer(device(), buffer, NULL);
    }

    // Try to bind memory to an image created with sparse memory flags
    {
        VkImageCreateInfo sparse_image_create_info = image_create_info;
        sparse_image_create_info.flags |= VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        VkImageFormatProperties image_format_properties = {};
        err = vkGetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), sparse_image_create_info.format,
                                                       sparse_image_create_info.imageType, sparse_image_create_info.tiling,
                                                       sparse_image_create_info.usage, sparse_image_create_info.flags,
                                                       &image_format_properties);
        if (!m_device->phy().features().sparseResidencyImage2D || err == VK_ERROR_FORMAT_NOT_SUPPORTED) {
            // most likely means sparse formats aren't supported here; skip this test.
        } else {
            ASSERT_VK_SUCCESS(err);
            if (image_format_properties.maxExtent.width == 0) {
                printf("%s Sparse image format not supported; skipped.\n", kSkipPrefix);
                return;
            } else {
                VkImage sparse_image = VK_NULL_HANDLE;
                err = vkCreateImage(m_device->device(), &sparse_image_create_info, NULL, &sparse_image);
                ASSERT_VK_SUCCESS(err);
                VkMemoryRequirements sparse_mem_reqs = {};
                vkGetImageMemoryRequirements(m_device->device(), sparse_image, &sparse_mem_reqs);
                if (sparse_mem_reqs.memoryTypeBits != 0) {
                    VkMemoryAllocateInfo sparse_mem_alloc = {};
                    sparse_mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    sparse_mem_alloc.pNext = NULL;
                    sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                    sparse_mem_alloc.memoryTypeIndex = 0;
                    pass = m_device->phy().set_memory_type(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                    ASSERT_TRUE(pass);
                    VkDeviceMemory sparse_mem = VK_NULL_HANDLE;
                    err = vkAllocateMemory(m_device->device(), &sparse_mem_alloc, NULL, &sparse_mem);
                    ASSERT_VK_SUCCESS(err);
                    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-image-01045");
                    err = vkBindImageMemory(m_device->device(), sparse_image, sparse_mem, 0);
                    // This may very well return an error.
                    (void)err;
                    m_errorMonitor->VerifyFound();
                    vkFreeMemory(m_device->device(), sparse_mem, NULL);
                }
                vkDestroyImage(m_device->device(), sparse_image, NULL);
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
            err = vkCreateBuffer(m_device->device(), &sparse_buffer_create_info, NULL, &sparse_buffer);
            ASSERT_VK_SUCCESS(err);
            VkMemoryRequirements sparse_mem_reqs = {};
            vkGetBufferMemoryRequirements(m_device->device(), sparse_buffer, &sparse_mem_reqs);
            if (sparse_mem_reqs.memoryTypeBits != 0) {
                VkMemoryAllocateInfo sparse_mem_alloc = {};
                sparse_mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                sparse_mem_alloc.pNext = NULL;
                sparse_mem_alloc.allocationSize = sparse_mem_reqs.size;
                sparse_mem_alloc.memoryTypeIndex = 0;
                pass = m_device->phy().set_memory_type(sparse_mem_reqs.memoryTypeBits, &sparse_mem_alloc, 0);
                ASSERT_TRUE(pass);
                VkDeviceMemory sparse_mem = VK_NULL_HANDLE;
                err = vkAllocateMemory(m_device->device(), &sparse_mem_alloc, NULL, &sparse_mem);
                ASSERT_VK_SUCCESS(err);
                m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-buffer-01030");
                err = vkBindBufferMemory(m_device->device(), sparse_buffer, sparse_mem, 0);
                // This may very well return an error.
                (void)err;
                m_errorMonitor->VerifyFound();
                vkFreeMemory(m_device->device(), sparse_mem, NULL);
            }
            vkDestroyBuffer(m_device->device(), sparse_buffer, NULL);
        }
    }
}

TEST_F(VkLayerTest, BindMemoryToDestroyedObject) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-image-parameter");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create an image object, allocate memory, destroy the object and then try
    // to bind it
    VkImage image;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;

    err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    vkGetImageMemoryRequirements(m_device->device(), image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);

    // Allocate memory
    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    // Introduce validation failure, destroy Image object before binding
    vkDestroyImage(m_device->device(), image, NULL);
    ASSERT_VK_SUCCESS(err);

    // Now Try to bind memory to this destroyed object
    err = vkBindImageMemory(m_device->device(), image, mem, 0);
    // This may very well return an error.
    (void)err;

    m_errorMonitor->VerifyFound();

    vkFreeMemory(m_device->device(), mem, NULL);
}

TEST_F(VkLayerTest, ExceedMemoryAllocationCount) {
    VkResult err = VK_SUCCESS;
    const int max_mems = 32;
    VkDeviceMemory mems[max_mems + 1];

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vkGetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vkGetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

    if (!(fpvkSetPhysicalDeviceLimitsEXT) || !(fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    if (props.limits.maxMemoryAllocationCount > max_mems) {
        props.limits.maxMemoryAllocationCount = max_mems;
        fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "Number of currently valid memory objects is not less than the maximum allowed");

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = 4;

    int i;
    for (i = 0; i <= max_mems; i++) {
        err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &mems[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }
    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vkFreeMemory(m_device->device(), mems[j], NULL);
    }
}

TEST_F(VkLayerTest, ImageSampleCounts) {
    TEST_DESCRIPTION("Use bad sample counts in image transfer calls to trigger validation errors.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkMemoryPropertyFlags reqs = 0;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.flags = 0;

    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {256, 256, 1};
    blit_region.dstOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[1] = {128, 128, 1};

    // Create two images, the source with sampleCount = 4, and attempt to blit
    // between them
    {
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkImageObj src_image(m_device);
        src_image.init(&image_create_info);
        src_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageObj dst_image(m_device);
        dst_image.init(&image_create_info);
        dst_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_commandBuffer->begin();
        // TODO: These 2 VUs are redundant - expect one of them to go away
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00233");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00228");
        vkCmdBlitImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }

    // Create two images, the dest with sampleCount = 4, and attempt to blit
    // between them
    {
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        VkImageObj src_image(m_device);
        src_image.init(&image_create_info);
        src_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageObj dst_image(m_device);
        dst_image.init(&image_create_info);
        dst_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_commandBuffer->begin();
        // TODO: These 2 VUs are redundant - expect one of them to go away
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-00234");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00228");
        vkCmdBlitImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }

    VkBufferImageCopy copy_region = {};
    copy_region.bufferRowLength = 128;
    copy_region.bufferImageHeight = 128;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent.height = 64;
    copy_region.imageExtent.width = 64;
    copy_region.imageExtent.depth = 1;

    // Create src buffer and dst image with sampleCount = 4 and attempt to copy
    // buffer to image
    {
        VkBufferObj src_buffer;
        src_buffer.init_as_src(*m_device, 128 * 128 * 4, reqs);
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageObj dst_image(m_device);
        dst_image.init(&image_create_info);
        dst_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(
            VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "was created with a sample count of VK_SAMPLE_COUNT_4_BIT but must be VK_SAMPLE_COUNT_1_BIT");
        vkCmdCopyBufferToImage(m_commandBuffer->handle(), src_buffer.handle(), dst_image.handle(),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }

    // Create dst buffer and src image with sampleCount = 4 and attempt to copy
    // image to buffer
    {
        VkBufferObj dst_buffer;
        dst_buffer.init_as_dst(*m_device, 128 * 128 * 4, reqs);
        image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        vk_testing::Image src_image;
        src_image.init(*m_device, (const VkImageCreateInfo &)image_create_info, reqs);
        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(
            VK_DEBUG_REPORT_ERROR_BIT_EXT,
            "was created with a sample count of VK_SAMPLE_COUNT_4_BIT but must be VK_SAMPLE_COUNT_1_BIT");
        vkCmdCopyImageToBuffer(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               dst_buffer.handle(), 1, &copy_region);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }
}

TEST_F(VkLayerTest, BlitImageFormatTypes) {
    ASSERT_NO_FATAL_FAILURE(Init());

    VkFormat f_unsigned = VK_FORMAT_R8G8B8A8_UINT;
    VkFormat f_signed = VK_FORMAT_R8G8B8A8_SINT;
    VkFormat f_float = VK_FORMAT_R32_SFLOAT;
    VkFormat f_depth = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkFormat f_depth2 = VK_FORMAT_D32_SFLOAT;

    if (!ImageFormatIsSupported(gpu(), f_unsigned, VK_IMAGE_TILING_OPTIMAL) ||
        !ImageFormatIsSupported(gpu(), f_signed, VK_IMAGE_TILING_OPTIMAL) ||
        !ImageFormatIsSupported(gpu(), f_float, VK_IMAGE_TILING_OPTIMAL) ||
        !ImageFormatIsSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL) ||
        !ImageFormatIsSupported(gpu(), f_depth2, VK_IMAGE_TILING_OPTIMAL)) {
        printf("%s Requested formats not supported - BlitImageFormatTypes skipped.\n", kSkipPrefix);
        return;
    }

    // Note any missing feature bits
    bool usrc = !ImageFormatAndFeaturesSupported(gpu(), f_unsigned, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
    bool udst = !ImageFormatAndFeaturesSupported(gpu(), f_unsigned, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool ssrc = !ImageFormatAndFeaturesSupported(gpu(), f_signed, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
    bool sdst = !ImageFormatAndFeaturesSupported(gpu(), f_signed, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool fsrc = !ImageFormatAndFeaturesSupported(gpu(), f_float, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
    bool fdst = !ImageFormatAndFeaturesSupported(gpu(), f_float, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool d1dst = !ImageFormatAndFeaturesSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);
    bool d2src = !ImageFormatAndFeaturesSupported(gpu(), f_depth2, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);

    VkImageObj unsigned_image(m_device);
    unsigned_image.Init(64, 64, 1, f_unsigned, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                        VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(unsigned_image.initialized());
    unsigned_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj signed_image(m_device);
    signed_image.Init(64, 64, 1, f_signed, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(signed_image.initialized());
    signed_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj float_image(m_device);
    float_image.Init(64, 64, 1, f_float, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL,
                     0);
    ASSERT_TRUE(float_image.initialized());
    float_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj depth_image(m_device);
    depth_image.Init(64, 64, 1, f_depth, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL,
                     0);
    ASSERT_TRUE(depth_image.initialized());
    depth_image.SetLayout(VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageObj depth_image2(m_device);
    depth_image2.Init(64, 64, 1, f_depth2, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                      VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(depth_image2.initialized());
    depth_image2.SetLayout(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {64, 64, 1};
    blitRegion.dstOffsets[0] = {0, 0, 0};
    blitRegion.dstOffsets[1] = {32, 32, 1};

    m_commandBuffer->begin();

    // Unsigned int vs not an int
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00230");
    if (usrc) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-01999");
    if (fdst) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), unsigned_image.image(), unsigned_image.Layout(), float_image.image(),
                   float_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00230");
    if (fsrc) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-01999");
    if (udst) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), float_image.image(), float_image.Layout(), unsigned_image.image(),
                   unsigned_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Signed int vs not an int,
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00229");
    if (ssrc) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-01999");
    if (fdst) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), signed_image.image(), signed_image.Layout(), float_image.image(),
                   float_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00229");
    if (fsrc) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-01999");
    if (sdst) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), float_image.image(), float_image.Layout(), signed_image.image(),
                   signed_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Signed vs Unsigned int - generates both VUs
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00229");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00230");
    if (ssrc) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-01999");
    if (udst) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), signed_image.image(), signed_image.Layout(), unsigned_image.image(),
                   unsigned_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00229");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00230");
    if (usrc) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-01999");
    if (sdst) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), unsigned_image.image(), unsigned_image.Layout(), signed_image.image(),
                   signed_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Depth vs any non-identical depth format
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00231");
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (d2src) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-01999");
    if (d1dst) m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), depth_image2.image(), depth_image2.Layout(), depth_image.image(),
                   depth_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BlitImageFilters) {
    bool cubic_support = false;
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, "VK_IMG_filter_cubic")) {
        m_device_extension_names.push_back("VK_IMG_filter_cubic");
        cubic_support = true;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkFormat fmt = VK_FORMAT_R8_UINT;
    if (!ImageFormatIsSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL)) {
        printf("%s No R8_UINT format support - BlitImageFilters skipped.\n", kSkipPrefix);
        return;
    }

    // Create 2D images
    VkImageObj src2D(m_device);
    VkImageObj dst2D(m_device);
    src2D.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    dst2D.Init(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(src2D.initialized());
    ASSERT_TRUE(dst2D.initialized());
    src2D.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    dst2D.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    // Create 3D image
    VkImageCreateInfo ci;
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = fmt;
    ci.extent = {64, 64, 4};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj src3D(m_device);
    src3D.init(&ci);
    ASSERT_TRUE(src3D.initialized());

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {48, 48, 1};
    blitRegion.dstOffsets[0] = {0, 0, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    // UINT format should not support linear filtering, but check to be sure
    if (!ImageFormatAndFeaturesSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-filter-02001");
        vkCmdBlitImage(m_commandBuffer->handle(), src2D.image(), src2D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                       VK_FILTER_LINEAR);
        m_errorMonitor->VerifyFound();
    }

    if (cubic_support && !ImageFormatAndFeaturesSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL,
                                                          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG)) {
        // Invalid filter CUBIC_IMG
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-filter-02002");
        vkCmdBlitImage(m_commandBuffer->handle(), src3D.image(), src3D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                       VK_FILTER_CUBIC_IMG);
        m_errorMonitor->VerifyFound();

        // Invalid filter CUBIC_IMG + invalid 2D source image
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-filter-02002");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-filter-00237");
        vkCmdBlitImage(m_commandBuffer->handle(), src2D.image(), src2D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                       VK_FILTER_CUBIC_IMG);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BlitImageLayout) {
    TEST_DESCRIPTION("Incorrect vkCmdBlitImage layouts");

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkResult err;
    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    // Create images
    VkImageObj img_src_transfer(m_device);
    VkImageObj img_dst_transfer(m_device);
    VkImageObj img_general(m_device);
    VkImageObj img_color(m_device);

    img_src_transfer.InitNoLayout(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                  VK_IMAGE_TILING_OPTIMAL, 0);
    img_dst_transfer.InitNoLayout(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                  VK_IMAGE_TILING_OPTIMAL, 0);
    img_general.InitNoLayout(64, 64, 1, fmt, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                             VK_IMAGE_TILING_OPTIMAL, 0);
    img_color.InitNoLayout(64, 64, 1, fmt,
                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                           VK_IMAGE_TILING_OPTIMAL, 0);

    ASSERT_TRUE(img_src_transfer.initialized());
    ASSERT_TRUE(img_dst_transfer.initialized());
    ASSERT_TRUE(img_general.initialized());
    ASSERT_TRUE(img_color.initialized());

    img_src_transfer.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    img_dst_transfer.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    img_general.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    img_color.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {48, 48, 1};
    blit_region.dstOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    // Illegal srcImageLayout
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImageLayout-00222");
    vkCmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                   img_dst_transfer.image(), img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();

    // Illegal destImageLayout
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImageLayout-00227");
    vkCmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_dst_transfer.image(),
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    // Source image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vkCmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_color.image(),
                   VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    // Destination image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vkCmdBlitImage(m_commandBuffer->handle(), img_color.image(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.image(),
                   img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    // Source image in invalid layout in the middle of CB
    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.pNext = nullptr;
    img_barrier.srcAccessMask = 0;
    img_barrier.dstAccessMask = 0;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.image = img_general.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImageLayout-00221");
    vkCmdBlitImage(m_commandBuffer->handle(), img_general.image(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.image(),
                   img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    // Destination image in invalid layout in the middle of CB
    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.image = img_dst_transfer.handle();

    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImageLayout-00226");
    vkCmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_dst_transfer.image(),
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vkQueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);
}

TEST_F(VkLayerTest, BlitImageOffsets) {
    ASSERT_NO_FATAL_FAILURE(Init());

    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    if (!ImageFormatAndFeaturesSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        printf("%s No blit feature bits - BlitImageOffsets skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci;
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_1D;
    ci.format = fmt;
    ci.extent = {64, 1, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image_1D(m_device);
    image_1D.init(&ci);
    ASSERT_TRUE(image_1D.initialized());

    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.extent = {64, 64, 1};
    VkImageObj image_2D(m_device);
    image_2D.init(&ci);
    ASSERT_TRUE(image_2D.initialized());

    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {64, 64, 64};
    VkImageObj image_3D(m_device);
    image_3D.init(&ci);
    ASSERT_TRUE(image_3D.initialized());

    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstSubresource.mipLevel = 0;

    m_commandBuffer->begin();

    // 1D, with src/dest y offsets other than (0,1)
    blit_region.srcOffsets[0] = {0, 1, 0};
    blit_region.srcOffsets[1] = {30, 1, 1};
    blit_region.dstOffsets[0] = {32, 0, 0};
    blit_region.dstOffsets[1] = {64, 1, 1};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-srcImage-00245");
    vkCmdBlitImage(m_commandBuffer->handle(), image_1D.image(), image_1D.Layout(), image_1D.image(), image_1D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[0] = {32, 1, 0};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-dstImage-00250");
    vkCmdBlitImage(m_commandBuffer->handle(), image_1D.image(), image_1D.Layout(), image_1D.image(), image_1D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // 2D, with src/dest z offsets other than (0,1)
    blit_region.srcOffsets[0] = {0, 0, 1};
    blit_region.srcOffsets[1] = {24, 31, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-srcImage-00247");
    vkCmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[0] = {32, 32, 1};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-dstImage-00252");
    vkCmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Source offsets exceeding source image dimensions
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {65, 64, 1};  // src x
    blit_region.dstOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-srcOffset-00243");    // x
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-pRegions-00215");  // src region
    vkCmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[1] = {64, 65, 1};                                                                    // src y
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-srcOffset-00244");    // y
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-pRegions-00215");  // src region
    vkCmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 65};  // src z
    blit_region.srcOffsets[1] = {64, 64, 64};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-srcOffset-00246");    // z
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-pRegions-00215");  // src region
    vkCmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Dest offsets exceeding source image dimensions
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {64, 64, 1};
    blit_region.dstOffsets[0] = {96, 64, 32};  // dst x
    blit_region.dstOffsets[1] = {64, 0, 33};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-dstOffset-00248");    // x
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-pRegions-00216");  // dst region
    vkCmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.dstOffsets[0] = {0, 65, 32};                                                                    // dst y
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-dstOffset-00249");    // y
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-pRegions-00216");  // dst region
    vkCmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.dstOffsets[0] = {0, 64, 65};  // dst z
    blit_region.dstOffsets[1] = {64, 0, 64};
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-dstOffset-00251");    // z
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-pRegions-00216");  // dst region
    vkCmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                   &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, MiscBlitImageTests) {
    ASSERT_NO_FATAL_FAILURE(Init());

    VkFormat f_color = VK_FORMAT_R32_SFLOAT;  // Need features ..BLIT_SRC_BIT & ..BLIT_DST_BIT

    if (!ImageFormatAndFeaturesSupported(gpu(), f_color, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        printf("%s Requested format features unavailable - MiscBlitImageTests skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci;
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = f_color;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // 2D color image
    VkImageObj color_img(m_device);
    color_img.init(&ci);
    ASSERT_TRUE(color_img.initialized());

    // 2D multi-sample image
    ci.samples = VK_SAMPLE_COUNT_4_BIT;
    VkImageObj ms_img(m_device);
    ms_img.init(&ci);
    ASSERT_TRUE(ms_img.initialized());

    // 3D color image
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.extent = {64, 64, 8};
    VkImageObj color_3D_img(m_device);
    color_3D_img.init(&ci);
    ASSERT_TRUE(color_3D_img.initialized());

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {16, 16, 1};
    blitRegion.dstOffsets[0] = {32, 32, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    // Blit with aspectMask errors
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-aspectMask-00241");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-aspectMask-00242");
    vkCmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                   &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid src mip level
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.mipLevel = ci.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdBlitImage-srcSubresource-01705");  // invalid srcSubresource.mipLevel
    // Redundant unavoidable errors
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageBlit-srcOffset-00243");  // out-of-bounds srcOffset.x
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageBlit-srcOffset-00244");  // out-of-bounds srcOffset.y
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageBlit-srcOffset-00246");  // out-of-bounds srcOffset.z
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdBlitImage-pRegions-00215");  // region not contained within src image
    vkCmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                   &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid dst mip level
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.mipLevel = ci.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdBlitImage-dstSubresource-01706");  // invalid dstSubresource.mipLevel
    // Redundant unavoidable errors
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageBlit-dstOffset-00248");  // out-of-bounds dstOffset.x
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageBlit-dstOffset-00249");  // out-of-bounds dstOffset.y
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageBlit-dstOffset-00251");  // out-of-bounds dstOffset.z
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdBlitImage-pRegions-00216");  // region not contained within dst image
    vkCmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                   &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid src array layer
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcSubresource.baseArrayLayer = ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdBlitImage-srcSubresource-01707");  // invalid srcSubresource layer range
    vkCmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                   &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid dst array layer
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.baseArrayLayer = ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdBlitImage-dstSubresource-01708");  // invalid dstSubresource layer range
                                                                                       // Redundant unavoidable errors
    vkCmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                   &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blitRegion.dstSubresource.baseArrayLayer = 0;

    // Blit multi-sample image
    // TODO: redundant VUs, one (1c8) or two (1d2 & 1d4) should be eliminated.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00228");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-srcImage-00233");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-00234");
    vkCmdBlitImage(m_commandBuffer->handle(), ms_img.image(), ms_img.Layout(), ms_img.image(), ms_img.Layout(), 1, &blitRegion,
                   VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit 3D with baseArrayLayer != 0 or layerCount != 1
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-srcImage-00240");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdBlitImage-srcSubresource-01707");  // base+count > total layer count
    vkCmdBlitImage(m_commandBuffer->handle(), color_3D_img.image(), color_3D_img.Layout(), color_3D_img.image(),
                   color_3D_img.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageBlit-srcImage-00240");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageSubresourceLayers-layerCount-01700");  // layer count == 0 (src)
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkImageBlit-layerCount-00239");  // src/dst layer count mismatch
    vkCmdBlitImage(m_commandBuffer->handle(), color_3D_img.image(), color_3D_img.Layout(), color_3D_img.image(),
                   color_3D_img.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BlitToDepthImageTests) {
    ASSERT_NO_FATAL_FAILURE(Init());

    // Need feature ..BLIT_SRC_BIT but not ..BLIT_DST_BIT
    // TODO: provide more choices here; supporting D32_SFLOAT as BLIT_DST isn't unheard of.
    VkFormat f_depth = VK_FORMAT_D32_SFLOAT;

    if (!ImageFormatAndFeaturesSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT) ||
        ImageFormatAndFeaturesSupported(gpu(), f_depth, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        printf("%s Requested format features unavailable - BlitToDepthImageTests skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci;
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = f_depth;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = NULL;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // 2D depth image
    VkImageObj depth_img(m_device);
    depth_img.init(&ci);
    ASSERT_TRUE(depth_img.initialized());

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {16, 16, 1};
    blitRegion.dstOffsets[0] = {32, 32, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_commandBuffer->begin();

    // Blit depth image - has SRC_BIT but not DST_BIT
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBlitImage-dstImage-02000");
    vkCmdBlitImage(m_commandBuffer->handle(), depth_img.image(), depth_img.Layout(), depth_img.image(), depth_img.Layout(), 1,
                   &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, MinImageTransferGranularity) {
    TEST_DESCRIPTION("Tests for validation of Queue Family property minImageTransferGranularity.");
    ASSERT_NO_FATAL_FAILURE(Init());

    auto queue_family_properties = m_device->phy().queue_properties();
    auto large_granularity_family =
        std::find_if(queue_family_properties.begin(), queue_family_properties.end(), [](VkQueueFamilyProperties family_properties) {
            VkExtent3D family_granularity = family_properties.minImageTransferGranularity;
            // We need a queue family that supports copy operations and has a large enough minImageTransferGranularity for the tests
            // below to make sense.
            return (family_properties.queueFlags & VK_QUEUE_TRANSFER_BIT || family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT ||
                    family_properties.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
                   family_granularity.depth >= 4 && family_granularity.width >= 4 && family_granularity.height >= 4;
        });

    if (large_granularity_family == queue_family_properties.end()) {
        printf("%s No queue family has a large enough granularity for this test to be meaningful, skipping test\n", kSkipPrefix);
        return;
    }
    const size_t queue_family_index = std::distance(queue_family_properties.begin(), large_granularity_family);
    VkExtent3D granularity = queue_family_properties[queue_family_index].minImageTransferGranularity;
    VkCommandPoolObj command_pool(m_device, queue_family_index, 0);

    // Create two images of different types and try to copy between them
    VkImage srcImage;
    VkImage dstImage;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = granularity.width * 2;
    image_create_info.extent.height = granularity.height * 2;
    image_create_info.extent.depth = granularity.depth * 2;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;

    VkImageObj src_image_obj(m_device);
    src_image_obj.init(&image_create_info);
    ASSERT_TRUE(src_image_obj.initialized());
    srcImage = src_image_obj.handle();

    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageObj dst_image_obj(m_device);
    dst_image_obj.init(&image_create_info);
    ASSERT_TRUE(dst_image_obj.initialized());
    dstImage = dst_image_obj.handle();

    VkCommandBufferObj command_buffer(m_device, &command_pool);
    ASSERT_TRUE(command_buffer.initialized());
    command_buffer.begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset.x = 0;
    copyRegion.srcOffset.y = 0;
    copyRegion.srcOffset.z = 0;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset.x = 0;
    copyRegion.dstOffset.y = 0;
    copyRegion.dstOffset.z = 0;
    copyRegion.extent.width = granularity.width;
    copyRegion.extent.height = granularity.height;
    copyRegion.extent.depth = granularity.depth;

    // Introduce failure by setting srcOffset to a bad granularity value
    copyRegion.srcOffset.y = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    command_buffer.CopyImage(srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Introduce failure by setting extent to a granularity value that is bad
    // for both the source and destination image.
    copyRegion.srcOffset.y = 0;
    copyRegion.extent.width = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdCopyImage-dstOffset-01784");  // dst extent image transfer granularity
    command_buffer.CopyImage(srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Now do some buffer/image copies
    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src_and_dst(*m_device, 8 * granularity.height * granularity.width * granularity.depth, reqs);
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.height = granularity.height;
    region.imageExtent.width = granularity.width;
    region.imageExtent.depth = granularity.depth;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;

    // Introduce failure by setting imageExtent to a bad granularity value
    region.imageExtent.width = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
    vkCmdCopyImageToBuffer(command_buffer.handle(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    region.imageExtent.width = granularity.width;

    // Introduce failure by setting imageOffset to a bad granularity value
    region.imageOffset.z = 3;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-01793");  // image transfer granularity
    vkCmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_errorMonitor->VerifyFound();

    command_buffer.end();
}

TEST_F(VkLayerTest, ImageBarrierSubpassConflicts) {
    TEST_DESCRIPTION("Add a pipeline barrier within a subpass that has conflicting state");
    ASSERT_NO_FATAL_FAILURE(Init());

    // A renderpass with a single subpass that declared a self-dependency
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               0,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 1, &dep};
    VkRenderPass rp;
    VkRenderPass rp_noselfdep;

    VkResult err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);
    rpci.dependencyCount = 0;
    rpci.pDependencies = nullptr;
    err = vkCreateRenderPass(m_device->device(), &rpci, nullptr, &rp_noselfdep);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    err = vkCreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_commandBuffer->begin();
    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp_noselfdep,
                                  fb,
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};

    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    VkMemoryBarrier mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    mem_barrier.pNext = NULL;
    mem_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1,
                         &mem_barrier, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    vkCmdEndRenderPass(m_commandBuffer->handle());

    rpbi.renderPass = rp;
    vkCmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    // Mis-match src stage mask
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();
    // Now mis-match dst stage mask
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();
    // Set srcQueueFamilyIndex to something other than IGNORED
    img_barrier.srcQueueFamilyIndex = 0;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    m_errorMonitor->VerifyFound();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // Mis-match mem barrier src access mask
    mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr, 0,
                         nullptr);
    m_errorMonitor->VerifyFound();
    // Mis-match mem barrier dst access mask. Also set srcAccessMask to 0 which should not cause an error
    mem_barrier.srcAccessMask = 0;
    mem_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr, 0,
                         nullptr);
    m_errorMonitor->VerifyFound();
    // Mis-match image barrier src access mask
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    m_errorMonitor->VerifyFound();
    // Mis-match image barrier dst access mask
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    m_errorMonitor->VerifyFound();
    // Mis-match dependencyFlags
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0 /* wrong */, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();
    // Send non-zero bufferMemoryBarrierCount
    // Construct a valid BufferMemoryBarrier to avoid any parameter errors
    // First we need a valid buffer to reference
    VkBufferObj buffer;
    VkMemoryPropertyFlags mem_reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    buffer.init_as_src_and_dst(*m_device, 256, mem_reqs);
    VkBufferMemoryBarrier bmb = {};
    bmb.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bmb.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.buffer = buffer.handle();
    bmb.offset = 0;
    bmb.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bmb, 0,
                         nullptr);
    m_errorMonitor->VerifyFound();
    // Add image barrier w/ image handle that's not in framebuffer
    VkImageObj lone_image(m_device);
    lone_image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    img_barrier.image = lone_image.handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-image-02635");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    m_errorMonitor->VerifyFound();
    // Have image barrier with mis-matched layouts
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-oldLayout-02636");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                         &img_barrier);
    m_errorMonitor->VerifyFound();
    vkCmdEndRenderPass(m_commandBuffer->handle());

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyRenderPass(m_device->device(), rp, nullptr);
    vkDestroyRenderPass(m_device->device(), rp_noselfdep, nullptr);
}

TEST_F(VkLayerTest, InvalidCmdBufferBufferDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to a buffer dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vkCreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vkGetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    err = vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vkBindBufferMemory(m_device->device(), buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    vkCmdFillBuffer(m_commandBuffer->handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");
    // Destroy buffer dependency prior to submit to cause ERROR
    vkDestroyBuffer(m_device->device(), buffer, NULL);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
    vkQueueWaitIdle(m_device->m_queue);
    vkFreeMemory(m_device->handle(), mem, NULL);
}

TEST_F(VkLayerTest, InvalidCmdBufferBufferViewDestroyed) {
    TEST_DESCRIPTION("Delete bufferView bound to cmd buffer, then attempt to submit cmd buffer.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    CreatePipelineHelper pipe(*this);
    VkBufferCreateInfo buffer_create_info = {};
    VkBufferViewCreateInfo bvci = {};
    VkBufferView view;

    {
        uint32_t queue_family_index = 0;
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;
        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        bvci.buffer = buffer.handle();
        bvci.format = VK_FORMAT_R32_SFLOAT;
        bvci.range = VK_WHOLE_SIZE;

        VkResult err = vkCreateBufferView(m_device->device(), &bvci, NULL, &view);
        ASSERT_VK_SUCCESS(err);

        descriptor_set.WriteDescriptorBufferView(0, view);
        descriptor_set.UpdateDescriptorSets();

        char const *fsSource =
            "#version 450\n"
            "\n"
            "layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;\n"
            "layout(location=0) out vec4 x;\n"
            "void main(){\n"
            "   x = imageLoad(s, 0);\n"
            "}\n";
        VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
        VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

        pipe.InitInfo();
        pipe.InitState();
        pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        VkViewport viewport = {0, 0, 16, 16, 0, 1};
        vkCmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        VkRect2D scissor = {{0, 0}, {16, 16}};
        vkCmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        // Bind pipeline to cmd buffer - This causes crash on Mali
        vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                &descriptor_set.set_, 0, nullptr);
    }
    // buffer is released.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "Descriptor in binding #0 index 0 is using buffer");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vkDestroyBufferView(m_device->device(), view, NULL);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "Descriptor in binding #0 index 0 is using bufferView");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    bvci.buffer = buffer.handle();
    VkResult err = vkCreateBufferView(m_device->device(), &bvci, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    descriptor_set.descriptor_writes.clear();
    descriptor_set.WriteDescriptorBufferView(0, view);
    descriptor_set.UpdateDescriptorSets();

    vkCmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                            &descriptor_set.set_, 0, nullptr);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Delete BufferView in order to invalidate cmd buffer
    vkDestroyBufferView(m_device->device(), view, NULL);
    // Now attempt submit of cmd buffer
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBufferView");
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferImageDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a command buffer that is invalid due to an image dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    {
        const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = NULL;
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
        VkImageObj image(m_device);
        image.init(&image_create_info);

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
        vkCmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &isr);
        m_commandBuffer->end();
    }
    // Destroy image dependency prior to submit to cause ERROR
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferFramebufferImageDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a framebuffer image dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vkGetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        printf("%s Image format doesn't support required features.\n", kSkipPrefix);
        return;
    }
    VkFramebuffer fb;
    VkImageView view;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    {
        VkImageCreateInfo image_ci = {};
        image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_ci.pNext = NULL;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_ci.extent.width = 32;
        image_ci.extent.height = 32;
        image_ci.extent.depth = 1;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.flags = 0;
        VkImageObj image(m_device);
        image.init(&image_ci);

        VkImageViewCreateInfo ivci = {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            image.handle(),
            VK_IMAGE_VIEW_TYPE_2D,
            VK_FORMAT_B8G8R8A8_UNORM,
            {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        err = vkCreateImageView(m_device->device(), &ivci, nullptr, &view);
        ASSERT_VK_SUCCESS(err);

        VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 32, 32, 1};
        err = vkCreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
        ASSERT_VK_SUCCESS(err);

        // Just use default renderpass with our framebuffer
        m_renderPassBeginInfo.framebuffer = fb;
        m_renderPassBeginInfo.renderArea.extent.width = 32;
        m_renderPassBeginInfo.renderArea.extent.height = 32;
        // Create Null cmd buffer for submit
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
    // Destroy image attached to framebuffer to invalidate cmd buffer
    // Now attempt to submit cmd buffer and verify error
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();

    vkDestroyFramebuffer(m_device->device(), fb, nullptr);
    vkDestroyImageView(m_device->device(), view, nullptr);
}

TEST_F(VkLayerTest, ImageMemoryNotBound) {
    TEST_DESCRIPTION("Attempt to draw with an image which has not had memory bound to it.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkImage image;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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
    VkResult err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);
    // Have to bind memory to image before recording cmd in cmd buffer using it
    VkMemoryRequirements mem_reqs;
    VkDeviceMemory image_mem;
    bool pass;
    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.memoryTypeIndex = 0;
    vkGetImageMemoryRequirements(m_device->device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    err = vkAllocateMemory(m_device->device(), &mem_alloc, NULL, &image_mem);
    ASSERT_VK_SUCCESS(err);

    // Introduce error, do not call vkBindImageMemory(m_device->device(), image, image_mem, 0);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
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
    vkCmdClearColorImage(m_commandBuffer->handle(), image, VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &isr);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
    vkDestroyImage(m_device->device(), image, NULL);
    vkFreeMemory(m_device->device(), image_mem, nullptr);
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

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buf_info.size = 1024;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vkCreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vkGetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = 1024;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vkDestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    err = vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    // Introduce failure by not calling vkBindBufferMemory(m_device->device(), buffer, mem, 0);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
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
    vkCmdCopyBufferToImage(m_commandBuffer->handle(), buffer, image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();

    vkDestroyBuffer(m_device->device(), buffer, NULL);
    vkFreeMemory(m_device->handle(), mem, NULL);
}

TEST_F(VkLayerTest, MultiplaneImageLayoutBadAspectFlags) {
    TEST_DESCRIPTION("Query layout of a multiplane image using illegal aspect flag masks");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_LINEAR;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify formats
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    supported = supported && ImageFormatAndFeaturesSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    VkImage image_2plane, image_3plane;
    ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    VkResult err = vkCreateImage(device(), &ci, NULL, &image_2plane);
    ASSERT_VK_SUCCESS(err);

    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    err = vkCreateImage(device(), &ci, NULL, &image_3plane);
    ASSERT_VK_SUCCESS(err);

    // Query layout of 3rd plane, for a 2-plane image
    VkImageSubresource subres = {};
    subres.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    subres.mipLevel = 0;
    subres.arrayLayer = 0;
    VkSubresourceLayout layout = {};

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageSubresourceLayout-format-01581");
    vkGetImageSubresourceLayout(device(), image_2plane, &subres, &layout);
    m_errorMonitor->VerifyFound();

    // Query layout using color aspect, for a 3-plane image
    subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageSubresourceLayout-format-01582");
    vkGetImageSubresourceLayout(device(), image_3plane, &subres, &layout);
    m_errorMonitor->VerifyFound();

    // Clean up
    vkDestroyImage(device(), image_2plane, NULL);
    vkDestroyImage(device(), image_3plane, NULL);
}

TEST_F(VkLayerTest, InvalidBufferViewObject) {
    // Create a single TEXEL_BUFFER descriptor and send it an invalid bufferView
    // First, cause the bufferView to be invalid due to underlying buffer being destroyed
    // Then destroy view itself and verify that same error is hit
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-00323");
    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });
    VkBufferView view;
    {
        // Create a valid bufferView to start with
        uint32_t queue_family_index = 0;
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;
        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        VkBufferViewCreateInfo bvci = {};
        bvci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        bvci.buffer = buffer.handle();
        bvci.format = VK_FORMAT_R32_SFLOAT;
        bvci.range = VK_WHOLE_SIZE;

        err = vkCreateBufferView(m_device->device(), &bvci, NULL, &view);
        ASSERT_VK_SUCCESS(err);
    }
    // First Destroy buffer underlying view which should hit error in CV

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    descriptor_write.pTexelBufferView = &view;

    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // Now destroy view itself and verify same error, which is hit in PV this time
    vkDestroyBufferView(m_device->device(), view, NULL);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-00323");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateBufferViewNoMemoryBoundToBuffer) {
    TEST_DESCRIPTION("Attempt to create a buffer view with a buffer that has no memory bound to it.");

    VkResult err;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " used with no memory bound. Memory should be bound by calling vkBindBufferMemory().");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create a buffer with no bound memory and then attempt to create
    // a buffer view.
    VkBufferCreateInfo buff_ci = {};
    buff_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buff_ci.size = 256;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer buffer;
    err = vkCreateBuffer(m_device->device(), &buff_ci, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferViewCreateInfo buff_view_ci = {};
    buff_view_ci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    buff_view_ci.buffer = buffer;
    buff_view_ci.format = VK_FORMAT_R8_UNORM;
    buff_view_ci.range = VK_WHOLE_SIZE;
    VkBufferView buff_view;
    err = vkCreateBufferView(m_device->device(), &buff_view_ci, NULL, &buff_view);

    m_errorMonitor->VerifyFound();
    vkDestroyBuffer(m_device->device(), buffer, NULL);
    // If last error is success, it still created the view, so delete it.
    if (err == VK_SUCCESS) {
        vkDestroyBufferView(m_device->device(), buff_view, NULL);
    }
}

TEST_F(VkLayerTest, InvalidBufferViewCreateInfoEntries) {
    TEST_DESCRIPTION("Attempt to create a buffer view with invalid create info.");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;
    const VkDeviceSize minTexelBufferOffsetAlignment = dev_limits.minTexelBufferOffsetAlignment;
    if (minTexelBufferOffsetAlignment == 1) {
        printf("%s Test requires minTexelOffsetAlignment to not be equal to 1. \n", kSkipPrefix);
        return;
    }

    const VkFormat format_with_uniform_texel_support = VK_FORMAT_R8G8B8A8_UNORM;
    const char *format_with_uniform_texel_support_string = "VK_FORMAT_R8G8B8A8_UNORM";
    const VkFormat format_without_texel_support = VK_FORMAT_R8G8B8_UNORM;
    const char *format_without_texel_support_string = "VK_FORMAT_R8G8B8_UNORM";
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(gpu(), format_with_uniform_texel_support, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        printf("%s Test requires %s to support VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT\n", kSkipPrefix,
               format_with_uniform_texel_support_string);
        return;
    }
    vkGetPhysicalDeviceFormatProperties(gpu(), format_without_texel_support, &format_properties);
    if ((format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) ||
        (format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        printf(
            "%s Test requires %s to not support VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT nor "
            "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT\n",
            kSkipPrefix, format_without_texel_support_string);
        return;
    }

    // Create a test buffer--buffer must have been created using VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT or
    // VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, so use a different usage value instead to cause an error
    const VkDeviceSize resource_size = 1024;
    const VkBufferCreateInfo bad_buffer_info = VkBufferObj::create_info(resource_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkBufferObj bad_buffer;
    bad_buffer.init(*m_device, bad_buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Create a test buffer view
    VkBufferViewCreateInfo buff_view_ci = {};
    buff_view_ci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    buff_view_ci.buffer = bad_buffer.handle();
    buff_view_ci.format = format_with_uniform_texel_support;
    buff_view_ci.range = VK_WHOLE_SIZE;
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-buffer-00932"});

    // Create a better test buffer
    const VkBufferCreateInfo buffer_info = VkBufferObj::create_info(resource_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Offset must be less than the size of the buffer, so set it equal to the buffer size to cause an error
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.offset = buffer.create_info().size;
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-offset-00925"});

    // Offset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment so add 1 to ensure it is not
    buff_view_ci.offset = minTexelBufferOffsetAlignment + 1;
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-offset-02749"});

    // Set offset to acceptable value for range tests
    buff_view_ci.offset = minTexelBufferOffsetAlignment;
    // Setting range equal to 0 will cause an error to occur
    buff_view_ci.range = 0;
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-range-00928"});

    uint32_t format_size = FormatElementSize(buff_view_ci.format);
    // Range must be a multiple of the element size of format, so add one to ensure it is not
    buff_view_ci.range = format_size + 1;
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-range-00929"});

    // Twice the element size of format multiplied by VkPhysicalDeviceLimits::maxTexelBufferElements guarantees range divided by the
    // element size is greater than maxTexelBufferElements, causing failure
    buff_view_ci.range = 2 * format_size * dev_limits.maxTexelBufferElements;
    CreateBufferViewTest(*this, &buff_view_ci,
                         {"VUID-VkBufferViewCreateInfo-range-00930", "VUID-VkBufferViewCreateInfo-offset-00931"});

    // Set rage to acceptable value for buffer tests
    buff_view_ci.format = format_without_texel_support;
    buff_view_ci.range = VK_WHOLE_SIZE;

    // `buffer` was created using VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT so we can use that for the first buffer test
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-buffer-00933"});

    // Create a new buffer using VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT
    const VkBufferCreateInfo storage_buffer_info =
        VkBufferObj::create_info(resource_size, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferObj storage_buffer;
    storage_buffer.init(*m_device, storage_buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    buff_view_ci.buffer = storage_buffer.handle();
    CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-buffer-00934"});
}

TEST_F(VkLayerTest, InvalidTexelBufferAlignment) {
    TEST_DESCRIPTION("Test VK_EXT_texel_buffer_alignment.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (DeviceIsMockICD() || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables texel_buffer_alignment
    auto texel_buffer_alignment_features = lvl_init_struct<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&texel_buffer_alignment_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    texel_buffer_alignment_features.texelBufferAlignment = VK_TRUE;

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props = {};
    align_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES_EXT;
    VkPhysicalDeviceProperties2 pd_props2 = {};
    pd_props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    pd_props2.pNext = &align_props;
    vkGetPhysicalDeviceProperties2(gpu(), &pd_props2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkFormat format_with_uniform_texel_support = VK_FORMAT_R8G8B8A8_UNORM;

    const VkDeviceSize resource_size = 1024;
    VkBufferCreateInfo buffer_info = VkBufferObj::create_info(
        resource_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Create a test buffer view
    VkBufferViewCreateInfo buff_view_ci = {};
    buff_view_ci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    buff_view_ci.buffer = buffer.handle();
    buff_view_ci.format = format_with_uniform_texel_support;
    buff_view_ci.range = VK_WHOLE_SIZE;

    buff_view_ci.offset = 1;
    std::vector<std::string> expectedErrors;
    if (buff_view_ci.offset < align_props.storageTexelBufferOffsetAlignmentBytes) {
        expectedErrors.push_back("VUID-VkBufferViewCreateInfo-buffer-02750");
    }
    if (buff_view_ci.offset < align_props.uniformTexelBufferOffsetAlignmentBytes) {
        expectedErrors.push_back("VUID-VkBufferViewCreateInfo-buffer-02751");
    }
    CreateBufferViewTest(*this, &buff_view_ci, expectedErrors);
    expectedErrors.clear();

    buff_view_ci.offset = 4;
    if (buff_view_ci.offset < align_props.storageTexelBufferOffsetAlignmentBytes &&
        !align_props.storageTexelBufferOffsetSingleTexelAlignment) {
        expectedErrors.push_back("VUID-VkBufferViewCreateInfo-buffer-02750");
    }
    if (buff_view_ci.offset < align_props.uniformTexelBufferOffsetAlignmentBytes &&
        !align_props.uniformTexelBufferOffsetSingleTexelAlignment) {
        expectedErrors.push_back("VUID-VkBufferViewCreateInfo-buffer-02751");
    }
    CreateBufferViewTest(*this, &buff_view_ci, expectedErrors);
    expectedErrors.clear();

    // Test a 3-component format
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_R32G32B32_SFLOAT, &format_properties);
    if (format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT) {
        buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        VkBufferObj buffer2;
        buffer2.init(*m_device, buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Create a test buffer view
        buff_view_ci.buffer = buffer2.handle();

        buff_view_ci.format = VK_FORMAT_R32G32B32_SFLOAT;
        buff_view_ci.offset = 1;
        if (buff_view_ci.offset < align_props.uniformTexelBufferOffsetAlignmentBytes) {
            expectedErrors.push_back("VUID-VkBufferViewCreateInfo-buffer-02751");
        }
        CreateBufferViewTest(*this, &buff_view_ci, expectedErrors);
        expectedErrors.clear();

        buff_view_ci.offset = 4;
        if (buff_view_ci.offset < align_props.uniformTexelBufferOffsetAlignmentBytes &&
            !align_props.uniformTexelBufferOffsetSingleTexelAlignment) {
            expectedErrors.push_back("VUID-VkBufferViewCreateInfo-buffer-02751");
        }
        CreateBufferViewTest(*this, &buff_view_ci, expectedErrors);
        expectedErrors.clear();
    }
}

TEST_F(VkLayerTest, FillBufferWithinRenderPass) {
    // Call CmdFillBuffer within an active renderpass
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdFillBuffer-renderpass");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkBufferObj dstBuffer;
    dstBuffer.init_as_dst(*m_device, (VkDeviceSize)1024, reqs);

    m_commandBuffer->FillBuffer(dstBuffer.handle(), 0, 4, 0x11111111);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, UpdateBufferWithinRenderPass) {
    // Call CmdUpdateBuffer within an active renderpass
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdUpdateBuffer-renderpass");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkBufferObj dstBuffer;
    dstBuffer.init_as_dst(*m_device, (VkDeviceSize)1024, reqs);

    VkDeviceSize dstOffset = 0;
    uint32_t Data[] = {1, 2, 3, 4, 5, 6, 7, 8};
    VkDeviceSize dataSize = sizeof(Data) / sizeof(uint32_t);
    vkCmdUpdateBuffer(m_commandBuffer->handle(), dstBuffer.handle(), dstOffset, dataSize, &Data);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ClearColorImageWithBadRange) {
    TEST_DESCRIPTION("Record clear color with an invalid VkImageSubresourceRange");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    m_commandBuffer->begin();
    const auto cb_handle = m_commandBuffer->handle();

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-baseMipLevel-01470");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-baseMipLevel-01470");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
        vkCmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ClearDepthStencilWithBadRange) {
    TEST_DESCRIPTION("Record clear depth with an invalid VkImageSubresourceRange");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    image.Init(32, 32, 1, depth_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());
    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    const VkClearDepthStencilValue clear_value = {};

    m_commandBuffer->begin();
    const auto cb_handle = m_commandBuffer->handle();

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        const VkImageSubresourceRange range = {ds_aspect, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 1, 1, 0, 1};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 0, 0, 0, 1};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 0, 2, 0, 1};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, 1};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 0};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 2};
        vkCmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ClearColorImageWithinRenderPass) {
    // Call CmdClearColorImage within an active RenderPass
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-renderpass");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkClearColorValue clear_color;
    memset(clear_color.uint32, 0, sizeof(uint32_t) * 4);
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VkImageObj dstImage(m_device);
    dstImage.init(&image_create_info);

    const VkImageSubresourceRange range = VkImageObj::subresource_range(image_create_info, VK_IMAGE_ASPECT_COLOR_BIT);

    vkCmdClearColorImage(m_commandBuffer->handle(), dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ClearDepthStencilImageErrors) {
    // Hit errors related to vkCmdClearDepthStencilImage()
    // 1. Use an image that doesn't have VK_IMAGE_USAGE_TRANSFER_DST_BIT set
    // 2. Call CmdClearDepthStencilImage within an active RenderPass

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    VkClearDepthStencilValue clear_value = {0};
    VkImageCreateInfo image_create_info = VkImageObj::create_info();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    // Error here is that VK_IMAGE_USAGE_TRANSFER_DST_BIT is excluded for DS image that we'll call Clear on below
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageObj dst_image_bad_usage(m_device);
    dst_image_bad_usage.init(&image_create_info);
    const VkImageSubresourceRange range = VkImageObj::subresource_range(image_create_info, VK_IMAGE_ASPECT_DEPTH_BIT);

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-image-00009");
    vkCmdClearDepthStencilImage(m_commandBuffer->handle(), dst_image_bad_usage.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                &range);
    m_errorMonitor->VerifyFound();

    // Fix usage for next test case
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj dst_image(m_device);
    dst_image.init(&image_create_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-renderpass");
    vkCmdClearDepthStencilImage(m_commandBuffer->handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BufferMemoryBarrierNoBuffer) {
    // Try to add a buffer memory barrier with no buffer.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "required parameter pBufferMemoryBarriers[0].buffer specified as VK_NULL_HANDLE");

    ASSERT_NO_FATAL_FAILURE(Init());
    m_commandBuffer->begin();

    VkBufferMemoryBarrier buf_barrier = {};
    buf_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buf_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.buffer = VK_NULL_HANDLE;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0, nullptr,
                         1, &buf_barrier, 0, nullptr);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidBarriers) {
    TEST_DESCRIPTION("A variety of ways to get VK_INVALID_BARRIER ");

    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    // Add a token self-dependency for this test to avoid unexpected errors
    m_addRenderPassSelfDependency = true;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->queue_props.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->queue_props[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr, false, false);

    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test(" used with no memory bound. Memory should be bound by calling vkBindImageMemory()",
              " used with no memory bound. Memory should be bound by calling vkBindBufferMemory()");

    VkBufferObj buffer;
    VkMemoryPropertyFlags mem_reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    buffer.init_as_src_and_dst(*m_device, 256, mem_reqs);
    conc_test.buffer_barrier_.buffer = buffer.handle();

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    conc_test.image_barrier_.image = image.handle();

    // New layout can't be UNDEFINED
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    conc_test("VUID-VkImageMemoryBarrier-newLayout-01198", "");

    // Transition image to color attachment optimal
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test("");

    // TODO: this looks vestigal or incomplete...
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Can't send buffer memory barrier during a render pass
    vkCmdEndRenderPass(m_commandBuffer->handle());

    // Duplicate barriers that change layout
    VkImageMemoryBarrier img_barrier = {};
    img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier.pNext = NULL;
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    VkImageMemoryBarrier img_barriers[2] = {img_barrier, img_barrier};

    // Transitions from UNDEFINED  are valid, even if duplicated
    m_errorMonitor->ExpectSuccess();
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                         img_barriers);
    m_errorMonitor->VerifyNotFound();

    // Duplication of layout transitions (not from undefined) are not valid
    img_barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barriers[1].oldLayout = img_barriers[0].oldLayout;
    img_barriers[1].newLayout = img_barriers[0].newLayout;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-oldLayout-01197");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                         img_barriers);
    m_errorMonitor->VerifyFound();

    // Exceed the buffer size
    conc_test.buffer_barrier_.offset = conc_test.buffer_.create_info().size + 1;
    conc_test("", "VUID-VkBufferMemoryBarrier-offset-01187");

    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = conc_test.buffer_.create_info().size + 1;
    // Size greater than total size
    conc_test("", "VUID-VkBufferMemoryBarrier-size-01189");

    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;

    // Now exercise barrier aspect bit errors, first DS
    VkDepthStencilObj ds_image(m_device);
    ds_image.Init(m_device, 128, 128, depth_format);
    ASSERT_TRUE(ds_image.initialized());

    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = ds_image.handle();

    // Not having DEPTH or STENCIL set is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageSubresource-aspectMask-parameter");
    conc_test("VUID-VkImageMemoryBarrier-image-01207");

    // Having only one of depth or stencil set for DS image is an error
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    conc_test("VUID-VkImageMemoryBarrier-image-01207");

    // Having anything other than DEPTH and STENCIL is an error
    conc_test.image_barrier_.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
    conc_test("VUID-VkImageSubresource-aspectMask-parameter");

    // Now test depth-only
    VkFormatProperties format_props;
    vkGetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D16_UNORM, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        VkDepthStencilObj d_image(m_device);
        d_image.Init(m_device, 128, 128, VK_FORMAT_D16_UNORM);
        ASSERT_TRUE(d_image.initialized());

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = d_image.handle();

        // DEPTH bit must be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
        conc_test("Depth-only image formats must have the VK_IMAGE_ASPECT_DEPTH_BIT set.");

        // No bits other than DEPTH may be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("Depth-only image formats can have only the VK_IMAGE_ASPECT_DEPTH_BIT set.");
    }

    // Now test stencil-only
    vkGetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_S8_UINT, &format_props);
    if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        VkDepthStencilObj s_image(m_device);
        s_image.Init(m_device, 128, 128, VK_FORMAT_S8_UINT);
        ASSERT_TRUE(s_image.initialized());

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = s_image.handle();

        // Use of COLOR aspect on depth image is error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("Stencil-only image formats must have the VK_IMAGE_ASPECT_STENCIL_BIT set.");
    }

    // Finally test color
    VkImageObj c_image(m_device);
    c_image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(c_image.initialized());
    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = c_image.handle();

    // COLOR bit must be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    conc_test("Color image formats must have the VK_IMAGE_ASPECT_COLOR_BIT set.");

    // No bits other than COLOR may be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    conc_test("Color image formats must have ONLY the VK_IMAGE_ASPECT_COLOR_BIT set.");

    // A barrier's new and old VkImageLayout must be compatible with an image's VkImageUsageFlags.
    {
        VkImageObj img_color(m_device);
        img_color.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_color.initialized());

        VkImageObj img_ds(m_device);
        img_ds.Init(128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_ds.initialized());

        VkImageObj img_xfer_src(m_device);
        img_xfer_src.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_src.initialized());

        VkImageObj img_xfer_dst(m_device);
        img_xfer_dst.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_xfer_dst.initialized());

        VkImageObj img_sampled(m_device);
        img_sampled.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_sampled.initialized());

        VkImageObj img_input(m_device);
        img_input.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
        ASSERT_TRUE(img_input.initialized());

        const struct {
            VkImageObj &image_obj;
            VkImageLayout bad_layout;
            std::string msg_code;
        } bad_buffer_layouts[] = {
            // clang-format off
            // images _without_ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            {img_ds,       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_xfer_src, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_sampled,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            {img_input,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01208"},
            // images _without_ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01209"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier-oldLayout-01210"},
            // images _without_ VK_IMAGE_USAGE_SAMPLED_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_ds,       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_xfer_src, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier-oldLayout-01211"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01212"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_DST_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier-oldLayout-01213"},
            // clang-format on
        };
        const uint32_t layout_count = sizeof(bad_buffer_layouts) / sizeof(bad_buffer_layouts[0]);

        for (uint32_t i = 0; i < layout_count; ++i) {
            conc_test.image_barrier_.image = bad_buffer_layouts[i].image_obj.handle();
            const VkImageUsageFlags usage = bad_buffer_layouts[i].image_obj.usage();
            conc_test.image_barrier_.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                                       ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                                       : VK_IMAGE_ASPECT_COLOR_BIT;

            conc_test.image_barrier_.oldLayout = bad_buffer_layouts[i].bad_layout;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test(bad_buffer_layouts[i].msg_code);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.newLayout = bad_buffer_layouts[i].bad_layout;
            conc_test(bad_buffer_layouts[i].msg_code);
        }

        conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        conc_test.image_barrier_.image = image.handle();
    }

    // Attempt barrier where srcAccessMask is not supported by srcStageMask
    // Have lower-order bit that's supported (shader write), but higher-order bit not supported to verify multi-bit validation
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;
    conc_test("", "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01184");

    // Attempt barrier where dstAccessMask is not supported by dstStageMask
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    conc_test("", "VUID-vkCmdPipelineBarrier-pMemoryBarriers-01185");

    // Attempt to mismatch barriers/waitEvents calls with incompatible queues
    // Create command pool with incompatible queueflags
    const std::vector<VkQueueFamilyProperties> queue_props = m_device->queue_props;
    uint32_t queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT);
    if (queue_family_index == UINT32_MAX) {
        printf("%s No non-compute queue supporting graphics found; skipped.\n", kSkipPrefix);
        return;  // NOTE: this exits the test function!
    }

    VkBufferMemoryBarrier buf_barrier = {};
    buf_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buf_barrier.pNext = NULL;
    buf_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    buf_barrier.buffer = buffer.handle();
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdPipelineBarrier-srcStageMask-01183");

    VkCommandPoolObj command_pool(m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj bad_command_buffer(m_device, &command_pool);

    bad_command_buffer.begin();
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    // Set two bits that should both be supported as a bonus positive check
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(bad_command_buffer.handle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buf_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();

    // Check for error for trying to wait on pipeline stage not supported by this queue. Specifically since our queue is not a
    // compute queue, vkCmdWaitEvents cannot have it's source stage mask be VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdWaitEvents-srcStageMask-01164");
    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vkCreateEvent(m_device->device(), &event_create_info, nullptr, &event);
    vkCmdWaitEvents(bad_command_buffer.handle(), 1, &event, /*source stage mask*/ VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    bad_command_buffer.end();

    vkDestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, InvalidBarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->queue_props.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->queue_props[other_family].queueCount == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    if (m_device->props.apiVersion >= VK_API_VERSION_1_1) {
        printf(
            "%s Device has apiVersion greater than 1.0 -- skipping test cases that require external memory "
            "to be "
            "disabled.\n",
            kSkipPrefix);
    } else {
        if (only_one_family) {
            printf("%s Single queue family found -- VK_SHARING_MODE_CONCURRENT testcases skipped.\n", kSkipPrefix);
        } else {
            std::vector<uint32_t> families = {submit_family, other_family};
            BarrierQueueFamilyTestHelper conc_test(&test_context);
            conc_test.Init(&families);
            // core_validation::barrier_queue_families::kSrcAndDestMustBeIgnore
            conc_test("VUID-VkImageMemoryBarrier-image-01199", "VUID-VkBufferMemoryBarrier-buffer-01190", VK_QUEUE_FAMILY_IGNORED,
                      submit_family);
            conc_test("VUID-VkImageMemoryBarrier-image-01199", "VUID-VkBufferMemoryBarrier-buffer-01190", submit_family,
                      VK_QUEUE_FAMILY_IGNORED);
            conc_test("VUID-VkImageMemoryBarrier-image-01199", "VUID-VkBufferMemoryBarrier-buffer-01190", submit_family,
                      submit_family);
            // true -> positive test
            conc_test("VUID-VkImageMemoryBarrier-image-01199", "VUID-VkBufferMemoryBarrier-buffer-01190", VK_QUEUE_FAMILY_IGNORED,
                      VK_QUEUE_FAMILY_IGNORED, true);
        }

        BarrierQueueFamilyTestHelper excl_test(&test_context);
        excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

        // core_validation::barrier_queue_families::kBothIgnoreOrBothValid
        excl_test("VUID-VkImageMemoryBarrier-image-01200", "VUID-VkBufferMemoryBarrier-buffer-01192", VK_QUEUE_FAMILY_IGNORED,
                  submit_family);
        excl_test("VUID-VkImageMemoryBarrier-image-01200", "VUID-VkBufferMemoryBarrier-buffer-01192", submit_family,
                  VK_QUEUE_FAMILY_IGNORED);
        // true -> positive test
        excl_test("VUID-VkImageMemoryBarrier-image-01200", "VUID-VkBufferMemoryBarrier-buffer-01192", submit_family, submit_family,
                  true);
        excl_test("VUID-VkImageMemoryBarrier-image-01200", "VUID-VkBufferMemoryBarrier-buffer-01192", VK_QUEUE_FAMILY_IGNORED,
                  VK_QUEUE_FAMILY_IGNORED, true);
    }

    if (only_one_family) {
        printf("%s Single queue family found -- VK_SHARING_MODE_EXCLUSIVE submit testcases skipped.\n", kSkipPrefix);
    } else {
        BarrierQueueFamilyTestHelper excl_test(&test_context);
        excl_test.Init(nullptr);

        // core_validation::barrier_queue_families::kSubmitQueueMustMatchSrcOrDst
        excl_test("VUID-VkImageMemoryBarrier-image-01205", "VUID-VkBufferMemoryBarrier-buffer-01196", other_family, other_family,
                  false, submit_family);

        // true -> positive test (testing both the index logic and the QFO transfer tracking.
        excl_test("POSITIVE_TEST", "POSITIVE_TEST", submit_family, other_family, true, submit_family);
        excl_test("POSITIVE_TEST", "POSITIVE_TEST", submit_family, other_family, true, other_family);
        excl_test("POSITIVE_TEST", "POSITIVE_TEST", other_family, submit_family, true, other_family);
        excl_test("POSITIVE_TEST", "POSITIVE_TEST", other_family, submit_family, true, submit_family);

        // negative testing for QFO transfer tracking
        // Duplicate release in one CB
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00001", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00001", submit_family,
                  other_family, false, submit_family, BarrierQueueFamilyTestHelper::DOUBLE_RECORD);
        // Duplicate pending release
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00003", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00003", submit_family,
                  other_family, false, submit_family);
        // Duplicate acquire in one CB
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00001", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00001", submit_family,
                  other_family, false, other_family, BarrierQueueFamilyTestHelper::DOUBLE_RECORD);
        // No pending release
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00004", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00004", submit_family,
                  other_family, false, other_family);
        // Duplicate release in two CB
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00002", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00002", submit_family,
                  other_family, false, submit_family, BarrierQueueFamilyTestHelper::DOUBLE_COMMAND_BUFFER);
        // Duplicate acquire in two CB
        excl_test("POSITIVE_TEST", "POSITIVE_TEST", submit_family, other_family, true, submit_family);  // need a succesful release
        excl_test("UNASSIGNED-VkImageMemoryBarrier-image-00002", "UNASSIGNED-VkBufferMemoryBarrier-buffer-00002", submit_family,
                  other_family, false, other_family, BarrierQueueFamilyTestHelper::DOUBLE_COMMAND_BUFFER);
    }
}

TEST_F(VkLayerTest, InvalidBarrierQueueFamilyWithMemExt) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families when memory extension is enabled ");
    std::vector<const char *> reqd_instance_extensions = {
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME}};
    for (auto extension_name : reqd_instance_extensions) {
        if (InstanceExtensionSupported(extension_name)) {
            m_instance_extension_names.push_back(extension_name);
        } else {
            printf("%s Required instance extension %s not supported, skipping test\n", kSkipPrefix, extension_name);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    // Check for external memory device extensions
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    } else {
        printf("%s External memory extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->queue_props.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->queue_props[other_family].queueCount == 0);

    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    if (only_one_family) {
        printf("%s Single queue family found -- VK_SHARING_MODE_CONCURRENT testcases skipped.\n", kSkipPrefix);
    } else {
        std::vector<uint32_t> families = {submit_family, other_family};
        BarrierQueueFamilyTestHelper conc_test(&test_context);

        // core_validation::barrier_queue_families::kSrcOrDstMustBeIgnore
        conc_test.Init(&families);
        conc_test("VUID-VkImageMemoryBarrier-image-01381", "VUID-VkBufferMemoryBarrier-buffer-01191", submit_family, submit_family);
        // true -> positive test
        conc_test("VUID-VkImageMemoryBarrier-image-01381", "VUID-VkBufferMemoryBarrier-buffer-01191", VK_QUEUE_FAMILY_IGNORED,
                  VK_QUEUE_FAMILY_IGNORED, true);
        conc_test("VUID-VkImageMemoryBarrier-image-01381", "VUID-VkBufferMemoryBarrier-buffer-01191", VK_QUEUE_FAMILY_IGNORED,
                  VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
        conc_test("VUID-VkImageMemoryBarrier-image-01381", "VUID-VkBufferMemoryBarrier-buffer-01191", VK_QUEUE_FAMILY_EXTERNAL_KHR,
                  VK_QUEUE_FAMILY_IGNORED, true);

        // core_validation::barrier_queue_families::kSpecialOrIgnoreOnly
        conc_test("VUID-VkImageMemoryBarrier-image-01766", "VUID-VkBufferMemoryBarrier-buffer-01763", submit_family,
                  VK_QUEUE_FAMILY_IGNORED);
        conc_test("VUID-VkImageMemoryBarrier-image-01766", "VUID-VkBufferMemoryBarrier-buffer-01763", VK_QUEUE_FAMILY_IGNORED,
                  submit_family);
        // This is to flag the errors that would be considered only "unexpected" in the parallel case above
        // true -> positive test
        conc_test("VUID-VkImageMemoryBarrier-image-01766", "VUID-VkBufferMemoryBarrier-buffer-01763", VK_QUEUE_FAMILY_IGNORED,
                  VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
        conc_test("VUID-VkImageMemoryBarrier-image-01766", "VUID-VkBufferMemoryBarrier-buffer-01763", VK_QUEUE_FAMILY_EXTERNAL_KHR,
                  VK_QUEUE_FAMILY_IGNORED, true);
    }

    BarrierQueueFamilyTestHelper excl_test(&test_context);
    excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

    // core_validation::barrier_queue_families::kSrcIgnoreRequiresDstIgnore
    excl_test("VUID-VkImageMemoryBarrier-image-01201", "VUID-VkBufferMemoryBarrier-buffer-01193", VK_QUEUE_FAMILY_IGNORED,
              submit_family);
    excl_test("VUID-VkImageMemoryBarrier-image-01201", "VUID-VkBufferMemoryBarrier-buffer-01193", VK_QUEUE_FAMILY_IGNORED,
              VK_QUEUE_FAMILY_EXTERNAL_KHR);
    // true -> positive test
    excl_test("VUID-VkImageMemoryBarrier-image-01201", "VUID-VkBufferMemoryBarrier-buffer-01193", VK_QUEUE_FAMILY_IGNORED,
              VK_QUEUE_FAMILY_IGNORED, true);

    // core_validation::barrier_queue_families::kDstValidOrSpecialIfNotIgnore
    excl_test("VUID-VkImageMemoryBarrier-image-01768", "VUID-VkBufferMemoryBarrier-buffer-01765", submit_family, invalid);
    // true -> positive test
    excl_test("VUID-VkImageMemoryBarrier-image-01768", "VUID-VkBufferMemoryBarrier-buffer-01765", submit_family, submit_family,
              true);
    excl_test("VUID-VkImageMemoryBarrier-image-01768", "VUID-VkBufferMemoryBarrier-buffer-01765", submit_family,
              VK_QUEUE_FAMILY_IGNORED, true);
    excl_test("VUID-VkImageMemoryBarrier-image-01768", "VUID-VkBufferMemoryBarrier-buffer-01765", submit_family,
              VK_QUEUE_FAMILY_EXTERNAL_KHR, true);

    // core_validation::barrier_queue_families::kSrcValidOrSpecialIfNotIgnore
    excl_test("VUID-VkImageMemoryBarrier-image-01767", "VUID-VkBufferMemoryBarrier-buffer-01764", invalid, submit_family);
    // true -> positive test
    excl_test("VUID-VkImageMemoryBarrier-image-01767", "VUID-VkBufferMemoryBarrier-buffer-01764", submit_family, submit_family,
              true);
    excl_test("VUID-VkImageMemoryBarrier-image-01767", "VUID-VkBufferMemoryBarrier-buffer-01764", VK_QUEUE_FAMILY_IGNORED,
              VK_QUEUE_FAMILY_IGNORED, true);
    excl_test("VUID-VkImageMemoryBarrier-image-01767", "VUID-VkBufferMemoryBarrier-buffer-01764", VK_QUEUE_FAMILY_EXTERNAL_KHR,
              submit_family, true);
}

TEST_F(VkLayerTest, ImageBarrierWithBadRange) {
    TEST_DESCRIPTION("VkImageMemoryBarrier with an invalid subresourceRange");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageMemoryBarrier img_barrier_template = {};
    img_barrier_template.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    img_barrier_template.pNext = NULL;
    img_barrier_template.srcAccessMask = 0;
    img_barrier_template.dstAccessMask = 0;
    img_barrier_template.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier_template.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier_template.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier_template.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // subresourceRange to be set later for the for the purposes of this test
    img_barrier_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier_template.subresourceRange.baseArrayLayer = 0;
    img_barrier_template.subresourceRange.baseMipLevel = 0;
    img_barrier_template.subresourceRange.layerCount = 0;
    img_barrier_template.subresourceRange.levelCount = 0;

    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t invalid = static_cast<uint32_t>(m_device->queue_props.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (invalid == 1) || (m_device->queue_props[other_family].queueCount == 0);
    std::vector<uint32_t> qf_indices{{submit_family, other_family}};
    if (only_one_family) {
        qf_indices.resize(1);
    }
    BarrierQueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    BarrierQueueFamilyTestHelper conc_test(&test_context);
    conc_test.Init(nullptr);
    img_barrier_template.image = conc_test.image_.handle();
    conc_test.image_barrier_ = img_barrier_template;
    // Nested scope here confuses clang-format, somehow
    // clang-format off

    // try for vkCmdPipelineBarrier
    {
        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try levelCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01724");
        }

        // Try baseMipLevel + levelCount > image.mipLevels
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01724");
        }

        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01488");
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01488");
        }

        // Try layerCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01725");
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01725");
        }
    }

    m_commandBuffer->begin();
    // try for vkCmdWaitEvents
    {
        VkEvent event;
        VkEventCreateInfo eci{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO, NULL, 0};
        VkResult err = vkCreateEvent(m_device->handle(), &eci, nullptr, &event);
        ASSERT_VK_SUCCESS(err);

        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01486");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01486");
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try levelCount = 0
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel + levelCount > image.mipLevels
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01488");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01488");
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try layerCount = 0
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vkCmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        vkDestroyEvent(m_device->handle(), event, nullptr);
    }
    // clang-format on
}

TEST_F(VkLayerTest, IdxBufferAlignmentError) {
    // Bind a BeginRenderPass within an active RenderPass
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t const indices[] = {0};
    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size = 1024;
    buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buf_info.queueFamilyIndexCount = 1;
    buf_info.pQueueFamilyIndices = indices;

    VkBufferObj buffer;
    buffer.init(*m_device, buf_info);

    m_commandBuffer->begin();

    // vkCmdBindPipeline(m_commandBuffer->handle(),
    // VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // Should error before calling to driver so don't care about actual data
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "vkCmdBindIndexBuffer() offset (0x7) does not fall on ");
    vkCmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 7, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Bad2DArrayImageType) {
    TEST_DESCRIPTION("Create an image with a flag specifying 2D_ARRAY_COMPATIBLE but not of imageType 3D.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    } else {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Trigger check by setting imagecreateflags to 2d_array_compat and imageType to 2D
    VkImageCreateInfo ici = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                             nullptr,
                             VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR,
                             VK_IMAGE_TYPE_2D,
                             VK_FORMAT_R8G8B8A8_UNORM,
                             {32, 32, 1},
                             1,
                             1,
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_SHARING_MODE_EXCLUSIVE,
                             0,
                             nullptr,
                             VK_IMAGE_LAYOUT_UNDEFINED};
    CreateImageTest(*this, &ici, "VUID-VkImageCreateInfo-flags-00950");
}

TEST_F(VkLayerTest, VertexBufferInvalid) {
    TEST_DESCRIPTION(
        "Submit a command buffer using deleted vertex buffer, delete a buffer twice, use an invalid offset for each buffer type, "
        "and attempt to bind a null buffer");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vkCmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");

    {
        // Create and bind a vertex buffer in a reduced scope, which will cause it to be deleted upon leaving this scope
        const float vbo_data[3] = {1.f, 0.f, 1.f};
        VkVerticesObj draw_verticies(m_device, 1, 1, sizeof(vbo_data[0]), sizeof(vbo_data) / sizeof(vbo_data[0]), vbo_data);
        draw_verticies.BindVertexBuffers(m_commandBuffer->handle());
        draw_verticies.AddVertexInputToPipeHelpr(&pipe);

        m_commandBuffer->Draw(1, 0, 0, 0);

        m_commandBuffer->EndRenderPass();
    }

    vkEndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    {
        // Create and bind a vertex buffer in a reduced scope, and delete it
        // twice, the second through the destructor
        VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VkBufferTest::eDoubleDelete);
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkDestroyBuffer-buffer-parameter");
        buffer_test.TestDoubleDestroy();
    }
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetUnexpectedError("value of pCreateInfo->usage must not be 0");
    if (VkBufferTest::GetTestConditionValid(m_device, VkBufferTest::eInvalidMemoryOffset)) {
        // Create and bind a memory buffer with an invalid offset.
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-memoryOffset-01036");
        m_errorMonitor->SetUnexpectedError(
            "If buffer was created with the VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT or VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "
            "memoryOffset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment");
        VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VkBufferTest::eInvalidMemoryOffset);
        (void)buffer_test;
        m_errorMonitor->VerifyFound();
    }

    {
        // Attempt to bind a null buffer.
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "vkBindBufferMemory: required parameter buffer specified as VK_NULL_HANDLE");
        VkBufferTest buffer_test(m_device, 0, VkBufferTest::eBindNullBuffer);
        (void)buffer_test;
        m_errorMonitor->VerifyFound();
    }

    {
        // Attempt to bind a fake buffer.
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-buffer-parameter");
        VkBufferTest buffer_test(m_device, 0, VkBufferTest::eBindFakeBuffer);
        (void)buffer_test;
        m_errorMonitor->VerifyFound();
    }

    {
        // Attempt to use an invalid handle to delete a buffer.
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkFreeMemory-memory-parameter");
        VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VkBufferTest::eFreeInvalidHandle);
        (void)buffer_test;
    }
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BadVertexBufferOffset) {
    TEST_DESCRIPTION("Submit an offset past the end of a vertex buffer");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    static const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void *)&vbo_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindVertexBuffers-pOffsets-00626");
    m_commandBuffer->BindVertexBuffer(&vbo, (VkDeviceSize)(3 * sizeof(float)), 1);  // Offset at the end of the buffer
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

// INVALID_IMAGE_LAYOUT tests (one other case is hit by MapMemWithoutHostVisibleBit and not here)
TEST_F(VkLayerTest, InvalidImageLayout) {
    TEST_DESCRIPTION(
        "Hit all possible validation checks associated with the UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout error. "
        "Generally these involve having images in the wrong layout when they're copied or transitioned.");
    // 3 in ValidateCmdBufImageLayouts
    // *  -1 Attempt to submit cmd buf w/ deleted image
    // *  -2 Cmd buf submit of image w/ layout not matching first use w/ subresource
    // *  -3 Cmd buf submit of image w/ layout not matching first use w/o subresource

    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    // Create src & dst images to use for copy operations
    VkImageObj src_image(m_device);
    VkImageObj dst_image(m_device);
    VkImageObj depth_image(m_device);

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 4;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;

    src_image.init(&image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    dst_image.init(&image_create_info);

    image_create_info.format = VK_FORMAT_D16_UNORM;
    image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_image.init(&image_create_info);

    m_commandBuffer->begin();
    VkImageCopy copy_region;
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.srcOffset.x = 0;
    copy_region.srcOffset.y = 0;
    copy_region.srcOffset.z = 0;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.dstOffset.x = 0;
    copy_region.dstOffset.y = 0;
    copy_region.dstOffset.z = 0;
    copy_region.extent.width = 1;
    copy_region.extent.height = 1;
    copy_region.extent.depth = 1;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");

    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    // The first call hits the expected WARNING and skips the call down the chain, so call a second time to call down chain and
    // update layer state
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    // Now cause error due to src image layout changing
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyImage-srcImageLayout-00128");
    m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    // Final src error is due to bad layout type
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyImage-srcImageLayout-00129");
    m_errorMonitor->SetUnexpectedError(
        "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    // Now verify same checks for dst
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    // Now cause error due to src image layout changing
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyImage-dstImageLayout-00133");
    m_errorMonitor->SetUnexpectedError(
        "is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdCopyImage-dstImageLayout-00134");
    m_errorMonitor->SetUnexpectedError(
        "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    // Convert dst and depth images to TRANSFER_DST for subsequent tests
    VkImageMemoryBarrier transfer_dst_image_barrier[1] = {};
    transfer_dst_image_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    transfer_dst_image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transfer_dst_image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transfer_dst_image_barrier[0].srcAccessMask = 0;
    transfer_dst_image_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transfer_dst_image_barrier[0].image = dst_image.handle();
    transfer_dst_image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    transfer_dst_image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         NULL, 0, NULL, 1, transfer_dst_image_barrier);
    transfer_dst_image_barrier[0].image = depth_image.handle();
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         NULL, 0, NULL, 1, transfer_dst_image_barrier);

    // Cause errors due to clearing with invalid image layouts
    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    // Fail due to explicitly prohibited layout for color clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-imageLayout-00005");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-imageLayout-00004");
    m_commandBuffer->ClearColorImage(dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for color clear.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearColorImage-imageLayout-00004");
    m_commandBuffer->ClearColorImage(dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();

    VkClearDepthStencilValue depth_clear_value = {};
    clear_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Fail due to explicitly prohibited layout for depth clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-imageLayout-00012");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    m_commandBuffer->ClearDepthStencilImage(depth_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &depth_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for depth clear.
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    m_commandBuffer->ClearDepthStencilImage(depth_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &depth_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();

    // Now cause error due to bad image layout transition in PipelineBarrier
    VkImageMemoryBarrier image_barrier[1] = {};
    image_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    image_barrier[0].image = src_image.handle();
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-oldLayout-01197");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageMemoryBarrier-oldLayout-01210");
    vkCmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                         NULL, 0, NULL, 1, image_barrier);
    m_errorMonitor->VerifyFound();

    // Finally some layout errors at RenderPass create time
    // Just hacking in specific state to get to the errors we want so don't copy this unless you know what you're doing.
    VkAttachmentReference attach = {};
    // perf warning for GENERAL layout w/ non-DS input attachment
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &attach;
    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_UNDEFINED;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "Layout for input attachment is GENERAL but should be READ_ONLY_OPTIMAL.");
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    // error w/ non-general layout
    attach.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    m_errorMonitor->SetDesiredFailureMsg(
        VK_DEBUG_REPORT_ERROR_BIT_EXT,
        "Layout for input attachment is VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL but can only be READ_ONLY_OPTIMAL or GENERAL.");
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    subpass.inputAttachmentCount = 0;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    // perf warning for GENERAL layout on color attachment
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "Layout for color attachment is GENERAL but should be COLOR_ATTACHMENT_OPTIMAL.");
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    // error w/ non-color opt or GENERAL layout for color attachment
    attach.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(
        VK_DEBUG_REPORT_ERROR_BIT_EXT,
        "Layout for color attachment is VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL but can only be COLOR_ATTACHMENT_OPTIMAL or GENERAL.");
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &attach;
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    // perf warning for GENERAL layout on DS attachment
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "GENERAL layout for depth attachment may not give optimal performance.");
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    // error w/ non-ds opt or GENERAL layout for color attachment
    attach.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "Layout for depth attachment is VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL but can only be "
                                         "DEPTH_STENCIL_ATTACHMENT_OPTIMAL, DEPTH_STENCIL_READ_ONLY_OPTIMAL or GENERAL.");
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    // For this error we need a valid renderpass so create default one
    attach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach.attachment = 0;
    attach_desc.format = depth_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Can't do a CLEAR load on READ_ONLY initialLayout
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "with invalid first layout VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL");
    vkCreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidStorageImageLayout) {
    TEST_DESCRIPTION("Attempt to update a STORAGE_IMAGE descriptor w/o GENERAL layout.");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageTiling tiling;
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(gpu(), tex_format, &format_properties);
    if (format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        tiling = VK_IMAGE_TILING_LINEAR;
    } else if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        tiling = VK_IMAGE_TILING_OPTIMAL;
    } else {
        printf("%s Device does not support VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT; skipped.\n", kSkipPrefix);
        return;
    }

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkImageObj image(m_device);
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_STORAGE_BIT, tiling, 0);
    ASSERT_TRUE(image.initialized());
    VkImageView view = image.targetView(tex_format);

    descriptor_set.WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         " of VK_DESCRIPTOR_TYPE_STORAGE_IMAGE type is being updated with layout "
                                         "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL but according to spec ");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateImageViewBreaksParameterCompatibilityRequirements) {
    TEST_DESCRIPTION(
        "Attempts to create an Image View with a view type that does not match the image type it is being created from.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memProps);

    // Test mismatch detection for image of type VK_IMAGE_TYPE_1D
    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_1D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image1D(m_device);
    image1D.init(&imgInfo);
    ASSERT_TRUE(image1D.initialized());

    // Initialize VkImageViewCreateInfo with mismatched viewType
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image1D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci,
                        "vkCreateImageView(): pCreateInfo->viewType VK_IMAGE_VIEW_TYPE_2D is not compatible with image");

    // Test mismatch detection for image of type VK_IMAGE_TYPE_2D
    imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               nullptr,
               VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
               VK_IMAGE_TYPE_2D,
               VK_FORMAT_R8G8B8A8_UNORM,
               {1, 1, 1},
               1,
               6,
               VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_SHARING_MODE_EXCLUSIVE,
               0,
               nullptr,
               VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image2D(m_device);
    image2D.init(&imgInfo);
    ASSERT_TRUE(image2D.initialized());

    // Initialize VkImageViewCreateInfo with mismatched viewType
    ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci,
                        "vkCreateImageView(): pCreateInfo->viewType VK_IMAGE_VIEW_TYPE_3D is not compatible with image");

    // Change VkImageViewCreateInfo to different mismatched viewType
    ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    ivci.subresourceRange.layerCount = 6;

    // Test for error message
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01003");

    // Test mismatch detection for image of type VK_IMAGE_TYPE_3D
    imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               nullptr,
               VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
               VK_IMAGE_TYPE_3D,
               VK_FORMAT_R8G8B8A8_UNORM,
               {1, 1, 1},
               1,
               1,
               VK_SAMPLE_COUNT_1_BIT,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               VK_SHARING_MODE_EXCLUSIVE,
               0,
               nullptr,
               VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image3D(m_device);
    image3D.init(&imgInfo);
    ASSERT_TRUE(image3D.initialized());

    // Initialize VkImageViewCreateInfo with mismatched viewType
    ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image3D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_1D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci,
                        "vkCreateImageView(): pCreateInfo->viewType VK_IMAGE_VIEW_TYPE_1D is not compatible with image");

    // Change VkImageViewCreateInfo to different mismatched viewType
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // Test for error message
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01005");
    } else {
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-subResourceRange-01021");
    }

    // Check if the device can make the image required for this test case.
    VkImageFormatProperties formProps = {{0, 0, 0}, 0, 0, 0, 0};
    VkResult res = vkGetPhysicalDeviceImageFormatProperties(
        m_device->phy().handle(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_3D, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
        &formProps);

    // If not, skip this part of the test.
    if (res || !m_device->phy().features().sparseBinding ||
        !DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        printf("%s %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        return;
    }

    // Initialize VkImageCreateInfo with VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR and VK_IMAGE_CREATE_SPARSE_BINDING_BIT which
    // are incompatible create flags.
    imgInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
        VK_IMAGE_TYPE_3D,
        VK_FORMAT_R8G8B8A8_UNORM,
        {1, 1, 1},
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED};
    VkImage imageSparse;

    // Creating a sparse image means we should not bind memory to it.
    res = vkCreateImage(m_device->device(), &imgInfo, NULL, &imageSparse);
    ASSERT_FALSE(res);

    // Initialize VkImageViewCreateInfo to create a view that will attempt to utilize VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR.
    ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = imageSparse;
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci,
                        " when the VK_IMAGE_CREATE_SPARSE_BINDING_BIT, VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT, or "
                        "VK_IMAGE_CREATE_SPARSE_ALIASED_BIT flags are enabled.");

    // Clean up
    vkDestroyImage(m_device->device(), imageSparse, nullptr);
}

TEST_F(VkLayerTest, CreateImageViewFormatFeatureMismatch) {
    TEST_DESCRIPTION("Create view with a format that does not have the same features as the image format.");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
    }

    // List of features to be tested
    VkFormatFeatureFlagBits features[] = {VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,
                                          VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT};
    uint32_t feature_count = 4;
    // List of usage cases for each feature test
    VkImageUsageFlags usages[] = {VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
    // List of errors that will be thrown in order of tests run
    std::string optimal_error_codes[] = {
        "VUID-VkImageViewCreateInfo-usage-02274",
        "VUID-VkImageViewCreateInfo-usage-02275",
        "VUID-VkImageViewCreateInfo-usage-02276",
        "VUID-VkImageViewCreateInfo-usage-02277",
    };

    VkFormatProperties formatProps;

    // First three tests
    uint32_t i = 0;
    for (i = 0; i < (feature_count - 1); i++) {
        // Modify formats to have mismatched features

        // Format for image
        fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, &formatProps);
        formatProps.optimalTilingFeatures |= features[i];
        fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, formatProps);

        memset(&formatProps, 0, sizeof(formatProps));

        // Format for view
        fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, &formatProps);
        formatProps.optimalTilingFeatures = features[(i + 1) % feature_count];
        fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, formatProps);

        // Create image with modified format
        VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                     nullptr,
                                     VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                     VK_IMAGE_TYPE_2D,
                                     VK_FORMAT_R32G32B32A32_UINT,
                                     {1, 1, 1},
                                     1,
                                     1,
                                     VK_SAMPLE_COUNT_1_BIT,
                                     VK_IMAGE_TILING_OPTIMAL,
                                     usages[i],
                                     VK_SHARING_MODE_EXCLUSIVE,
                                     0,
                                     nullptr,
                                     VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageObj image(m_device);
        image.init(&imgInfo);
        ASSERT_TRUE(image.initialized());

        // Initialize VkImageViewCreateInfo with modified format
        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = image.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = VK_FORMAT_R32G32B32A32_SINT;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        // Test for error message
        CreateImageViewTest(*this, &ivci, optimal_error_codes[i]);
    }

    // Test for VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT.  Needs special formats

    // Only run this test if format supported
    if (!ImageFormatIsSupported(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL)) {
        printf("%s VK_FORMAT_D24_UNORM_S8_UINT format not supported - skipped.\n", kSkipPrefix);
        return;
    }
    // Modify formats to have mismatched features

    // Format for image
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, &formatProps);
    formatProps.optimalTilingFeatures |= features[i];
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D24_UNORM_S8_UINT, formatProps);

    memset(&formatProps, 0, sizeof(formatProps));

    // Format for view
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, &formatProps);
    formatProps.optimalTilingFeatures = features[(i + 1) % feature_count];
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_D32_SFLOAT_S8_UINT, formatProps);

    // Create image with modified format
    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_2D,
                                 VK_FORMAT_D24_UNORM_S8_UINT,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 usages[i],
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image(m_device);
    image.init(&imgInfo);
    ASSERT_TRUE(image.initialized());

    // Initialize VkImageViewCreateInfo with modified format
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

    // Test for error message
    CreateImageViewTest(*this, &ivci, optimal_error_codes[i]);
}

TEST_F(VkLayerTest, InvalidImageViewUsageCreateInfo) {
    TEST_DESCRIPTION("Usage modification via a chained VkImageViewUsageCreateInfo struct");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Test requires DeviceProfileLayer, unavailable - skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Required extensions are not avaiable.\n", kSkipPrefix);
        return;
    }

    VkFormatProperties formatProps;

    // Ensure image format claims support for sampled and storage, excludes color attachment
    memset(&formatProps, 0, sizeof(formatProps));
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, &formatProps);
    formatProps.optimalTilingFeatures |= (VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
    formatProps.optimalTilingFeatures = formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_UINT, formatProps);

    // Create image with sampled and storage usages
    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_2D,
                                 VK_FORMAT_R32G32B32A32_UINT,
                                 {1, 1, 1},
                                 1,
                                 1,
                                 VK_SAMPLE_COUNT_1_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                                 VK_SHARING_MODE_EXCLUSIVE,
                                 0,
                                 nullptr,
                                 VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image(m_device);
    image.init(&imgInfo);
    ASSERT_TRUE(image.initialized());

    // Force the imageview format to exclude storage feature, include color attachment
    memset(&formatProps, 0, sizeof(formatProps));
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    formatProps.optimalTilingFeatures = (formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_R32G32B32A32_SINT, formatProps);

    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R32G32B32A32_SINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // ImageView creation should fail because view format doesn't support all the underlying image's usages
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-usage-02275");

    // Add a chained VkImageViewUsageCreateInfo to override original image usage bits, removing storage
    VkImageViewUsageCreateInfo usage_ci = {VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO, nullptr, VK_IMAGE_USAGE_SAMPLED_BIT};
    // Link the VkImageViewUsageCreateInfo struct into the view's create info pNext chain
    ivci.pNext = &usage_ci;

    // ImageView should now succeed without error
    CreateImageViewTest(*this, &ivci);

    // Try a zero usage field
    usage_ci.usage = 0;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewUsageCreateInfo-usage-requiredbitmask");

    // Try an illegal bit in usage field
    usage_ci.usage = 0x10000000 | VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewUsageCreateInfo-usage-parameter");
}

TEST_F(VkLayerTest, CreateImageViewNoMemoryBoundToImage) {
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());

    // Create an image and try to create a view with no memory backing the image
    VkImage image;

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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

    err = vkCreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    CreateImageViewTest(*this, &image_view_create_info,
                        " used with no memory bound. Memory should be bound by calling vkBindImageMemory().");
    vkDestroyImage(m_device->device(), image, NULL);
}

TEST_F(VkLayerTest, InvalidImageViewAspect) {
    TEST_DESCRIPTION("Create an image and try to create a view with an invalid aspectMask");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.layerCount = 1;
    // Cause an error by setting an invalid image aspect
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageSubresource-aspectMask-parameter");
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExerciseGetImageSubresourceLayout) {
    TEST_DESCRIPTION("Test vkGetImageSubresourceLayout() valid usages");

    ASSERT_NO_FATAL_FAILURE(Init());
    VkSubresourceLayout subres_layout = {};

    // VU 00732: image must have been created with tiling equal to VK_IMAGE_TILING_LINEAR
    {
        const VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;  // ERROR: violates VU 00732
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, tiling);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 0;
        subres.arrayLayer = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageSubresourceLayout-image-00996");
        vkGetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }

    // VU 00733: The aspectMask member of pSubresource must only have a single bit set
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_METADATA_BIT;  // ERROR: triggers VU 00733
        subres.mipLevel = 0;
        subres.arrayLayer = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageSubresourceLayout-aspectMask-00997");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageSubresource-aspectMask-parameter");
        vkGetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }

    // 00739 mipLevel must be less than the mipLevels specified in VkImageCreateInfo when the image was created
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 1;  // ERROR: triggers VU 00739
        subres.arrayLayer = 0;

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageSubresourceLayout-mipLevel-01716");
        vkGetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }

    // 00740 arrayLayer must be less than the arrayLayers specified in VkImageCreateInfo when the image was created
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 0;
        subres.arrayLayer = 1;  // ERROR: triggers VU 00740

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetImageSubresourceLayout-arrayLayer-01717");
        vkGetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ImageLayerUnsupportedFormat) {
    TEST_DESCRIPTION("Creating images with unsupported formats ");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create image with unsupported format - Expect FORMAT_UNSUPPORTED
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_UNDEFINED;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-00943");
}

TEST_F(VkLayerTest, CreateImageViewFormatMismatchUnrelated) {
    TEST_DESCRIPTION("Create an image with a color format, then try to create a depth view of it");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Load required functions
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT =
        (PFN_vkSetPhysicalDeviceFormatPropertiesEXT)vkGetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFormatPropertiesEXT");
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT =
        (PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT)vkGetInstanceProcAddr(instance(),
                                                                                  "vkGetOriginalPhysicalDeviceFormatPropertiesEXT");

    if (!(fpvkSetPhysicalDeviceFormatPropertiesEXT) || !(fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
    }

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s Couldn't find depth stencil image format.\n", kSkipPrefix);
        return;
    }

    VkFormatProperties formatProps;

    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), depth_format, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), depth_format, formatProps);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.image = image.handle();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = depth_format;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Can't use depth format for view into color image - Expect INVALID_FORMAT
    CreateImageViewTest(*this, &imgViewInfo,
                        "Formats MUST be IDENTICAL unless VK_IMAGE_CREATE_MUTABLE_FORMAT BIT was set on image creation.");
}

TEST_F(VkLayerTest, CreateImageViewNoMutableFormatBit) {
    TEST_DESCRIPTION("Create an image view with a different format, when the image does not have MUTABLE_FORMAT bit");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Couldn't enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Required extensions are not present.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkFormatProperties formatProps;

    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_B8G8R8A8_UINT, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), VK_FORMAT_B8G8R8A8_UINT, formatProps);

    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.image = image.handle();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UINT;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Same compatibility class but no MUTABLE_FORMAT bit - Expect
    // VIEW_CREATE_ERROR
    CreateImageViewTest(*this, &imgViewInfo, "VUID-VkImageViewCreateInfo-image-01019");
}

TEST_F(VkLayerTest, CreateImageViewDifferentClass) {
    TEST_DESCRIPTION("Passing bad parameters to CreateImageView");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!(m_device->format_properties(VK_FORMAT_R8_UINT).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
        printf("%s Device does not support R8_UINT as color attachment; skipped", kSkipPrefix);
        return;
    }

    VkImageCreateInfo mutImgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                    nullptr,
                                    VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                    VK_IMAGE_TYPE_2D,
                                    VK_FORMAT_R8_UINT,
                                    {128, 128, 1},
                                    1,
                                    1,
                                    VK_SAMPLE_COUNT_1_BIT,
                                    VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                    VK_SHARING_MODE_EXCLUSIVE,
                                    0,
                                    nullptr,
                                    VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj mutImage(m_device);
    mutImage.init(&mutImgInfo);
    ASSERT_TRUE(mutImage.initialized());

    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewInfo.image = mutImage.handle();

    CreateImageViewTest(*this, &imgViewInfo, "VUID-VkImageViewCreateInfo-image-01018");
}

TEST_F(VkLayerTest, MultiplaneIncompatibleViewFormat) {
    TEST_DESCRIPTION("Postive/negative tests of multiplane imageview format compatibility");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ci.pNext = NULL;
    ci.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify format
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }

    VkImageObj image_obj(m_device);
    image_obj.init(&ci);
    ASSERT_TRUE(image_obj.initialized());

    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image_obj.image();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8_SNORM;  // Compat is VK_FORMAT_R8_UNORM
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;

    // Incompatible format error
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01586");

    // Correct format succeeds
    ivci.format = VK_FORMAT_R8_UNORM;
    CreateImageViewTest(*this, &ivci);

    // Try a multiplane imageview
    ivci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    CreateImageViewTest(*this, &ivci);
}

TEST_F(VkLayerTest, CreateImageViewInvalidSubresourceRange) {
    TEST_DESCRIPTION("Passing bad image subrange to CreateImageView");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo img_view_info_template = {};
    img_view_info_template.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    img_view_info_template.image = image.handle();
    img_view_info_template.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    img_view_info_template.format = image.format();
    // subresourceRange to be filled later for the purposes of this test
    img_view_info_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_view_info_template.subresourceRange.baseMipLevel = 0;
    img_view_info_template.subresourceRange.levelCount = 0;
    img_view_info_template.subresourceRange.baseArrayLayer = 0;
    img_view_info_template.subresourceRange.layerCount = 0;

    // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01478");
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01478");
    }

    // Try levelCount = 0
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
    }

    // These tests rely on having the Maintenance1 extension not being enabled, and are invalid on all but version 1.0
    if (m_device->props.apiVersion < VK_API_VERSION_1_1) {
        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            VkImageViewCreateInfo img_view_info = img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01480");
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            VkImageViewCreateInfo img_view_info = img_view_info_template;
            img_view_info.subresourceRange = range;
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                                 "VUID-VkImageViewCreateInfo-subresourceRange-01719");
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01480");
        }

        // Try layerCount = 0
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            VkImageViewCreateInfo img_view_info = img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01719");
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            VkImageViewCreateInfo img_view_info = img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01719");
        }
    }
}

TEST_F(VkLayerTest, CreateImageMiscErrors) {
    TEST_DESCRIPTION("Misc leftover valid usage errors in VkImageCreateInfo struct");

    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));

    VkImageCreateInfo tmp_img_ci = {};
    tmp_img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    tmp_img_ci.flags = 0;                          // assumably any is supported
    tmp_img_ci.imageType = VK_IMAGE_TYPE_2D;       // any is supported
    tmp_img_ci.format = VK_FORMAT_R8G8B8A8_UNORM;  // has mandatory support for all usages
    tmp_img_ci.extent = {64, 64, 1};               // limit is 256 for 3D, or 4096
    tmp_img_ci.mipLevels = 1;                      // any is supported
    tmp_img_ci.arrayLayers = 1;                    // limit is 256
    tmp_img_ci.samples = VK_SAMPLE_COUNT_1_BIT;    // needs to be 1 if TILING_LINEAR
    // if VK_IMAGE_TILING_LINEAR imageType must be 2D, usage must be TRANSFER, and levels layers samplers all 1
    tmp_img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    tmp_img_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // depends on format
    tmp_img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    const VkImageCreateInfo safe_image_ci = tmp_img_ci;

    ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &safe_image_ci));

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.sharingMode = VK_SHARING_MODE_CONCURRENT;
        image_ci.queueFamilyIndexCount = 2;
        image_ci.pQueueFamilyIndices = nullptr;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-sharingMode-00941");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.sharingMode = VK_SHARING_MODE_CONCURRENT;
        image_ci.queueFamilyIndexCount = 1;
        const uint32_t queue_family = 0;
        image_ci.pQueueFamilyIndices = &queue_family;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-sharingMode-00942");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.format = VK_FORMAT_UNDEFINED;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-format-00943");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.arrayLayers = 6;
        image_ci.imageType = VK_IMAGE_TYPE_1D;
        m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageType-00954");
        image_ci.extent = {64, 1, 1};
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-00949");

        image_ci = safe_image_ci;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageType-00954");
        image_ci.extent = {4, 4, 4};
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-00949");

        image_ci = safe_image_ci;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.extent = {8, 6, 1};
        image_ci.arrayLayers = 6;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-imageType-00954");

        image_ci = safe_image_ci;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.extent = {8, 8, 1};
        image_ci.arrayLayers = 4;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-imageType-00954");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.extent = {4, 4, 4};
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.arrayLayers = 6;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.tiling = VK_IMAGE_TILING_LINEAR;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // always has 4 samples support
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.mipLevels = 2;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00963");

        image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00966");

        image_ci.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        image_ci.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-usage-00963");
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00966");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-00969");
    }

    // InitialLayout not VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREDEFINED
    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-initialLayout-00993");
    }
}

TEST_F(VkLayerTest, CreateImageMinLimitsViolation) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters violation minimum limit, such as being zero.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkImage null_image;  // throwaway target for all the vkCreateImage

    VkImageCreateInfo tmp_img_ci = {};
    tmp_img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    tmp_img_ci.flags = 0;                          // assumably any is supported
    tmp_img_ci.imageType = VK_IMAGE_TYPE_2D;       // any is supported
    tmp_img_ci.format = VK_FORMAT_R8G8B8A8_UNORM;  // has mandatory support for all usages
    tmp_img_ci.extent = {1, 1, 1};                 // limit is 256 for 3D, or 4096
    tmp_img_ci.mipLevels = 1;                      // any is supported
    tmp_img_ci.arrayLayers = 1;                    // limit is 256
    tmp_img_ci.samples = VK_SAMPLE_COUNT_1_BIT;    // needs to be 1 if TILING_LINEAR
    // if VK_IMAGE_TILING_LINEAR imageType must be 2D, usage must be TRANSFER, and levels layers samplers all 1
    tmp_img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    tmp_img_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // depends on format
    tmp_img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    const VkImageCreateInfo safe_image_ci = tmp_img_ci;

    enum Dimension { kWidth = 0x1, kHeight = 0x2, kDepth = 0x4 };

    for (underlying_type<Dimension>::type bad_dimensions = 0x1; bad_dimensions < 0x8; ++bad_dimensions) {
        VkExtent3D extent = {1, 1, 1};

        if (bad_dimensions & kWidth) {
            extent.width = 0;
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-extent-00944");
        }

        if (bad_dimensions & kHeight) {
            extent.height = 0;
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-extent-00945");
        }

        if (bad_dimensions & kDepth) {
            extent.depth = 0;
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-extent-00946");
        }

        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.imageType = VK_IMAGE_TYPE_3D;  // has to be 3D otherwise it might trigger the non-1 error instead
        bad_image_ci.extent = extent;

        vkCreateImage(m_device->device(), &bad_image_ci, NULL, &null_image);

        m_errorMonitor->VerifyFound();
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.mipLevels = 0;
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-mipLevels-00947");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.arrayLayers = 0;
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-arrayLayers-00948");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        bad_image_ci.arrayLayers = 5;
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00954");

        bad_image_ci.arrayLayers = 6;
        bad_image_ci.extent = {64, 63, 1};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00954");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.imageType = VK_IMAGE_TYPE_1D;
        bad_image_ci.extent = {64, 2, 1};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00956");

        bad_image_ci.imageType = VK_IMAGE_TYPE_1D;
        bad_image_ci.extent = {64, 1, 2};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00956");

        bad_image_ci.imageType = VK_IMAGE_TYPE_2D;
        bad_image_ci.extent = {64, 64, 2};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00957");

        bad_image_ci.imageType = VK_IMAGE_TYPE_2D;
        bad_image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        bad_image_ci.arrayLayers = 6;
        bad_image_ci.extent = {64, 64, 2};
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00957");
    }

    {
        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.imageType = VK_IMAGE_TYPE_3D;
        bad_image_ci.arrayLayers = 2;
        CreateImageTest(*this, &bad_image_ci, "VUID-VkImageCreateInfo-imageType-00961");
    }
}

TEST_F(VkLayerTest, CreateImageMaxLimitsViolation) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters exceeding physical device limits.");

    // Check for VK_KHR_get_physical_device_properties2
    bool push_physical_device_properties_2_support =
        InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (push_physical_device_properties_2_support) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    bool push_fragment_density_support = false;

    if (push_physical_device_properties_2_support) {
        push_fragment_density_support = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
        if (push_fragment_density_support) m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, 0));

    VkImageCreateInfo tmp_img_ci = {};
    tmp_img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    tmp_img_ci.flags = 0;                          // assumably any is supported
    tmp_img_ci.imageType = VK_IMAGE_TYPE_2D;       // any is supported
    tmp_img_ci.format = VK_FORMAT_R8G8B8A8_UNORM;  // has mandatory support for all usages
    tmp_img_ci.extent = {1, 1, 1};                 // limit is 256 for 3D, or 4096
    tmp_img_ci.mipLevels = 1;                      // any is supported
    tmp_img_ci.arrayLayers = 1;                    // limit is 256
    tmp_img_ci.samples = VK_SAMPLE_COUNT_1_BIT;    // needs to be 1 if TILING_LINEAR
    // if VK_IMAGE_TILING_LINEAR imageType must be 2D, usage must be TRANSFER, and levels layers samplers all 1
    tmp_img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    tmp_img_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // depends on format
    tmp_img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    const VkImageCreateInfo safe_image_ci = tmp_img_ci;

    ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &safe_image_ci));

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.extent = {8, 8, 1};
        image_ci.mipLevels = 4 + 1;  // 4 = log2(8) + 1
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-mipLevels-00958");

        image_ci.extent = {8, 15, 1};
        image_ci.mipLevels = 4 + 1;  // 4 = floor(log2(15)) + 1
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-mipLevels-00958");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.tiling = VK_IMAGE_TILING_LINEAR;
        image_ci.extent = {64, 64, 1};
        image_ci.format = FindFormatLinearWithoutMips(gpu(), image_ci);
        image_ci.mipLevels = 2;

        if (image_ci.format != VK_FORMAT_UNDEFINED) {
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-mipLevels-02255");
        } else {
            printf("%s Cannot find a format to test maxMipLevels limit; skipping part of test.\n", kSkipPrefix);
        }
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;

        VkImageFormatProperties img_limits;
        ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));

        if (img_limits.maxArrayLayers != UINT32_MAX) {
            image_ci.arrayLayers = img_limits.maxArrayLayers + 1;
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-arrayLayers-02256");
        } else {
            printf("%s VkImageFormatProperties::maxArrayLayers is already UINT32_MAX; skipping part of test.\n", kSkipPrefix);
        }
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        bool found = FindFormatWithoutSamples(gpu(), image_ci);

        if (found) {
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02258");
        } else {
            printf("%s Could not find a format with some unsupported samples; skipping part of test.\n", kSkipPrefix);
        }
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // (any attachment bit)

        VkImageFormatProperties img_limits;
        ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));

        if (dev_limits.maxFramebufferWidth != UINT32_MAX) {
            image_ci.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00964");
        } else {
            printf("%s VkPhysicalDeviceLimits::maxFramebufferWidth is already UINT32_MAX; skipping part of test.\n", kSkipPrefix);
        }

        if (dev_limits.maxFramebufferHeight != UINT32_MAX) {
            image_ci.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;  // try different one too
            image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00965");
        } else {
            printf("%s VkPhysicalDeviceLimits::maxFramebufferHeight is already UINT32_MAX; skipping part of test.\n", kSkipPrefix);
        }
    }

    {
        if (!push_fragment_density_support) {
            printf("%s VK_EXT_fragment_density_map Extension not supported, skipping tests\n", kSkipPrefix);
        } else {
            VkImageCreateInfo image_ci = safe_image_ci;
            image_ci.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
            VkImageFormatProperties img_limits;
            ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));

            image_ci.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-02559");

            image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-02560");
        }
    }
}

TEST_F(VkLayerTest, MultiplaneImageSamplerConversionMismatch) {
    TEST_DESCRIPTION(
        "Create sampler with ycbcr conversion and use with an image created without ycrcb conversion or immutable sampler");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    // Enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = {};
    ycbcr_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
    ycbcr_features.samplerYcbcrConversion = VK_TRUE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &ycbcr_features));

    PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionFunction = nullptr;
    PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionFunction = nullptr;

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkCreateSamplerYcbcrConversionFunction = vkCreateSamplerYcbcrConversion;
        vkDestroySamplerYcbcrConversionFunction = vkDestroySamplerYcbcrConversion;
    } else {
        vkCreateSamplerYcbcrConversionFunction =
            (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
        vkDestroySamplerYcbcrConversionFunction =
            (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetDeviceProcAddr(m_device->handle(), "vkDestroySamplerYcbcrConversionKHR");
    }

    if (!vkCreateSamplerYcbcrConversionFunction || !vkDestroySamplerYcbcrConversionFunction) {
        printf("%s Did not find required device extension %s; test skipped.\n", kSkipPrefix,
               VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkImageCreateInfo ci = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                  NULL,
                                  0,
                                  VK_IMAGE_TYPE_2D,
                                  VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
                                  {128, 128, 1},
                                  1,
                                  1,
                                  VK_SAMPLE_COUNT_1_BIT,
                                  VK_IMAGE_TILING_LINEAR,
                                  VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_SHARING_MODE_EXCLUSIVE,
                                  VK_IMAGE_LAYOUT_UNDEFINED};

    // Verify formats
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }

    // Create Ycbcr conversion
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
                                                            NULL,
                                                            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
                                                            VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY,
                                                            VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_FILTER_NEAREST,
                                                            false};
    VkSamplerYcbcrConversion conversions[2];
    vkCreateSamplerYcbcrConversionFunction(m_device->handle(), &ycbcr_create_info, nullptr, &conversions[0]);
    ycbcr_create_info.components.r = VK_COMPONENT_SWIZZLE_ZERO;  // Just anything different than above
    vkCreateSamplerYcbcrConversionFunction(m_device->handle(), &ycbcr_create_info, nullptr, &conversions[1]);

    VkSamplerYcbcrConversionInfo ycbcr_info = {};
    ycbcr_info.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
    ycbcr_info.conversion = conversions[0];

    // Create a sampler using conversion
    VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo();
    sci.pNext = &ycbcr_info;
    // Create two samplers with two different conversions, such that one will mismatch
    // It will make the second sampler fail to see if the log prints the second sampler or the first sampler.
    VkSampler samplers[2];
    VkResult err = vkCreateSampler(m_device->device(), &sci, NULL, &samplers[0]);
    ASSERT_VK_SUCCESS(err);
    ycbcr_info.conversion = conversions[1];  // Need two samplers with different conversions
    err = vkCreateSampler(m_device->device(), &sci, NULL, &samplers[1]);
    ASSERT_VK_SUCCESS(err);

    // Create an image without a Ycbcr conversion
    VkImageObj mpimage(m_device);
    mpimage.init(&ci);

    VkImageView view;
    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ycbcr_info.conversion = conversions[0];  // Need two samplers with different conversions
    ivci.pNext = &ycbcr_info;
    ivci.image = mpimage.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkCreateImageView(m_device->device(), &ivci, nullptr, &view);

    // Use the image and sampler together in a descriptor set
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, samplers},
                                       });

    // Use the same image view twice, using the same sampler, with the *second* mismatched with the *second* immutable sampler
    VkDescriptorImageInfo image_infos[2];
    image_infos[0] = {};
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[0].imageView = view;
    image_infos[0].sampler = samplers[0];
    image_infos[1] = image_infos[0];

    // Update the descriptor set expecting to get an error
    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 2;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = image_infos;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-01948");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // pImmutableSamplers = nullptr causes an error , VUID-VkWriteDescriptorSet-descriptorType-02738.
    // Because if pNext chains a VkSamplerYcbcrConversionInfo, the sampler has to be a immutable sampler.
    OneOffDescriptorSet descriptor_set_1947(m_device,
                                            {
                                                {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                            });
    descriptor_write.dstSet = descriptor_set_1947.set_;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &image_infos[0];
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkWriteDescriptorSet-descriptorType-02738");
    vkUpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    vkDestroySamplerYcbcrConversionFunction(m_device->device(), conversions[0], nullptr);
    vkDestroySamplerYcbcrConversionFunction(m_device->device(), conversions[1], nullptr);
    vkDestroyImageView(m_device->device(), view, NULL);
    vkDestroySampler(m_device->device(), samplers[0], nullptr);
    vkDestroySampler(m_device->device(), samplers[1], nullptr);
}

TEST_F(VkLayerTest, DepthStencilImageViewWithColorAspectBitError) {
    // Create a single Image descriptor and cause it to first hit an error due
    //  to using a DS format, then cause it to hit error due to COLOR_BIT not
    //  set in aspect
    // The image format check comes 2nd in validation so we trigger it first,
    //  then when we cause aspect fail next, bad format check will be preempted

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "Combination depth/stencil image formats can have only the ");

    ASSERT_NO_FATAL_FAILURE(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s Couldn't find depth stencil format.\n", kSkipPrefix);
        return;
    }

    VkImageObj image_bad(m_device);
    VkImageObj image_good(m_device);
    // One bad format and one good format for Color attachment
    const VkFormat tex_format_bad = depth_format;
    const VkFormat tex_format_good = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 32;
    const int32_t tex_height = 32;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format_bad;
    image_create_info.extent.width = tex_width;
    image_create_info.extent.height = tex_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    image_bad.init(&image_create_info);

    image_create_info.format = tex_format_good;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_good.init(&image_create_info);

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image_bad.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format_bad;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

    VkImageView view;
    vkCreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ExtensionNotEnabled) {
    TEST_DESCRIPTION("Validate that using an API from an unenabled extension returns an error");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // Required extensions except VK_KHR_GET_MEMORY_REQUIREMENTS_2 -- to create the needed error
    std::vector<const char *> required_device_extensions = {VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
                                                            VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME};
    for (auto dev_ext : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, dev_ext)) {
            m_device_extension_names.push_back(dev_ext);
        } else {
            printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix, dev_ext);
            break;
        }
    }

    // Need to ignore this error to get to the one we're testing
    m_errorMonitor->SetUnexpectedError("VUID-vkCreateDevice-ppEnabledExtensionNames-01387");
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Find address of extension API
    auto vkCreateSamplerYcbcrConversionKHR =
        (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
    if (vkCreateSamplerYcbcrConversionKHR == nullptr) {
        printf("%s VK_KHR_sampler_ycbcr_conversion not supported by device; skipped.\n", kSkipPrefix);
        return;
    }
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-GeneralParameterError-ExtensionNotEnabled");
    VkSamplerYcbcrConversionCreateInfo ycbcr_info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
                                                     NULL,
                                                     VK_FORMAT_UNDEFINED,
                                                     VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY,
                                                     VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                     {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                      VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                                     VK_CHROMA_LOCATION_COSITED_EVEN,
                                                     VK_CHROMA_LOCATION_COSITED_EVEN,
                                                     VK_FILTER_NEAREST,
                                                     false};
    VkSamplerYcbcrConversion conversion;
    vkCreateSamplerYcbcrConversionKHR(m_device->handle(), &ycbcr_info, nullptr, &conversion);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCreateBufferSize) {
    TEST_DESCRIPTION("Attempt to create VkBuffer with size of zero");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    info.size = 0;
    CreateBufferTest(*this, &info, "VUID-VkBufferCreateInfo-size-00912");
}

TEST_F(VkLayerTest, DuplicateValidPNextStructures) {
    TEST_DESCRIPTION("Create a pNext chain containing valid structures, but with a duplicate structure type");

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME);
    } else {
        printf("%s VK_NV_dedicated_allocation extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Create two pNext structures which by themselves would be valid
    VkDedicatedAllocationBufferCreateInfoNV dedicated_buffer_create_info = {};
    VkDedicatedAllocationBufferCreateInfoNV dedicated_buffer_create_info_2 = {};
    dedicated_buffer_create_info.sType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV;
    dedicated_buffer_create_info.pNext = &dedicated_buffer_create_info_2;
    dedicated_buffer_create_info.dedicatedAllocation = VK_TRUE;

    dedicated_buffer_create_info_2.sType = VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV;
    dedicated_buffer_create_info_2.pNext = nullptr;
    dedicated_buffer_create_info_2.dedicatedAllocation = VK_TRUE;

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = &dedicated_buffer_create_info;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    CreateBufferTest(*this, &buffer_create_info, "chain contains duplicate structure types");
}

TEST_F(VkLayerTest, DedicatedAllocation) {
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Dedicated allocation extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize resource_size = 1024;
    auto buffer_info = VkBufferObj::create_info(resource_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_info);
    auto buffer_alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, buffer.memory_requirements(), mem_flags);
    auto buffer_dedicated_info = lvl_init_struct<VkMemoryDedicatedAllocateInfoKHR>();
    buffer_dedicated_info.buffer = buffer.handle();
    buffer_alloc_info.pNext = &buffer_dedicated_info;
    vk_testing::DeviceMemory dedicated_buffer_memory;
    dedicated_buffer_memory.init(*m_device, buffer_alloc_info);

    VkBufferObj wrong_buffer;
    wrong_buffer.init_no_mem(*m_device, buffer_info);

    // Bind with wrong buffer
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindBufferMemory-memory-01508");
    vkBindBufferMemory(m_device->handle(), wrong_buffer.handle(), dedicated_buffer_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset (same VUID)
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkBindBufferMemory-memory-01508");  // offset must be zero
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkBindBufferMemory-size-01037");  // offset pushes us past size
    auto offset = buffer.memory_requirements().alignment;
    vkBindBufferMemory(m_device->handle(), buffer.handle(), dedicated_buffer_memory.handle(), offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    m_errorMonitor->ExpectSuccess();
    vkBindBufferMemory(m_device->handle(), buffer.handle(), dedicated_buffer_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();

    // And for images...
    VkImageObj image(m_device);
    VkImageObj wrong_image(m_device);
    auto image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image.init_no_mem(*m_device, image_info);
    wrong_image.init_no_mem(*m_device, image_info);

    auto image_dedicated_info = lvl_init_struct<VkMemoryDedicatedAllocateInfoKHR>();
    image_dedicated_info.image = image.handle();
    auto image_alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vk_testing::DeviceMemory dedicated_image_memory;
    dedicated_image_memory.init(*m_device, image_alloc_info);

    // Bind with wrong image
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkBindImageMemory-memory-01509");
    vkBindImageMemory(m_device->handle(), wrong_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset (same VUID)
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkBindImageMemory-memory-01509");  // offset must be zero
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-vkBindImageMemory-size-01049");  // offset pushes us past size
    auto image_offset = image.memory_requirements().alignment;
    vkBindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), image_offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    m_errorMonitor->ExpectSuccess();
    vkBindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, CornerSampledImageNV) {
    TEST_DESCRIPTION("Test VK_NV_corner_sampled_image.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables exclusive scissor but disables multiViewport
    auto corner_sampled_image_features = lvl_init_struct<VkPhysicalDeviceCornerSampledImageFeaturesNV>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&corner_sampled_image_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 2;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;

    // image type must be 2D or 3D
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02050");

    // cube/depth not supported
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 2;
    image_create_info.format = VK_FORMAT_D24_UNORM_S8_UINT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02051");

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;

    // 2D width/height must be > 1
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 1;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02052");

    // 3D width/height/depth must be > 1
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent.height = 2;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02053");

    image_create_info.imageType = VK_IMAGE_TYPE_2D;

    // Valid # of mip levels
    image_create_info.extent = {7, 7, 1};
    image_create_info.mipLevels = 3;  // 3 = ceil(log2(7))
    CreateImageTest(*this, &image_create_info);

    image_create_info.extent = {8, 8, 1};
    image_create_info.mipLevels = 3;  // 3 = ceil(log2(8))
    CreateImageTest(*this, &image_create_info);

    image_create_info.extent = {9, 9, 1};
    image_create_info.mipLevels = 3;  // 4 = ceil(log2(9))
    CreateImageTest(*this, &image_create_info);

    // Invalid # of mip levels
    image_create_info.extent = {8, 8, 1};
    image_create_info.mipLevels = 4;  // 3 = ceil(log2(8))
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-mipLevels-00958");
}

TEST_F(VkLayerTest, CreateYCbCrSampler) {
    TEST_DESCRIPTION("Verify YCbCr sampler creation.");

    // Test requires API 1.1 or (API 1.0 + SamplerYCbCr extension). Request API 1.1
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    // In case we don't have API 1.1+, try enabling the extension directly (and it's dependencies)
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    VkDevice dev = m_device->device();

    PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionFunction = nullptr;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkCreateSamplerYcbcrConversionFunction = vkCreateSamplerYcbcrConversion;
    } else {
        vkCreateSamplerYcbcrConversionFunction =
            (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
    }

    if (!vkCreateSamplerYcbcrConversionFunction) {
        printf("%s Did not find required device support for YcbcrSamplerConversion; test skipped.\n", kSkipPrefix);
        return;
    }

    // Verify we have the requested support
    bool ycbcr_support = (DeviceExtensionEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) ||
                          (DeviceValidationVersion() >= VK_API_VERSION_1_1));
    if (!ycbcr_support) {
        printf("%s Did not find required device extension %s; test skipped.\n", kSkipPrefix,
               VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        return;
    }

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = {};
    sycci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01649");
    vkCreateSamplerYcbcrConversionFunction(dev, &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BufferDeviceAddressEXT) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (DeviceIsMockICD() || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables buffer_device_address
    auto buffer_device_address_features = lvl_init_struct<VkPhysicalDeviceBufferAddressFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressEXT vkGetBufferDeviceAddressEXT =
        (PFN_vkGetBufferDeviceAddressEXT)vkGetInstanceProcAddr(instance(), "vkGetBufferDeviceAddressEXT");

    VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-02605");

    buffer_create_info.flags = 0;
    VkBufferDeviceAddressCreateInfoEXT addr_ci = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT};
    addr_ci.deviceAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-deviceAddress-02604");

    buffer_create_info.pNext = nullptr;
    VkBuffer buffer;
    VkResult err = vkCreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoEXT info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT};
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBufferDeviceAddressInfoEXT-buffer-02600");
    vkGetBufferDeviceAddressEXT(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    vkDestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, BufferDeviceAddressEXTDisabled) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (DeviceIsMockICD() || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that disables buffer_device_address
    auto buffer_device_address_features = lvl_init_struct<VkPhysicalDeviceBufferAddressFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressEXT vkGetBufferDeviceAddressEXT =
        (PFN_vkGetBufferDeviceAddressEXT)vkGetInstanceProcAddr(instance(), "vkGetBufferDeviceAddressEXT");

    VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-usage-02606");

    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBuffer buffer;
    VkResult err = vkCreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoEXT info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT};
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkGetBufferDeviceAddressEXT-None-02598");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBufferDeviceAddressInfoEXT-buffer-02601");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBufferDeviceAddressInfoEXT-buffer-02600");
    vkGetBufferDeviceAddressEXT(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    vkDestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, CreateImageYcbcrArrayLayers) {
    TEST_DESCRIPTION("Creating images with out-of-range arrayLayers ");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create ycbcr image with unsupported arrayLayers
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), image_create_info, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }

    VkImageFormatProperties img_limits;
    ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_create_info, &img_limits));
    if (img_limits.maxArrayLayers == 1) {
        return;
    }
    image_create_info.arrayLayers = img_limits.maxArrayLayers;

    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02653");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkImageCreateInfo-format-02653");
}

TEST_F(VkLayerTest, BindImageMemorySwapchain) {
    TEST_DESCRIPTION("Invalid bind image with a swapchain");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping BindSwapchainImageMemory test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping BindSwapchainImageMemory test\n", kSkipPrefix);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s VkBindImageMemoryInfo requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        printf("%s Cannot create surface or swapchain, skipping BindSwapchainImageMemory test\n", kSkipPrefix);
        return;
    }

    auto image_create_info = lvl_init_struct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto image_swapchain_create_info = lvl_init_struct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    VkImage image_from_swapchain;
    vkCreateImage(device(), &image_create_info, NULL, &image_from_swapchain);

    VkMemoryRequirements mem_reqs = {};
    vkGetImageMemoryRequirements(device(), image_from_swapchain, &mem_reqs);

    auto alloc_info = lvl_init_struct<VkMemoryAllocateInfo>();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = mem_reqs.size;

    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    VkDeviceMemory mem;
    VkResult err = vkAllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    auto bind_info = lvl_init_struct<VkBindImageMemoryInfo>();
    bind_info.image = image_from_swapchain;
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBindImageMemoryInfo-image-01630");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBindImageMemoryInfo-pNext-01632");
    vkBindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    auto bind_swapchain_info = lvl_init_struct<VkBindImageMemorySwapchainInfoKHR>();
    bind_swapchain_info.swapchain = VK_NULL_HANDLE;
    bind_swapchain_info.imageIndex = 0;
    bind_info.pNext = &bind_swapchain_info;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-GeneralParameterError-RequiredParameter");
    vkBindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    bind_info.memory = mem;
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = UINT32_MAX;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBindImageMemoryInfo-pNext-01631");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkBindImageMemorySwapchainInfoKHR-imageIndex-01644");
    vkBindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    vkDestroyImage(m_device->device(), image_from_swapchain, NULL);
    vkFreeMemory(m_device->device(), mem, NULL);
    DestroySwapchain();
}

TEST_F(VkLayerTest, TransferImageToSwapchainWithInvalidLayoutDeviceGroup) {
    TEST_DESCRIPTION("Transfer an image to a swapchain's image with a invalid layout between device group");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    printf(
        "%s According to VUID-01631, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is NULL, "
        "skipping test\n",
        kSkipPrefix);
    return;
#endif

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(myDbgFunc, m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s VkBindImageMemoryInfo requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    uint32_t physical_device_group_count = 0;
    vkEnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        printf("%s physical_device_group_count is 0, skipping test\n", kSkipPrefix);
        return;
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vkEnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = {};
    create_device_pnext.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &create_device_pnext));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain(VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        printf("%s Cannot create surface or swapchain, skipping test\n", kSkipPrefix);
        return;
    }

    auto image_create_info = lvl_init_struct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj src_Image(m_device);
    src_Image.init(&image_create_info);

    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_ALIAS_BIT;

    auto image_swapchain_create_info = lvl_init_struct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;
    image_create_info.pNext = &image_swapchain_create_info;

    VkImage peer_image;
    vkCreateImage(device(), &image_create_info, NULL, &peer_image);

    auto bind_devicegroup_info = lvl_init_struct<VkBindImageMemoryDeviceGroupInfo>();
    bind_devicegroup_info.deviceIndexCount = 2;
    std::array<uint32_t, 2> deviceIndices = {0, 0};
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    auto bind_swapchain_info = lvl_init_struct<VkBindImageMemorySwapchainInfoKHR>(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    auto bind_info = lvl_init_struct<VkBindImageMemoryInfo>(&bind_swapchain_info);
    bind_info.image = peer_image;
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vkBindImageMemory2(m_device->device(), 1, &bind_info);

    uint32_t swapchain_images_count = 0;
    vkGetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vkGetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

    m_commandBuffer->begin();

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.dstSubresource.mipLevel = 0;
    copy_region.srcSubresource.baseArrayLayer = 0;
    copy_region.dstSubresource.baseArrayLayer = 0;
    copy_region.srcSubresource.layerCount = 1;
    copy_region.dstSubresource.layerCount = 1;
    copy_region.srcOffset = {0, 0, 0};
    copy_region.dstOffset = {0, 0, 0};
    copy_region.extent = {10, 10, 1};
    vkCmdCopyImage(m_commandBuffer->handle(), src_Image.handle(), VK_IMAGE_LAYOUT_GENERAL, peer_image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vkQueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vkDestroyImage(m_device->device(), peer_image, NULL);
    DestroySwapchain();
}
