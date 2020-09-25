/*
 * Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2020 Valve Corporation
 * Copyright (c) 2015-2020 LunarG, Inc.
 * Copyright (c) 2015-2020 Google, Inc.
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

#include <type_traits>

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, BufferExtents) {
    TEST_DESCRIPTION("Perform copies across a buffer, provoking out-of-range errors.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkDeviceSize buffer_size = 2048;

    VkBufferObj buffer_one;
    VkBufferObj buffer_two;
    VkMemoryPropertyFlags reqs = 0;
    buffer_one.init_as_src_and_dst(*m_device, buffer_size, reqs);
    buffer_two.init_as_src_and_dst(*m_device, buffer_size, reqs);

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-srcOffset-00113");
    VkBufferCopy copy_info = {4096, 256, 256};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-dstOffset-00114");
    copy_info = {256, 4096, 256};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-size-00115");
    copy_info = {1024, 256, 1280};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-size-00116");
    copy_info = {256, 1024, 1280};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-pRegions-00117");
    copy_info = {256, 512, 512};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_two.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCopy-size-01988");
    copy_info = {256, 256, 0};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_two.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, MirrorClampToEdgeNotEnabled) {
    TEST_DESCRIPTION("Validation should catch using CLAMP_TO_EDGE addressing mode if the extension is not enabled.");

    ASSERT_NO_FATAL_FAILURE(Init());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01079");
    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    // Set the modes to cause the error
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AnisotropyFeatureDisabled) {
    TEST_DESCRIPTION("Validation should check anisotropy parameters are correct with samplerAnisotropy disabled.");

    // Determine if required device features are available
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    device_features.samplerAnisotropy = VK_FALSE;  // force anisotropy off
    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-anisotropyEnable-01070");
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    // With the samplerAnisotropy disable, the sampler must not enable it.
    sampler_info.anisotropyEnable = VK_TRUE;
    VkSampler sampler = VK_NULL_HANDLE;

    VkResult err;
    err = vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == err) {
        vk::DestroySampler(m_device->device(), sampler, NULL);
    }
    sampler = VK_NULL_HANDLE;
}

TEST_F(VkLayerTest, AnisotropyFeatureEnabled) {
    TEST_DESCRIPTION("Validation must check several conditions that apply only when Anisotropy is enabled.");

    // Determine if required device features are available
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

TEST_F(VkLayerTest, InvalidSamplerCreateInfo) {
    TEST_DESCRIPTION("Checks various cases where VkSamplerCreateInfo is invalid");
    ASSERT_NO_FATAL_FAILURE(Init());

    // reference to reset values between test cases
    VkSamplerCreateInfo const sampler_info_ref = SafeSaneSamplerCreateInfo();
    VkSamplerCreateInfo sampler_info = sampler_info_ref;

    // Mix up Lod values
    sampler_info.minLod = 4.0f;
    sampler_info.maxLod = 1.0f;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-maxLod-01973");
    sampler_info.minLod = sampler_info_ref.minLod;
    sampler_info.maxLod = sampler_info_ref.maxLod;

    // Larger mipLodBias than max limit
    sampler_info.mipLodBias = NearestGreater(m_device->phy().properties().limits.maxSamplerLodBias);
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-mipLodBias-01069");
    sampler_info.mipLodBias = sampler_info_ref.mipLodBias;
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " is not a multiple of 4");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 1, 4, updateData);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using dataSize that is not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " is not a multiple of 4");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 0, 6, updateData);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using dataSize that is < 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "must be greater than zero and less than or equal to 65536");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 0, (VkDeviceSize)-44, updateData);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using dataSize that is > 65536
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "must be greater than zero and less than or equal to 65536");
    m_commandBuffer->UpdateBuffer(buffer.handle(), 0, (VkDeviceSize)80000, updateData);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, FillBufferAlignmentAndSize) {
    TEST_DESCRIPTION("Check alignment and size parameters for vkCmdFillBuffer");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkBufferObj buffer;
    buffer.init_as_dst(*m_device, (VkDeviceSize)20, reqs);

    m_commandBuffer->begin();

    // Introduce failure by using dstOffset greater than bufferSize
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-dstOffset-00024");
    m_commandBuffer->FillBuffer(buffer.handle(), 40, 4, 0x11111111);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using size <= buffersize minus dstoffset
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-size-00027");
    m_commandBuffer->FillBuffer(buffer.handle(), 16, 12, 0x11111111);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using dstOffset that is not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " is not a multiple of 4");
    m_commandBuffer->FillBuffer(buffer.handle(), 1, 4, 0x11111111);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using size that is not multiple of 4
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " is not a multiple of 4");
    m_commandBuffer->FillBuffer(buffer.handle(), 0, 6, 0x11111111);
    m_errorMonitor->VerifyFound();

    // Introduce failure by using size that is zero
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "must be greater than zero");
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

TEST_F(VkLayerTest, SparseResidencyFlagMissing) {
    TEST_DESCRIPTION("Try to use VkSparseImageMemoryBindInfo without sparse residency flag");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!m_device->phy().features().sparseResidencyImage2D) {
        printf("%s Test requires unsupported sparseResidencyImage2D feature. Skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
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

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkSparseImageMemoryBind image_memory_bind = {};
    image_memory_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSparseImageMemoryBindInfo image_memory_bind_info = {};
    image_memory_bind_info.image = image.handle();
    image_memory_bind_info.bindCount = 1;
    image_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = {};
    bind_info.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
    bind_info.imageBindCount = 1;
    bind_info.pImageBinds = &image_memory_bind_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseImageMemoryBindInfo-image-02901");
    vk::QueueBindSparse(m_device->m_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
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
    err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);
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
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    uint8_t *pData;
    // Attempt to map memory size 0 is invalid
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-size-00680");
    err = vk::MapMemory(m_device->device(), mem, 0, 0, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Map memory twice
    err = vk::MapMemory(m_device->device(), mem, 0, mem_reqs.size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-memory-00678");
    err = vk::MapMemory(m_device->device(), mem, 0, mem_reqs.size, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();

    // Unmap the memory to avoid re-map error
    vk::UnmapMemory(m_device->device(), mem);
    // overstep offset with VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-offset-00679");
    err = vk::MapMemory(m_device->device(), mem, allocation_size + 1, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep offset w/o VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-offset-00679");
    err = vk::MapMemory(m_device->device(), mem, allocation_size + 1, VK_WHOLE_SIZE, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // overstep allocation w/o VK_WHOLE_SIZE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkMapMemory-size-00681");
    err = vk::MapMemory(m_device->device(), mem, 1, allocation_size, 0, (void **)&pData);
    m_errorMonitor->VerifyFound();
    // Now error due to unmapping memory that's not mapped
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUnmapMemory-memory-00689");
    vk::UnmapMemory(m_device->device(), mem);
    m_errorMonitor->VerifyFound();

    // Now map memory and cause errors due to flushing invalid ranges
    err = vk::MapMemory(m_device->device(), mem, 4 * atom_size, VK_WHOLE_SIZE, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    VkMappedMemoryRange mmr = {};
    mmr.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mmr.pNext = nullptr;
    mmr.memory = mem;
    mmr.offset = atom_size;  // Error b/c offset less than offset of mapped mem
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range that oversteps mapped range
    vk::UnmapMemory(m_device->device(), mem);
    err = vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    mmr.offset = atom_size;
    mmr.size = 4 * atom_size;  // Flushing bounds exceed mapped bounds
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00685");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Now flush range with VK_WHOLE_SIZE that oversteps offset
    vk::UnmapMemory(m_device->device(), mem);
    err = vk::MapMemory(m_device->device(), mem, 2 * atom_size, 4 * atom_size, 0, (void **)&pData);
    ASSERT_VK_SUCCESS(err);
    mmr.offset = atom_size;
    mmr.size = VK_WHOLE_SIZE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-00686");
    vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
    m_errorMonitor->VerifyFound();

    // Some platforms have an atomsize of 1 which makes the test meaningless
    if (atom_size > 3) {
        // Now with an offset NOT a multiple of the device limit
        vk::UnmapMemory(m_device->device(), mem);
        err = vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData);
        ASSERT_VK_SUCCESS(err);
        mmr.offset = 3;  // Not a multiple of atom_size
        mmr.size = VK_WHOLE_SIZE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-offset-00687");
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyFound();

        // Now with a size NOT a multiple of the device limit
        vk::UnmapMemory(m_device->device(), mem);
        err = vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size, 0, (void **)&pData);
        ASSERT_VK_SUCCESS(err);
        mmr.offset = atom_size;
        mmr.size = 2 * atom_size + 1;  // Not a multiple of atom_size
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMappedMemoryRange-size-01390");
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

    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vk::FreeMemory(m_device->device(), mem, NULL);
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        return;
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
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    void *mappedAddress = NULL;
    err = vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, &mappedAddress);
    m_errorMonitor->VerifyFound();

    // Attempt to flush and invalidate non-host memory
    VkMappedMemoryRange memory_range = {};
    memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memory_range.pNext = nullptr;
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

TEST_F(VkLayerTest, RebindMemory_MultiObjectDebugUtils) {
    VkResult err;
    bool pass;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-01044");

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceMemoryCommitment-memory-00690");
    VkDeviceSize size;
    vk::GetDeviceMemoryCommitment(m_device->device(), mem.handle(), &size);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-04441");
    vk::CreateImageView(m_device->device(), &dsvci, NULL, &dsv);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-dstImage-00177");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-srcBuffer-00174");

    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageOffset-00197");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), width_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageOffset-00200");
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
    err = vk::CreateImage(m_device->handle(), &depth_image_create_info, NULL, &depth_image);
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
    vk::GetImageMemoryRequirements(m_device->device(), depth_image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem1);
    ASSERT_VK_SUCCESS(err);
    err = vk::BindImageMemory(m_device->device(), depth_image, mem1, 0);

    region.imageExtent.depth = 2;
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), depth_image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), depth_image, NULL);
    vk::FreeMemory(m_device->device(), mem1, NULL);
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CreateUnknownObject) {
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageMemoryRequirements-image-parameter");

    TEST_DESCRIPTION("Pass an invalid image object handle into a Vulkan API call.");

    ASSERT_NO_FATAL_FAILURE(Init());

    // Pass bogus handle into GetImageMemoryRequirements
    VkMemoryRequirements mem_reqs;
    uint64_t fakeImageHandle = 0xCADECADE;
    VkImage fauxImage = reinterpret_cast<VkImage &>(fakeImageHandle);

    vk::GetImageMemoryRequirements(m_device->device(), fauxImage, &mem_reqs);

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

    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    vk::GetImageMemoryRequirements(m_device->device(), image, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;

    // Introduce Failure, select invalid TypeIndex
    VkPhysicalDeviceMemoryProperties memory_info;

    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);
    unsigned int i;
    for (i = 0; i < memory_info.memoryTypeCount; i++) {
        if ((mem_reqs.memoryTypeBits & (1 << i)) == 0) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }
    if (i >= memory_info.memoryTypeCount) {
        printf("%s No invalid memory type index could be found; skipped.\n", kSkipPrefix);
        vk::DestroyImage(m_device->device(), image, NULL);
        return;
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "for this object type are not compatible with the memory");

    err = vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindImageMemory(m_device->device(), image, mem, 0);
    (void)err;

    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), image, NULL);
    vk::FreeMemory(m_device->device(), mem, NULL);
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
        err = vk::CreateImage(device(), &image_create_info, NULL, &image);
        ASSERT_VK_SUCCESS(err);
        err = vk::CreateBuffer(device(), &buffer_create_info, NULL, &buffer);
        ASSERT_VK_SUCCESS(err);
        VkMemoryRequirements image_mem_reqs = {}, buffer_mem_reqs = {};
        vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
        vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);

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
        err = vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem);
        ASSERT_VK_SUCCESS(err);
        err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
        ASSERT_VK_SUCCESS(err);

        err = vk::BindImageMemory(device(), image, image_mem, 0);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-01044");
        err = vk::BindImageMemory(device(), image, image_mem, 0);
        (void)err;  // This may very well return an error.
        m_errorMonitor->VerifyFound();

        err = vk::BindBufferMemory(device(), buffer, buffer_mem, 0);
        ASSERT_VK_SUCCESS(err);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-buffer-01029");
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
        VkMemoryAllocateInfo image_alloc_info = {}, buffer_alloc_info = {};
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_alloc_info.allocationSize = image_mem_reqs.size;
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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
                printf("%s Sparse image format not supported; skipped.\n", kSkipPrefix);
                return;
            } else {
                VkImage sparse_image = VK_NULL_HANDLE;
                err = vk::CreateImage(m_device->device(), &sparse_image_create_info, NULL, &sparse_image);
                ASSERT_VK_SUCCESS(err);
                VkMemoryRequirements sparse_mem_reqs = {};
                vk::GetImageMemoryRequirements(m_device->device(), sparse_image, &sparse_mem_reqs);
                if (sparse_mem_reqs.memoryTypeBits != 0) {
                    VkMemoryAllocateInfo sparse_mem_alloc = {};
                    sparse_mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                    sparse_mem_alloc.pNext = NULL;
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
                VkMemoryAllocateInfo sparse_mem_alloc = {};
                sparse_mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                sparse_mem_alloc.pNext = NULL;
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

TEST_F(VkLayerTest, BindInvalidMemoryYcbcr) {
    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

    // Create aliased function pointers for 1.0 and 1.1 contexts
    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
    PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2Function = nullptr;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkBindImageMemory2Function = vk::BindImageMemory2;
        vkGetImageMemoryRequirements2Function = vk::GetImageMemoryRequirements2;
    } else {
        vkBindImageMemory2Function = (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
        vkGetImageMemoryRequirements2Function =
            (PFN_vkGetImageMemoryRequirements2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkGetImageMemoryRequirements2KHR");
    }

    // Try to bind an image created with Disjoint bit
    VkFormatProperties format_properties;
    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &format_properties);
    // Need to make sure disjoint is supported for format
    // Also need to support an arbitrary image usage feature
    if (0 == (format_properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
        printf("%s test requires disjoint/sampled feature bit on format.  Skipping.\n", kSkipPrefix);
    } else {
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = NULL;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = mp_format;
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
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &image));

        VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

        // Find a valid memory type index to memory to be allocated from
        VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        alloc_info.allocationSize = mem_req2.memoryRequirements.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));

        VkDeviceMemory image_memory;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &image_memory));

        // Bind disjoint with BindImageMemory instead of BindImageMemory2
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-01608");
        vk::BindImageMemory(device(), image, image_memory, 0);
        m_errorMonitor->VerifyFound();

        VkBindImagePlaneMemoryInfo plane_memory_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO};
        ASSERT_TRUE(FormatPlaneCount(mp_format) == 2);
        plane_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;

        VkBindImageMemoryInfo bind_image_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO};
        bind_image_info.pNext = &plane_memory_info;
        bind_image_info.image = image;
        bind_image_info.memory = image_memory;
        bind_image_info.memoryOffset = 0;

        // Set invalid planeAspect
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImagePlaneMemoryInfo-planeAspect-02283");
        // Error is thrown from not having both planes bound
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-02858");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory2-pBindInfos-02858");
        // Might happen as plane2 wasn't queried for its memroy type
        m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01619");
        m_errorMonitor->SetUnexpectedError("VUID-VkBindImageMemoryInfo-pNext-01621");
        vkBindImageMemory2Function(device(), 1, &bind_image_info);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), image_memory, NULL);
        vk::DestroyImage(device(), image, nullptr);
    }

    // Bind image with VkBindImagePlaneMemoryInfo without disjoint bit in image
    // Need to support an arbitrary image usage feature for multi-planar format
    if (0 == (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        printf("%s test requires sampled feature bit on multi-planar format.  Skipping.\n", kSkipPrefix);
    } else {
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = NULL;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = mp_format;
        image_create_info.extent.width = 64;
        image_create_info.extent.height = 64;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_create_info.flags = 0;  // no disjoint bit set

        VkImage image;
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &image));

        VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        mem_req_info2.pNext = NULL;
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

        // Find a valid memory type index to memory to be allocated from
        VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        alloc_info.allocationSize = mem_req2.memoryRequirements.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));

        VkDeviceMemory image_memory;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &image_memory));

        VkBindImagePlaneMemoryInfo plane_memory_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO};
        plane_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        VkBindImageMemoryInfo bind_image_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO};
        bind_image_info.pNext = &plane_memory_info;
        bind_image_info.image = image;
        bind_image_info.memory = image_memory;
        bind_image_info.memoryOffset = 0;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01618");
        vkBindImageMemory2Function(device(), 1, &bind_image_info);
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), image_memory, NULL);
        vk::DestroyImage(device(), image, nullptr);
    }
}

TEST_F(VkLayerTest, BindInvalidMemory2Disjoint) {
    TEST_DESCRIPTION("These tests deal with VK_KHR_bind_memory_2 and disjoint memory being bound");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    bool bind_memory_2_extension = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else if (bind_memory_2_extension) {
        // bind_memory_2 extension is subset of mp_extensions
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    } else {
        printf("%s test requires VK_KHR_bind_memory2 extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Create aliased function pointers for 1.0 and 1.1 contexts
    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
    PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2Function = nullptr;

    if (bind_memory_2_extension) {
        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vkBindImageMemory2Function = vk::BindImageMemory2;
        } else {
            vkBindImageMemory2Function =
                (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
        }
    }

    if (mp_extensions) {
        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vkGetImageMemoryRequirements2Function = vk::GetImageMemoryRequirements2;
        } else {
            vkGetImageMemoryRequirements2Function =
                (PFN_vkGetImageMemoryRequirements2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkGetImageMemoryRequirements2KHR");
        }
    }

    const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = tex_format;
    image_create_info.extent.width = 256;
    image_create_info.extent.height = 256;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    // Only gets used in MP tests
    VkImageCreateInfo mp_image_create_info = image_create_info;
    mp_image_create_info.format = mp_format;
    mp_image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;

    // Check for support of format used by all multi-planar tests
    // Need seperate boolean as its valid to do tests that support YCbCr but not disjoint
    bool mp_disjoint_support = false;
    if (mp_extensions == true) {
        VkFormatProperties mp_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &mp_format_properties);
        if (0 !=
            (mp_format_properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            mp_disjoint_support = true;
        }
    }

    // Try to bind memory to an object with an invalid memoryOffset

    VkImage image = VK_NULL_HANDLE;
    ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &image));
    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = {};
    image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    // Leave some extra space for alignment wiggle room
    image_alloc_info.allocationSize = image_mem_reqs.size + image_mem_reqs.alignment;
    ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
    VkDeviceMemory image_mem;
    ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem));

    // Keep values outside scope so multiple tests cases can reuse
    VkImage mp_image = VK_NULL_HANDLE;
    VkDeviceMemory mp_image_mem[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
    VkMemoryRequirements2 mp_image_mem_reqs2[2];
    VkMemoryAllocateInfo mp_image_alloc_info[2];
    if (mp_disjoint_support == true) {
        ASSERT_VK_SUCCESS(vk::CreateImage(device(), &mp_image_create_info, NULL, &mp_image));

        VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
        image_plane_req.pNext = nullptr;
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        mem_req_info2.pNext = (void *)&image_plane_req;
        mem_req_info2.image = mp_image;
        mp_image_mem_reqs2[0].sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        mp_image_mem_reqs2[0].pNext = nullptr;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
        mp_image_mem_reqs2[1].sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        mp_image_mem_reqs2[1].pNext = nullptr;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[1]);

        mp_image_alloc_info[0].sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mp_image_alloc_info[0].pNext = nullptr;
        mp_image_alloc_info[1].sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mp_image_alloc_info[1].pNext = nullptr;
        // Leave some extra space for alignment wiggle room
        // plane 0
        mp_image_alloc_info[0].allocationSize =
            mp_image_mem_reqs2[0].memoryRequirements.size + mp_image_mem_reqs2[0].memoryRequirements.alignment;
        ASSERT_TRUE(
            m_device->phy().set_memory_type(mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[0], 0));
        // Exact size as VU will always be for plane 1
        // plane 1
        mp_image_alloc_info[1].allocationSize = mp_image_mem_reqs2[1].memoryRequirements.size;
        ASSERT_TRUE(
            m_device->phy().set_memory_type(mp_image_mem_reqs2[1].memoryRequirements.memoryTypeBits, &mp_image_alloc_info[1], 0));

        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_image_mem[0]));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[1], NULL, &mp_image_mem[1]));
    }

    // All planes must be bound at once the same here
    VkBindImagePlaneMemoryInfo plane_memory_info[2];
    plane_memory_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
    plane_memory_info[0].pNext = nullptr;
    plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    plane_memory_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
    plane_memory_info[1].pNext = nullptr;
    plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

    // Test unaligned memory offset

    // single-plane image
    if (bind_memory_2_extension == true) {
        VkBindImageMemoryInfo bind_image_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO};
        bind_image_info.pNext = nullptr;
        bind_image_info.image = image;
        bind_image_info.memory = image_mem;
        bind_image_info.memoryOffset = 1;  // off alignment

        if (mp_disjoint_support == true) {
            VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
            mem_req_info2.pNext = nullptr;
            mem_req_info2.image = image;
            VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, nullptr};
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

            if (mem_req2.memoryRequirements.alignment > 1) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01616");
                vkBindImageMemory2Function(device(), 1, &bind_image_info);
                m_errorMonitor->VerifyFound();
            }
        } else {
            // Same as 01048 but with bindImageMemory2 call
            if (image_mem_reqs.alignment > 1) {
                const char *vuid =
                    (mp_extensions) ? "VUID-VkBindImageMemoryInfo-pNext-01616" : "VUID-VkBindImageMemoryInfo-memoryOffset-01613";
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
                vkBindImageMemory2Function(device(), 1, &bind_image_info);
                m_errorMonitor->VerifyFound();
            }
        }
    }

    // Multi-plane image
    if (mp_disjoint_support == true) {
        if (mp_image_mem_reqs2[0].memoryRequirements.alignment > 1) {
            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[0].pNext = (void *)&plane_memory_info[0];
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = 1;  // off alignment
            bind_image_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[1].pNext = (void *)&plane_memory_info[1];
            bind_image_info[1].image = mp_image;
            bind_image_info[1].memory = mp_image_mem[1];
            bind_image_info[1].memoryOffset = 0;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01620");
            vkBindImageMemory2Function(device(), 2, bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    }

    // Test memory offsets within the memory allocation, but which leave too little memory for
    // the resource.
    // single-plane image
    if (bind_memory_2_extension == true) {
        VkBindImageMemoryInfo bind_image_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO};
        bind_image_info.pNext = nullptr;
        bind_image_info.image = image;
        bind_image_info.memory = image_mem;

        if (mp_disjoint_support == true) {
            VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
            mem_req_info2.pNext = nullptr;
            mem_req_info2.image = image;
            VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, nullptr};
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

            VkDeviceSize image2_offset = (mem_req2.memoryRequirements.size - 1) & ~(mem_req2.memoryRequirements.alignment - 1);
            if ((image2_offset > 0) &&
                (mem_req2.memoryRequirements.size < (image_alloc_info.allocationSize - mem_req2.memoryRequirements.alignment))) {
                bind_image_info.memoryOffset = image2_offset;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01617");
                vkBindImageMemory2Function(device(), 1, &bind_image_info);
                m_errorMonitor->VerifyFound();
            }
        } else {
            // Same as 01049 but with bindImageMemory2 call
            VkDeviceSize image_offset = (image_mem_reqs.size - 1) & ~(image_mem_reqs.alignment - 1);
            if ((image_offset > 0) && (image_mem_reqs.size < (image_alloc_info.allocationSize - image_mem_reqs.alignment))) {
                bind_image_info.memoryOffset = image_offset;
                const char *vuid =
                    (mp_extensions) ? "VUID-VkBindImageMemoryInfo-pNext-01617" : "VUID-VkBindImageMemoryInfo-memory-01614";
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
                vkBindImageMemory2Function(device(), 1, &bind_image_info);
                m_errorMonitor->VerifyFound();
            }
        }
    }

    // Multi-plane image
    if (mp_disjoint_support == true) {
        VkDeviceSize mp_image_offset =
            (mp_image_mem_reqs2[0].memoryRequirements.size - 1) & ~(mp_image_mem_reqs2[0].memoryRequirements.alignment - 1);
        if ((mp_image_offset > 0) &&
            (mp_image_mem_reqs2[0].memoryRequirements.size <
             (mp_image_alloc_info[0].allocationSize - mp_image_mem_reqs2[0].memoryRequirements.alignment))) {
            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[0].pNext = (void *)&plane_memory_info[0];
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = mp_image_offset;  // mis-offset
            bind_image_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[1].pNext = (void *)&plane_memory_info[1];
            bind_image_info[1].image = mp_image;
            bind_image_info[1].memory = mp_image_mem[1];
            bind_image_info[1].memoryOffset = 0;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01621");
            vkBindImageMemory2Function(device(), 2, bind_image_info);
            m_errorMonitor->VerifyFound();
        }
    }

    // Free Memory to reset
    vk::FreeMemory(device(), image_mem, NULL);
    if (mp_disjoint_support == true) {
        // only reset plane 0
        vk::FreeMemory(device(), mp_image_mem[0], NULL);
    }

    // Try to bind memory to an object with an invalid memory type

    // Create a mask of available memory types *not* supported by these resources, and try to use one of them.
    VkPhysicalDeviceMemoryProperties memory_properties = {};
    vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memory_properties);

    // single-plane image
    if (bind_memory_2_extension == true) {
        VkBindImageMemoryInfo bind_image_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO};
        bind_image_info.pNext = nullptr;
        bind_image_info.image = image;
        bind_image_info.memoryOffset = 0;

        if (mp_disjoint_support == true) {
            VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
            mem_req_info2.pNext = nullptr;
            mem_req_info2.image = image;
            VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2, nullptr};
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

            uint32_t image2_unsupported_mem_type_bits =
                ((1 << memory_properties.memoryTypeCount) - 1) & ~mem_req2.memoryRequirements.memoryTypeBits;
            if (image2_unsupported_mem_type_bits != 0) {
                ASSERT_TRUE(m_device->phy().set_memory_type(image2_unsupported_mem_type_bits, &image_alloc_info, 0));
                ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem));
                bind_image_info.memory = image_mem;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01615");
                vkBindImageMemory2Function(device(), 1, &bind_image_info);
                m_errorMonitor->VerifyFound();
                vk::FreeMemory(device(), image_mem, NULL);
            }
        } else {
            // Same as 01047 but with bindImageMemory2 call
            uint32_t image_unsupported_mem_type_bits =
                ((1 << memory_properties.memoryTypeCount) - 1) & ~image_mem_reqs.memoryTypeBits;
            if (image_unsupported_mem_type_bits != 0) {
                ASSERT_TRUE(m_device->phy().set_memory_type(image_unsupported_mem_type_bits, &image_alloc_info, 0));
                ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_mem));
                bind_image_info.memory = image_mem;
                const char *vuid =
                    (mp_extensions) ? "VUID-VkBindImageMemoryInfo-pNext-01615" : "VUID-VkBindImageMemoryInfo-memory-01612";
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
                vkBindImageMemory2Function(device(), 1, &bind_image_info);
                m_errorMonitor->VerifyFound();
                vk::FreeMemory(device(), image_mem, NULL);
            }
        }
    }

    // Multi-plane image
    if (mp_disjoint_support == true) {
        // Get plane 0 memory requirements
        VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
        image_plane_req.pNext = nullptr;
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        mem_req_info2.pNext = (void *)&image_plane_req;
        mem_req_info2.image = mp_image;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        uint32_t mp_image_unsupported_mem_type_bits =
            ((1 << memory_properties.memoryTypeCount) - 1) & ~mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits;
        if (mp_image_unsupported_mem_type_bits != 0) {
            mp_image_alloc_info[0].allocationSize = mp_image_mem_reqs2[0].memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mp_image_unsupported_mem_type_bits, &mp_image_alloc_info[0], 0));
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_image_mem[0]));

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[0].pNext = (void *)&plane_memory_info[0];
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[1].pNext = (void *)&plane_memory_info[1];
            bind_image_info[1].image = mp_image;
            bind_image_info[1].memory = mp_image_mem[1];
            bind_image_info[1].memoryOffset = 0;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01619");
            vkBindImageMemory2Function(device(), 2, bind_image_info);
            m_errorMonitor->VerifyFound();
            vk::FreeMemory(device(), mp_image_mem[0], NULL);
        }
    }

    vk::DestroyImage(device(), image, NULL);
    if (mp_disjoint_support == true) {
        vk::FreeMemory(device(), mp_image_mem[1], NULL);
        vk::DestroyImage(device(), mp_image, NULL);
    }
}

TEST_F(VkLayerTest, BindInvalidMemoryNoCheck) {
    TEST_DESCRIPTION("Tests case were no call to memory requirements was made prior to binding");

    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // first test buffer
    {
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.pNext = NULL;
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
        VkMemoryAllocateInfo buffer_alloc_info = {};
        buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = NULL;
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
        VkMemoryAllocateInfo image_alloc_info = {};
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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
        printf("%s Rest of test rely on YCbCr Multi-planar support.\n", kSkipPrefix);
        return;
    } else {
        // Check for support of format used by all multi-planar tests
        const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        VkFormatProperties mp_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &mp_format_properties);
        if (0 ==
            (mp_format_properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            printf("%s Rest of test rely on a supported disjoint format.\n", kSkipPrefix);
            return;
        }

        VkImageCreateInfo mp_image_create_info = {};
        mp_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        mp_image_create_info.pNext = NULL;
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

        VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
        image_plane_req.pNext = nullptr;
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        mem_req_info2.pNext = (void *)&image_plane_req;
        mem_req_info2.image = mp_image;
        mp_image_mem_reqs2[0].sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        mp_image_mem_reqs2[0].pNext = nullptr;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
        mp_image_mem_reqs2[1].sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        mp_image_mem_reqs2[1].pNext = nullptr;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[1]);

        mp_image_alloc_info[0].sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mp_image_alloc_info[0].pNext = nullptr;
        mp_image_alloc_info[1].sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mp_image_alloc_info[1].pNext = nullptr;

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
            plane_memory_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
            plane_memory_info[0].pNext = nullptr;
            plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
            plane_memory_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
            plane_memory_info[1].pNext = nullptr;
            plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[0].pNext = (void *)&plane_memory_info[0];
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[1].pNext = (void *)&plane_memory_info[1];
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
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    bool bind_memory_2_extension = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    } else if (bind_memory_2_extension) {
        // bind_memory_2 extension is subset of mp_extensions
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    } else {
        printf("%s test requires VK_KHR_bind_memory2 extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Create aliased function pointers for 1.0 and 1.1 contexts
    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkBindImageMemory2Function = vk::BindImageMemory2;
    } else {
        vkBindImageMemory2Function = (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
    }

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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
        VkMemoryAllocateInfo image_alloc_info = {};
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_a_mem));
        vk::GetImageMemoryRequirements(device(), image_b, &image_mem_reqs);
        image_alloc_info.allocationSize = image_mem_reqs.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(image_mem_reqs.memoryTypeBits, &image_alloc_info, 0));
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &image_alloc_info, NULL, &image_b_mem));

        // Try binding same image twice in array
        VkBindImageMemoryInfo bind_image_info[3];
        bind_image_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
        bind_image_info[0].pNext = nullptr;
        bind_image_info[0].image = image_a;
        bind_image_info[0].memory = image_a_mem;
        bind_image_info[0].memoryOffset = 0;
        bind_image_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
        bind_image_info[1].pNext = nullptr;
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
        if (0 ==
            (mp_format_properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            printf("%s test requires disjoint support extensions, not available.  Skipping.\n", kSkipPrefix);
            return;
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
        VkMemoryAllocateInfo image_alloc_info = {};
        image_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
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
        plane_memory_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
        plane_memory_info[0].pNext = nullptr;
        plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        plane_memory_info[1].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
        plane_memory_info[1].pNext = nullptr;
        plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

        // set all sType and memoryOffset as they are the same
        VkBindImageMemoryInfo bind_image_info[6];
        for (int i = 0; i < 6; i++) {
            bind_image_info[i].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
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

        // Valid case of binding 2 disjoint image and normal image by removing duplicate
        m_errorMonitor->ExpectSuccess();
        vkBindImageMemory2Function(device(), 5, bind_image_info);
        m_errorMonitor->VerifyNotFound();

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

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "Number of currently valid memory objects is not less than the maximum allowed");

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
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

TEST_F(VkLayerTest, ExceedSamplerAllocationCount) {
    VkResult err = VK_SUCCESS;
    const int max_samplers = 32;
    VkSampler samplers[max_samplers + 1];

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkSetPhysicalDeviceLimitsEXT fpvkSetPhysicalDeviceLimitsEXT =
        (PFN_vkSetPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceLimitsEXT");
    PFN_vkGetOriginalPhysicalDeviceLimitsEXT fpvkGetOriginalPhysicalDeviceLimitsEXT =
        (PFN_vkGetOriginalPhysicalDeviceLimitsEXT)vk::GetInstanceProcAddr(instance(), "vkGetOriginalPhysicalDeviceLimitsEXT");

    if (!(fpvkSetPhysicalDeviceLimitsEXT) || !(fpvkGetOriginalPhysicalDeviceLimitsEXT)) {
        printf("%s Can't find device_profile_api functions; skipped.\n", kSkipPrefix);
        return;
    }
    VkPhysicalDeviceProperties props;
    fpvkGetOriginalPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    if (props.limits.maxSamplerAllocationCount > max_samplers) {
        props.limits.maxSamplerAllocationCount = max_samplers;
        fpvkSetPhysicalDeviceLimitsEXT(gpu(), &props.limits);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "Number of currently valid sampler objects is not less than the maximum allowed");

    VkSamplerCreateInfo sampler_create_info = SafeSaneSamplerCreateInfo();

    int i;
    for (i = 0; i <= max_samplers; i++) {
        err = vk::CreateSampler(m_device->device(), &sampler_create_info, NULL, &samplers[i]);
        if (err != VK_SUCCESS) {
            break;
        }
    }
    m_errorMonitor->VerifyFound();

    for (int j = 0; j < i; j++) {
        vk::DestroySampler(m_device->device(), samplers[j], NULL);
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00233");
        vk::CmdBlitImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00234");
        vk::CmdBlitImage(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst_image.handle(),
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
            kErrorBit, "was created with a sample count of VK_SAMPLE_COUNT_4_BIT but must be VK_SAMPLE_COUNT_1_BIT");
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), src_buffer.handle(), dst_image.handle(),
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
            kErrorBit, "was created with a sample count of VK_SAMPLE_COUNT_4_BIT but must be VK_SAMPLE_COUNT_1_BIT");
        vk::CmdCopyImageToBuffer(m_commandBuffer->handle(), src_image.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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
    VkFormat f_ycbcr = VK_FORMAT_B16G16R16G16_422_UNORM;

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (usrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (fdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), unsigned_image.image(), unsigned_image.Layout(), float_image.image(),
                     float_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (fsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (udst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), float_image.image(), float_image.Layout(), unsigned_image.image(),
                     unsigned_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Signed int vs not an int,
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    if (ssrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (fdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), signed_image.image(), signed_image.Layout(), float_image.image(),
                     float_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    if (fsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (sdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), float_image.image(), float_image.Layout(), signed_image.image(),
                     signed_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Signed vs Unsigned int - generates both VUs
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (ssrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (udst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), signed_image.image(), signed_image.Layout(), unsigned_image.image(),
                     unsigned_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (usrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (sdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), unsigned_image.image(), unsigned_image.Layout(), signed_image.image(),
                     signed_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    if (ImageFormatIsSupported(gpu(), f_ycbcr, VK_IMAGE_TILING_OPTIMAL)) {
        bool ycbcrsrc = !ImageFormatAndFeaturesSupported(gpu(), f_ycbcr, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_SRC_BIT);
        bool ycbcrdst = !ImageFormatAndFeaturesSupported(gpu(), f_ycbcr, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_BLIT_DST_BIT);

        VkImageObj ycbcr_image(m_device);
        ycbcr_image.Init(64, 64, 1, f_ycbcr, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                         VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ycbcr_image.initialized());
        ycbcr_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

        // Src, dst is ycbcr format
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-01561");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-01562");
        if (ycbcrsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
        if (ycbcrdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
        vk::CmdBlitImage(m_commandBuffer->handle(), ycbcr_image.image(), ycbcr_image.Layout(), ycbcr_image.image(),
                         ycbcr_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
        m_errorMonitor->VerifyFound();
    } else {
        printf("%s Requested ycbcr format not supported - skipping test case.\n", kSkipPrefix);
    }

    // Depth vs any non-identical depth format
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00231");
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (d2src) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (d1dst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), depth_image2.image(), depth_image2.Layout(), depth_image.image(),
                     depth_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BlitImageFilters) {
    bool cubic_support = false;
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-02001");
        vk::CmdBlitImage(m_commandBuffer->handle(), src2D.image(), src2D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                         VK_FILTER_LINEAR);
        m_errorMonitor->VerifyFound();
    }

    if (cubic_support && !ImageFormatAndFeaturesSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL,
                                                          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG)) {
        // Invalid filter CUBIC_IMG
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-02002");
        vk::CmdBlitImage(m_commandBuffer->handle(), src3D.image(), src3D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                         VK_FILTER_CUBIC_IMG);
        m_errorMonitor->VerifyFound();

        // Invalid filter CUBIC_IMG + invalid 2D source image
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-02002");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-00237");
        vk::CmdBlitImage(m_commandBuffer->handle(), src2D.image(), src2D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImageLayout-00222");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     img_dst_transfer.image(), img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);
    m_errorMonitor->VerifyFound();

    // Illegal destImageLayout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImageLayout-00227");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_dst_transfer.image(),
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    // Source image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_color.image(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    // Destination image in invalid layout at start of the CB
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_color.image(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.image(),
                     img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vk::QueueWaitIdle(m_device->m_queue);
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

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImageLayout-00221");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_general.image(), VK_IMAGE_LAYOUT_GENERAL, img_dst_transfer.image(),
                     img_dst_transfer.Layout(), 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(err);

    // Destination image in invalid layout in the middle of CB
    m_commandBuffer->reset(0);
    m_commandBuffer->begin();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    img_barrier.image = img_dst_transfer.handle();

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &img_barrier);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImageLayout-00226");
    vk::CmdBlitImage(m_commandBuffer->handle(), img_src_transfer.image(), img_src_transfer.Layout(), img_dst_transfer.image(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    m_commandBuffer->end();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    err = vk::QueueWaitIdle(m_device->m_queue);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00245");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_1D.image(), image_1D.Layout(), image_1D.image(), image_1D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[0] = {32, 1, 0};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00250");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_1D.image(), image_1D.Layout(), image_1D.image(), image_1D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // 2D, with src/dest z offsets other than (0,1)
    blit_region.srcOffsets[0] = {0, 0, 1};
    blit_region.srcOffsets[1] = {24, 31, 1};
    blit_region.dstOffsets[0] = {32, 32, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00247");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[0] = {32, 32, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00252");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Source offsets exceeding source image dimensions
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {65, 64, 1};  // src x
    blit_region.dstOffsets[0] = {0, 0, 0};
    blit_region.dstOffsets[1] = {64, 64, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcOffset-00243");  // x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00215");   // src region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[1] = {64, 65, 1};                                                 // src y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcOffset-00244");  // y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00215");   // src region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.srcOffsets[0] = {0, 0, 65};  // src z
    blit_region.srcOffsets[1] = {64, 64, 64};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcOffset-00246");  // z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00215");   // src region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_3D.image(), image_3D.Layout(), image_2D.image(), image_2D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Dest offsets exceeding source image dimensions
    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {64, 64, 1};
    blit_region.dstOffsets[0] = {96, 64, 32};  // dst x
    blit_region.dstOffsets[1] = {64, 0, 33};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstOffset-00248");  // x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00216");   // dst region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.dstOffsets[0] = {0, 65, 32};                                                 // dst y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstOffset-00249");  // y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00216");   // dst region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
                     &blit_region, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blit_region.dstOffsets[0] = {0, 64, 65};  // dst z
    blit_region.dstOffsets[1] = {64, 0, 64};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstOffset-00251");  // z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00216");   // dst region
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_3D.image(), image_3D.Layout(), 1,
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-aspectMask-00241");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-aspectMask-00242");
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid src mip level
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.mipLevel = ci.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcSubresource-01705");  // invalid srcSubresource.mipLevel
    // Redundant unavoidable errors
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcOffset-00243");  // out-of-bounds srcOffset.x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcOffset-00244");  // out-of-bounds srcOffset.y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcOffset-00246");  // out-of-bounds srcOffset.z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-pRegions-00215");  // region not contained within src image
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid dst mip level
    blitRegion.srcSubresource.mipLevel = 0;
    blitRegion.dstSubresource.mipLevel = ci.mipLevels;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstSubresource-01706");  // invalid dstSubresource.mipLevel
    // Redundant unavoidable errors
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstOffset-00248");  // out-of-bounds dstOffset.x
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstOffset-00249");  // out-of-bounds dstOffset.y
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstOffset-00251");  // out-of-bounds dstOffset.z
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-pRegions-00216");  // region not contained within dst image
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid src array layer
    blitRegion.dstSubresource.mipLevel = 0;
    blitRegion.srcSubresource.baseArrayLayer = ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcSubresource-01707");  // invalid srcSubresource layer range
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit with invalid dst array layer
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.baseArrayLayer = ci.arrayLayers;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-dstSubresource-01708");  // invalid dstSubresource layer range
                                                                                       // Redundant unavoidable errors
    vk::CmdBlitImage(m_commandBuffer->handle(), color_img.image(), color_img.Layout(), color_img.image(), color_img.Layout(), 1,
                     &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    blitRegion.dstSubresource.baseArrayLayer = 0;

    // Blit multi-sample image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00233");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-00234");
    vk::CmdBlitImage(m_commandBuffer->handle(), ms_img.image(), ms_img.Layout(), ms_img.image(), ms_img.Layout(), 1, &blitRegion,
                     VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // Blit 3D with baseArrayLayer != 0 or layerCount != 1
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00240");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdBlitImage-srcSubresource-01707");  // base+count > total layer count
    vk::CmdBlitImage(m_commandBuffer->handle(), color_3D_img.image(), color_3D_img.Layout(), color_3D_img.image(),
                     color_3D_img.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00240");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkImageSubresourceLayers-layerCount-01700");  // layer count == 0 (src)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkImageBlit-layerCount-00239");  // src/dst layer count mismatch
    vk::CmdBlitImage(m_commandBuffer->handle(), color_3D_img.image(), color_3D_img.Layout(), color_3D_img.image(),
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), depth_img.image(), depth_img.Layout(), depth_img.image(), depth_img.Layout(), 1,
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // srcOffset image transfer granularity
    command_buffer.CopyImage(srcImage, VK_IMAGE_LAYOUT_GENERAL, dstImage, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
    m_errorMonitor->VerifyFound();

    // Introduce failure by setting extent to a granularity value that is bad
    // for both the source and destination image.
    copyRegion.srcOffset.y = 0;
    copyRegion.extent.width = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImage-srcOffset-01783");  // src extent image transfer granularity
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyImageToBuffer-imageOffset-01794");  // image transfer granularity
    vk::CmdCopyImageToBuffer(command_buffer.handle(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.handle(), 1, &region);
    m_errorMonitor->VerifyFound();
    region.imageExtent.width = granularity.width;

    // Introduce failure by setting imageOffset to a bad granularity value
    region.imageOffset.z = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkCmdCopyBufferToImage-imageOffset-01793");  // image transfer granularity
    vk::CmdCopyBufferToImage(command_buffer.handle(), buffer.handle(), dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
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

    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);
    rpci.dependencyCount = 0;
    rpci.pDependencies = nullptr;
    err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp_noselfdep);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
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

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    VkMemoryBarrier mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    mem_barrier.pNext = NULL;
    mem_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_commandBuffer->handle());

    rpbi.renderPass = rp;
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();
    // Now mis-match dst stage mask
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_errorMonitor->VerifyFound();
    // Set srcQueueFamilyIndex to something other than IGNORED
    img_barrier.srcQueueFamilyIndex = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcQueueFamilyIndex-01182");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    // Mis-match mem barrier src access mask
    mem_barrier = {};
    mem_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    mem_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr,
                           0, nullptr);
    m_errorMonitor->VerifyFound();
    // Mis-match mem barrier dst access mask. Also set srcAccessMask to 0 which should not cause an error
    mem_barrier.srcAccessMask = 0;
    mem_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1, &mem_barrier, 0, nullptr,
                           0, nullptr);
    m_errorMonitor->VerifyFound();
    // Mis-match image barrier src access mask
    img_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();
    // Mis-match image barrier dst access mask
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();
    // Mis-match dependencyFlags
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-pDependencies-02285");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-bufferMemoryBarrierCount-01178");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &bmb, 0,
                           nullptr);
    m_errorMonitor->VerifyFound();
    // Add image barrier w/ image handle that's not in framebuffer
    VkImageObj lone_image(m_device);
    lone_image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    img_barrier.image = lone_image.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-image-04073");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();
    // Have image barrier with mis-matched layouts
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-oldLayout-01181");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_commandBuffer->handle());

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp_noselfdep, nullptr);
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
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindBufferMemory(m_device->device(), buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer, 0, VK_WHOLE_SIZE, 0);
    m_commandBuffer->end();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");
    // Destroy buffer dependency prior to submit to cause ERROR
    vk::DestroyBuffer(m_device->device(), buffer, NULL);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    vk::FreeMemory(m_device->handle(), mem, NULL);
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

        VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
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
        err = pipe.CreateGraphicsPipeline();
        if (err != VK_SUCCESS) {
            printf("%s Unable to compile shader, skipping.\n", kSkipPrefix);
            return;
        }

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        VkViewport viewport = {0, 0, 16, 16, 0, 1};
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        VkRect2D scissor = {{0, 0}, {16, 16}};
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        // Bind pipeline to cmd buffer - This causes crash on Mali
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
    }
    // buffer is released.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Descriptor in binding #0 index 0 is using buffer");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::DestroyBufferView(m_device->device(), view, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Descriptor in binding #0 index 0 is using bufferView");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    bvci.buffer = buffer.handle();
    VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    descriptor_set.descriptor_writes.clear();
    descriptor_set.WriteDescriptorBufferView(0, view);
    descriptor_set.UpdateDescriptorSets();

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Delete BufferView in order to invalidate cmd buffer
    vk::DestroyBufferView(m_device->device(), view, NULL);
    // Now attempt submit of cmd buffer
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBufferView");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
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
        vk::CmdClearColorImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &isr);
        m_commandBuffer->end();
    }
    // Destroy image dependency prior to submit to cause ERROR
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferFramebufferImageDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a framebuffer image dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);
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
        err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
        ASSERT_VK_SUCCESS(err);

        VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 32, 32, 1};
        err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    m_commandBuffer->QueueCommandBuffer(false);
    m_errorMonitor->VerifyFound();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyImageView(m_device->device(), view, nullptr);
}

TEST_F(VkLayerTest, FramebufferAttachmentMemoryFreed) {
    TEST_DESCRIPTION("Attempt to create framebuffer with attachment which memory was freed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkFormatProperties format_properties;
    VkResult err = VK_SUCCESS;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_B8G8R8A8_UNORM, &format_properties);
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

        vk_testing::Image image;
        image.init_no_mem(*m_device, image_ci);

        vk_testing::DeviceMemory *image_memory = new vk_testing::DeviceMemory;
        image_memory->init(*m_device, vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), 0));
        image.bind_memory(*image_memory, 0);

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
        err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
        ASSERT_VK_SUCCESS(err);

        VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 32, 32, 1};

        // Introduce error:
        // Free the attachment image memory, then create framebuffer.
        delete image_memory;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-BoundResourceFreedMemoryAccess");
        err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyImageView(m_device->device(), view, nullptr);
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
    VkResult err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);
    // Have to bind memory to image before recording cmd in cmd buffer using it
    VkMemoryRequirements mem_reqs;
    VkDeviceMemory image_mem;
    bool pass;
    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
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

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buf_info.size = 1024;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = 1024;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        return;
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

TEST_F(VkLayerTest, MultiplaneImageLayoutBadAspectFlags) {
    TEST_DESCRIPTION("Query layout of a multiplane image using illegal aspect flag masks");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
    VkResult err = vk::CreateImage(device(), &ci, NULL, &image_2plane);
    ASSERT_VK_SUCCESS(err);

    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    err = vk::CreateImage(device(), &ci, NULL, &image_3plane);
    ASSERT_VK_SUCCESS(err);

    // Query layout of 3rd plane, for a 2-plane image
    VkImageSubresource subres = {};
    subres.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    subres.mipLevel = 0;
    subres.arrayLayer = 0;
    VkSubresourceLayout layout = {};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-01581");
    vk::GetImageSubresourceLayout(device(), image_2plane, &subres, &layout);
    m_errorMonitor->VerifyFound();

    // Query layout using color aspect, for a 3-plane image
    subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-01582");
    vk::GetImageSubresourceLayout(device(), image_3plane, &subres, &layout);
    m_errorMonitor->VerifyFound();

    // Clean up
    vk::DestroyImage(device(), image_2plane, NULL);
    vk::DestroyImage(device(), image_3plane, NULL);
}

TEST_F(VkLayerTest, InvalidBufferViewObject) {
    // Create a single TEXEL_BUFFER descriptor and send it an invalid bufferView
    // First, cause the bufferView to be invalid due to underlying buffer being destroyed
    // Then destroy view itself and verify that same error is hit
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02994");
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

        err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
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

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // Now destroy view itself and verify same error, which is hit in PV this time
    vk::DestroyBufferView(m_device->device(), view, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02994");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateBufferViewNoMemoryBoundToBuffer) {
    TEST_DESCRIPTION("Attempt to create a buffer view with a buffer that has no memory bound to it.");

    VkResult err;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
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
    err = vk::CreateBuffer(m_device->device(), &buff_ci, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferViewCreateInfo buff_view_ci = {};
    buff_view_ci.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    buff_view_ci.buffer = buffer;
    buff_view_ci.format = VK_FORMAT_R8_UNORM;
    buff_view_ci.range = VK_WHOLE_SIZE;
    VkBufferView buff_view;
    err = vk::CreateBufferView(m_device->device(), &buff_view_ci, NULL, &buff_view);

    m_errorMonitor->VerifyFound();
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    // If last error is success, it still created the view, so delete it.
    if (err == VK_SUCCESS) {
        vk::DestroyBufferView(m_device->device(), buff_view, NULL);
    }
}

TEST_F(VkLayerTest, InvalidBufferViewCreateInfoEntries) {
    TEST_DESCRIPTION("Attempt to create a buffer view with invalid create info.");

    // Attempt to enable texel buffer alignmnet extension
    bool texel_buffer_alignment = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (texel_buffer_alignment) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    texel_buffer_alignment =
        texel_buffer_alignment && DeviceExtensionSupported(gpu(), nullptr, VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME);
    if (texel_buffer_alignment) {
        m_device_extension_names.push_back(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

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
    vk::GetPhysicalDeviceFormatProperties(gpu(), format_with_uniform_texel_support, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        printf("%s Test requires %s to support VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT\n", kSkipPrefix,
               format_with_uniform_texel_support_string);
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
    const char *offset_vuid =
        (texel_buffer_alignment == true) ? "VUID-VkBufferViewCreateInfo-offset-02749" : "VUID-VkBufferViewCreateInfo-offset-00926";
    CreateBufferViewTest(*this, &buff_view_ci, {offset_vuid});

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
    buff_view_ci.range = 2 * static_cast<VkDeviceSize>(format_size) * static_cast<VkDeviceSize>(dev_limits.maxTexelBufferElements);
    CreateBufferViewTest(*this, &buff_view_ci,
                         {"VUID-VkBufferViewCreateInfo-range-00930", "VUID-VkBufferViewCreateInfo-offset-00931"});

    // Create a new test buffer that is larger than VkPhysicalDeviceLimits::maxTexelBufferElements
    // The spec min max is just 64K, but some implementations support a much larger value than that.
    // Skip the test if the limit is very large to not allocate excessive amounts of memory.
    if (dev_limits.maxTexelBufferElements > 64 * 1024 * 1024) {
        printf("%s Test skipped if maxTexelBufferElements is very large. \n", kSkipPrefix);
    } else {
        const VkDeviceSize large_resource_size =
            2 * static_cast<VkDeviceSize>(format_size) * static_cast<VkDeviceSize>(dev_limits.maxTexelBufferElements);
        const VkBufferCreateInfo large_buffer_info =
            VkBufferObj::create_info(large_resource_size, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
        VkBufferObj large_buffer;
        large_buffer.init(*m_device, large_buffer_info, (VkMemoryPropertyFlags)VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Offset must be less than the size of the buffer, so set it equal to the buffer size to cause an error
        buff_view_ci.buffer = large_buffer.handle();
        buff_view_ci.range = VK_WHOLE_SIZE;

        // For VK_WHOLE_SIZE, the buffer size - offset must be less than VkPhysicalDeviceLimits::maxTexelBufferElements
        CreateBufferViewTest(*this, &buff_view_ci, {"VUID-VkBufferViewCreateInfo-range-04059"});
    }

    vk::GetPhysicalDeviceFormatProperties(gpu(), format_without_texel_support, &format_properties);
    if ((format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT) ||
        (format_properties.bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT)) {
        printf(
            "%s Test requires %s to not support VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT nor "
            "VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT\n",
            kSkipPrefix, format_without_texel_support_string);
        return;
    }

    // Set range to acceptable value for buffer tests
    buff_view_ci.buffer = buffer.handle();
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
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
    vk::GetPhysicalDeviceProperties2(gpu(), &pd_props2);

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
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_R32G32B32_SFLOAT, &format_properties);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-renderpass");

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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdUpdateBuffer-renderpass");

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
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), dstBuffer.handle(), dstOffset, dataSize, &Data);

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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseMipLevel-01470");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseMipLevel-01470");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-levelCount-01720");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01692");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-baseArrayLayer-01472");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-layerCount-01721");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-pRanges-01693");
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
        vk::CmdClearColorImage(cb_handle, image.handle(), image.Layout(), &clear_color, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ClearColorImageWithInvalidFormat) {
    TEST_DESCRIPTION("Record clear color with an invalid image formats");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

    VkImageObj mp_image(m_device);
    VkFormat mp_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = mp_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.arrayLayers = 1;

    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), image_create_info, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT);
    if (supported == false) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }

    mp_image.init(&image_create_info);
    m_commandBuffer->begin();

    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-image-01545");
    m_commandBuffer->ClearColorImage(mp_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearDepthStencilWithBadAspect) {
    TEST_DESCRIPTION("Verify ClearDepth with an invalid VkImageAspectFlags.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool separate_stencil_usage_supported =
        DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    if (separate_stencil_usage_supported) {
        m_device_extension_names.push_back(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    }

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    const VkClearDepthStencilValue clear_value = {};
    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1};

    m_commandBuffer->begin();

    if (!separate_stencil_usage_supported) {
        printf("%s VK_EXT_separate_stencil_usage Extension not supported, skipping part of test\n", kSkipPrefix);
    } else {
        VkImageStencilUsageCreateInfoEXT image_stencil_create_info = {};
        image_stencil_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO_EXT;
        image_stencil_create_info.pNext = nullptr;
        image_stencil_create_info.stencilUsage =
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // not VK_IMAGE_USAGE_TRANSFER_DST_BIT

        image_create_info.pNext = &image_stencil_create_info;

        VkImageObj image(m_device);
        image.init(&image_create_info);
        ASSERT_TRUE(image.initialized());

        // Element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT, and image was created with separate stencil usage,
        // VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageStencilUsageCreateInfo::stencilUsage used to create image
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02658");
        // ... since VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageCreateInfo::usage used to create image
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-image-00009");
        vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    image_create_info.pNext = nullptr;

    range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());

    // Element of pRanges.aspect includes VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the
    // VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02659");
    // Element of pRanges.aspect includes VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the
    // VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02660");
    // ... since VK_IMAGE_USAGE_TRANSFER_DST_BIT not included in the VkImageCreateInfo::usage used to create image
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-image-00009");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image.handle(), image.Layout(), &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    // Using stencil aspect when format only have depth
    const VkFormat depth_only_format = FindSupportedDepthOnlyFormat(gpu());
    if (depth_only_format != VK_FORMAT_UNDEFINED) {
        VkImageObj depth_image(m_device);
        image_create_info.format = depth_only_format;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        depth_image.init(&image_create_info);
        ASSERT_TRUE(depth_image.initialized());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-image-02825");
        vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), depth_image.handle(), depth_image.Layout(), &clear_value, 1,
                                      &range);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->end();
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        const VkImageSubresourceRange range = {ds_aspect, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseMipLevel-01474");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 1, 1, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try levelCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-levelCount-01720");
        const VkImageSubresourceRange range = {ds_aspect, 0, 0, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01694");
        const VkImageSubresourceRange range = {ds_aspect, 0, 2, 0, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-baseArrayLayer-01476");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 1, 1};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try layerCount = 0
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-layerCount-01721");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 0};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }

    // Try baseArrayLayer + layerCount > image.arrayLayers
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-01695");
        const VkImageSubresourceRange range = {ds_aspect, 0, 1, 0, 2};
        vk::CmdClearDepthStencilImage(cb_handle, image.handle(), image.Layout(), &clear_value, 1, &range);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ClearColorImageWithinRenderPass) {
    // Call CmdClearColorImage within an active RenderPass
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-renderpass");

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

    vk::CmdClearColorImage(m_commandBuffer->handle(), dstImage.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &range);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ClearDepthStencilImageErrors) {
    // Hit errors related to vk::CmdClearDepthStencilImage()
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
    // need to handle since an element of pRanges includes VK_IMAGE_ASPECT_DEPTH_BIT without VkImageCreateInfo::usage having
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT being set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-pRanges-02660");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-image-00009");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), dst_image_bad_usage.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &range);
    m_errorMonitor->VerifyFound();

    // Fix usage for next test case
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj dst_image(m_device);
    dst_image.init(&image_create_info);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-renderpass");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &range);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BufferMemoryBarrierNoBuffer) {
    // Try to add a buffer memory barrier with no buffer.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
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
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                           nullptr, 1, &buf_barrier, 0, nullptr);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidBarriers) {
    TEST_DESCRIPTION("A variety of ways to get VK_INVALID_BARRIER ");

    // Make sure extensions for multi-planar and separate depth stencil images are enabled if possible
    bool mp_extensions = true;
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        mp_extensions = false;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    }
    bool separate_ds_layouts = false;
    if (rp2Supported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
        separate_ds_layouts = true;
    }
    bool maintaince2 = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    if (maintaince2) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    }
    // Check for external memory device extensions
    bool external_memory = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        external_memory = true;
    }

    // Set separate depth stencil feature bit
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    auto separate_depth_stencil_layouts_features = lvl_init_struct<VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&separate_depth_stencil_layouts_features);
    if (vkGetPhysicalDeviceFeatures2KHR) {
        vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    } else {
        separate_depth_stencil_layouts_features.separateDepthStencilLayouts = VK_FALSE;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, (vkGetPhysicalDeviceFeatures2KHR) ? &features2 : nullptr));

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
    vk::CmdEndRenderPass(m_commandBuffer->handle());

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
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                           img_barriers);
    m_errorMonitor->VerifyNotFound();

    // Duplication of layout transitions (not from undefined) are not valid
    img_barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barriers[1].oldLayout = img_barriers[0].oldLayout;
    img_barriers[1].newLayout = img_barriers[0].newLayout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-01197");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 2,
                           img_barriers);
    m_errorMonitor->VerifyFound();

    if (!external_memory) {
        printf("%s External memory extension not supported, skipping external queue family subcase\n", kSkipPrefix);
    } else {
        // Transitions to and from EXTERNAL within the same command buffer are valid, if pointless.
        m_errorMonitor->ExpectSuccess();
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.srcQueueFamilyIndex = submit_family;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        img_barrier.dstAccessMask = 0;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &img_barrier);
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.dstQueueFamilyIndex = submit_family;
        img_barrier.srcAccessMask = 0;
        img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               0, 0, nullptr, 0, nullptr, 1, &img_barrier);
        m_errorMonitor->VerifyNotFound();
    }

    // Exceed the buffer size
    conc_test.buffer_barrier_.offset = conc_test.buffer_.create_info().size + 1;
    conc_test("", "VUID-VkBufferMemoryBarrier-offset-01187");

    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = conc_test.buffer_.create_info().size + 1;
    // Size greater than total size
    conc_test("", "VUID-VkBufferMemoryBarrier-size-01189");

    conc_test.buffer_barrier_.size = 0;
    // Size is zero
    conc_test("", "VUID-VkBufferMemoryBarrier-size-01188");

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    if (separate_depth_stencil_layouts_features.separateDepthStencilLayouts) {
        conc_test("VUID-VkImageMemoryBarrier-image-03319");
    } else {
        const char *vuid =
            (separate_ds_layouts == true) ? "VUID-VkImageMemoryBarrier-image-03320" : "VUID-VkImageMemoryBarrier-image-01207";
        conc_test(vuid);

        // Having only one of depth or stencil set for DS image is an error
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        conc_test(vuid);
    }

    // Having anything other than DEPTH and STENCIL is an error
    conc_test.image_barrier_.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
    conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");

    // Now test depth-only
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_D16_UNORM, &format_props);
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
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_S8_UINT, &format_props);
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

    const char *color_vuid = (mp_extensions) ? "VUID-VkImageMemoryBarrier-image-01671" : "VUID-VkImageMemoryBarrier-image-02902";

    // COLOR bit must be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test(color_vuid);

    // No bits other than COLOR may be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test(color_vuid);

    // Test multip-planar image
    if (mp_extensions) {
        PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;
        PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2Function = nullptr;
        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vkBindImageMemory2Function = vk::BindImageMemory2;
            vkGetImageMemoryRequirements2Function = vk::GetImageMemoryRequirements2;
        } else {
            vkBindImageMemory2Function =
                (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
            vkGetImageMemoryRequirements2Function =
                (PFN_vkGetImageMemoryRequirements2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkGetImageMemoryRequirements2KHR");
        }

        VkFormatProperties format_properties;
        VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &format_properties);
        if (0 !=
            (format_properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.pNext = NULL;
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

            VkImage mp_image;
            VkDeviceMemory plane_0_memory;
            VkDeviceMemory plane_1_memory;
            ASSERT_VK_SUCCESS(vk::CreateImage(m_device->device(), &image_create_info, NULL, &mp_image));

            VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

            VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
            mem_req_info2.pNext = &image_plane_req;
            mem_req_info2.image = mp_image;
            VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

            // Find a valid memory type index to memory to be allocated from
            VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &plane_0_memory));

            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &plane_1_memory));

            VkBindImagePlaneMemoryInfo plane_0_memory_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO};
            plane_0_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
            VkBindImagePlaneMemoryInfo plane_1_memory_info = {VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO};
            plane_1_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
            bind_image_info[0].pNext = &plane_0_memory_info;
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = plane_0_memory;
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1] = bind_image_info[0];
            bind_image_info[1].pNext = &plane_1_memory_info;
            bind_image_info[1].memory = plane_1_memory;
            vkBindImageMemory2Function(device(), 2, bind_image_info);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.image = mp_image;

            // Test valid usage first
            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
            conc_test("", "", VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, true);

            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
            conc_test("VUID-VkImageMemoryBarrier-image-01672");

            conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
            conc_test("VUID-VkImageMemoryBarrier-image-01673");

            vk::FreeMemory(device(), plane_0_memory, NULL);
            vk::FreeMemory(device(), plane_1_memory, NULL);
            vk::DestroyImage(m_device->device(), mp_image, nullptr);
        }
    }

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
            // images _without_ VK_KHR_maintenance2 added layouts
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01658"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier-oldLayout-01659"},
            // clang-format on
        };
        const uint32_t layout_count = sizeof(bad_buffer_layouts) / sizeof(bad_buffer_layouts[0]);

        for (uint32_t i = 0; i < layout_count; ++i) {
            const VkImageLayout bad_layout = bad_buffer_layouts[i].bad_layout;
            // Skip layouts that require maintaince2 support
            if ((maintaince2 == false) && ((bad_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
                                           (bad_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL))) {
                continue;
            }
            conc_test.image_barrier_.image = bad_buffer_layouts[i].image_obj.handle();
            const VkImageUsageFlags usage = bad_buffer_layouts[i].image_obj.usage();
            conc_test.image_barrier_.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                                       ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                                       : VK_IMAGE_ASPECT_COLOR_BIT;

            conc_test.image_barrier_.oldLayout = bad_layout;
            conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test(bad_buffer_layouts[i].msg_code);

            conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            conc_test.image_barrier_.newLayout = bad_layout;
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
    conc_test("", "VUID-vkCmdPipelineBarrier-srcAccessMask-02815");

    // Attempt barrier where dstAccessMask is not supported by dstStageMask
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    conc_test("", "VUID-vkCmdPipelineBarrier-dstAccessMask-02816");

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-4098");

    VkCommandPoolObj command_pool(m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj bad_command_buffer(m_device, &command_pool);

    bad_command_buffer.begin();
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    // Set two bits that should both be supported as a bonus positive check
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    vk::CmdPipelineBarrier(bad_command_buffer.handle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buf_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();

    // Check for error for trying to wait on pipeline stage not supported by this queue. Specifically since our queue is not a
    // compute queue, vk::CmdWaitEvents cannot have it's source stage mask be VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-4098");
    VkEvent event;
    VkEventCreateInfo event_create_info{};
    event_create_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);
    vk::CmdWaitEvents(bad_command_buffer.handle(), 1, &event, /*source stage mask*/ VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                      VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    bad_command_buffer.end();

    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, InvalidBarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families");
    SetTargetApiVersion(VK_API_VERSION_1_0);
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

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
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

        // core_validation::barrier_queue_families::kSrcAndDstBothValid
        excl_test("VUID-VkImageMemoryBarrier-image-04069", "VUID-VkBufferMemoryBarrier-buffer-04086", VK_QUEUE_FAMILY_IGNORED,
                  submit_family);
        excl_test("VUID-VkImageMemoryBarrier-image-04069", "VUID-VkBufferMemoryBarrier-buffer-04086", submit_family,
                  VK_QUEUE_FAMILY_IGNORED);
        // true -> positive test
        excl_test("VUID-VkImageMemoryBarrier-image-04069", "VUID-VkBufferMemoryBarrier-buffer-04086", submit_family, submit_family,
                  true);
        excl_test("VUID-VkImageMemoryBarrier-image-04069", "VUID-VkBufferMemoryBarrier-buffer-04086", VK_QUEUE_FAMILY_IGNORED,
                  VK_QUEUE_FAMILY_IGNORED, true);
    }

    if (only_one_family) {
        printf("%s Single queue family found -- VK_SHARING_MODE_EXCLUSIVE submit testcases skipped.\n", kSkipPrefix);
    } else {
        BarrierQueueFamilyTestHelper excl_test(&test_context);
        excl_test.Init(nullptr);

        // core_validation::barrier_queue_families::kSubmitQueueMustMatchSrcOrDst
        excl_test("UNASSIGNED-CoreValidation-vkImageMemoryBarrier-sharing-mode-exclusive-same-family",
                  "UNASSIGNED-CoreValidation-vkBufferMemoryBarrier-sharing-mode-exclusive-same-family", other_family, other_family,
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
        conc_test("VUID-VkImageMemoryBarrier-image-04071", "VUID-VkBufferMemoryBarrier-buffer-04088", submit_family,
                  VK_QUEUE_FAMILY_IGNORED);
        conc_test("VUID-VkImageMemoryBarrier-image-04071", "VUID-VkBufferMemoryBarrier-buffer-04088", VK_QUEUE_FAMILY_IGNORED,
                  submit_family);
        // This is to flag the errors that would be considered only "unexpected" in the parallel case above
        // true -> positive test
        conc_test("VUID-VkImageMemoryBarrier-image-04071", "VUID-VkBufferMemoryBarrier-buffer-04088", VK_QUEUE_FAMILY_IGNORED,
                  VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
        conc_test("VUID-VkImageMemoryBarrier-image-04071", "VUID-VkBufferMemoryBarrier-buffer-04088", VK_QUEUE_FAMILY_EXTERNAL_KHR,
                  VK_QUEUE_FAMILY_IGNORED, true);
    }

    BarrierQueueFamilyTestHelper excl_test(&test_context);
    excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

    // core_validation::barrier_queue_families::kSrcAndDstValidOrSpecial
    excl_test("VUID-VkImageMemoryBarrier-image-04072", "VUID-VkBufferMemoryBarrier-buffer-04089", submit_family, invalid);
    excl_test("VUID-VkImageMemoryBarrier-image-04072", "VUID-VkBufferMemoryBarrier-buffer-04089", invalid, submit_family);
    // true -> positive test
    excl_test("VUID-VkImageMemoryBarrier-image-04072", "VUID-VkBufferMemoryBarrier-buffer-04089", submit_family, submit_family,
              true);
    excl_test("VUID-VkImageMemoryBarrier-image-04072", "VUID-VkBufferMemoryBarrier-buffer-04089", submit_family,
              VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
    excl_test("VUID-VkImageMemoryBarrier-image-04072", "VUID-VkBufferMemoryBarrier-buffer-04089", VK_QUEUE_FAMILY_EXTERNAL_KHR,
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

    // try for vk::CmdPipelineBarrier
    {
        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01486");
        }

        // Try levelCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            conc_test("VUID-VkImageSubresourceRange-levelCount-01720");
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
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01488");
        }

        // Try layerCount = 0
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            conc_test("VUID-VkImageSubresourceRange-layerCount-01721");
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            conc_test.image_barrier_.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            conc_test("VUID-VkImageMemoryBarrier-subresourceRange-01725");
        }
    }

    m_commandBuffer->begin();
    // try for vk::CmdWaitEvents
    {
        VkEvent event;
        VkEventCreateInfo eci{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO, NULL, 0};
        VkResult err = vk::CreateEvent(m_device->handle(), &eci, nullptr, &event);
        ASSERT_VK_SUCCESS(err);

        // Try baseMipLevel >= image.mipLevels with VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01486");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_REMAINING_MIP_LEVELS, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel >= image.mipLevels without VK_REMAINING_MIP_LEVELS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01486");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try levelCount = 0
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-levelCount-01720");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseMipLevel + levelCount > image.mipLevels
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01724");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01488");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01488");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try layerCount = 0
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageSubresourceRange-layerCount-01721");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-subresourceRange-01725");
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            VkImageMemoryBarrier img_barrier = img_barrier_template;
            img_barrier.subresourceRange = range;
            vk::CmdWaitEvents(m_commandBuffer->handle(), 1, &event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
            m_errorMonitor->VerifyFound();
        }

        vk::DestroyEvent(m_device->handle(), event, nullptr);
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

    // vk::CmdBindPipeline(m_commandBuffer->handle(),
    // VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // Should error before calling to driver so don't care about actual data
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "vkCmdBindIndexBuffer() offset (0x7) does not fall on ");
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), buffer.handle(), 7, VK_INDEX_TYPE_UINT16);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Bad2DArrayImageType) {
    TEST_DESCRIPTION("Create an image with a flag specifying 2D_ARRAY_COMPATIBLE but not of imageType 3D.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");

    {
        // Create and bind a vertex buffer in a reduced scope, which will cause it to be deleted upon leaving this scope
        const float vbo_data[3] = {1.f, 0.f, 1.f};
        VkVerticesObj draw_verticies(m_device, 1, 1, sizeof(vbo_data[0]), sizeof(vbo_data) / sizeof(vbo_data[0]), vbo_data);
        draw_verticies.BindVertexBuffers(m_commandBuffer->handle());
        draw_verticies.AddVertexInputToPipeHelpr(&pipe);

        m_commandBuffer->Draw(1, 0, 0, 0);

        m_commandBuffer->EndRenderPass();
    }

    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    {
        // Create and bind a vertex buffer in a reduced scope, and delete it
        // twice, the second through the destructor
        VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VkBufferTest::eDoubleDelete);
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyBuffer-buffer-parameter");
        buffer_test.TestDoubleDestroy();
    }
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetUnexpectedError("value of pCreateInfo->usage must not be 0");
    if (VkBufferTest::GetTestConditionValid(m_device, VkBufferTest::eInvalidMemoryOffset)) {
        // Create and bind a memory buffer with an invalid offset.
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memoryOffset-01036");
        m_errorMonitor->SetUnexpectedError(
            "If buffer was created with the VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT or VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT, "
            "memoryOffset must be a multiple of VkPhysicalDeviceLimits::minTexelBufferOffsetAlignment");
        VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VkBufferTest::eInvalidMemoryOffset);
        (void)buffer_test;
        m_errorMonitor->VerifyFound();
    }

    {
        // Attempt to bind a null buffer.
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "vkBindBufferMemory: required parameter buffer specified as VK_NULL_HANDLE");
        VkBufferTest buffer_test(m_device, 0, VkBufferTest::eBindNullBuffer);
        (void)buffer_test;
        m_errorMonitor->VerifyFound();
    }

    {
        // Attempt to bind a fake buffer.
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-buffer-parameter");
        VkBufferTest buffer_test(m_device, 0, VkBufferTest::eBindFakeBuffer);
        (void)buffer_test;
        m_errorMonitor->VerifyFound();
    }

    {
        // Attempt to use an invalid handle to delete a buffer.
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeMemory-memory-parameter");
        VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VkBufferTest::eFreeInvalidHandle);
        (void)buffer_test;
    }
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BadVertexBufferOffset) {
    TEST_DESCRIPTION("Submit an offset past the end of a vertex buffer");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    const uint32_t maxVertexInputBindings = m_device->props.limits.maxVertexInputBindings;
    static const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void *)&vbo_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers-pOffsets-00626");
    m_commandBuffer->BindVertexBuffer(&vbo, (VkDeviceSize)(3 * sizeof(float)), 1);  // Offset at the end of the buffer
    m_errorMonitor->VerifyFound();

    // firstBinding set over limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers-firstBinding-00624");
    m_commandBuffer->BindVertexBuffer(&vbo, 0, (maxVertexInputBindings + 1));
    m_errorMonitor->VerifyFound();

    // sum of firstBinding and bindingCount set over limit
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers-firstBinding-00625");
    m_commandBuffer->BindVertexBuffer(&vbo, 0, (maxVertexInputBindings));  // bindingCount of 1 puts it over limit
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, BadIndexBufferOffset) {
    TEST_DESCRIPTION("Submit bad offsets binding the index buffer");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    static const uint32_t ibo_data[3] = {0, 1, 2};
    VkConstantBufferObj ibo(m_device, sizeof(ibo_data), (const void *)&ibo_data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), ibo.handle(), &mem_reqs);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Set offset over buffer size
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindIndexBuffer-offset-00431");
    m_commandBuffer->BindIndexBuffer(&ibo, mem_reqs.size + sizeof(uint32_t), VK_INDEX_TYPE_UINT32);
    m_errorMonitor->VerifyFound();

    // Set offset to be misaligned with index buffer UINT32 type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindIndexBuffer-offset-00432");
    m_commandBuffer->BindIndexBuffer(&ibo, 1, VK_INDEX_TYPE_UINT32);
    m_errorMonitor->VerifyFound();

    // Test for missing pNext struct for index buffer UINT8 type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindIndexBuffer-indexType-02765");
    m_commandBuffer->BindIndexBuffer(&ibo, 1, VK_INDEX_TYPE_UINT8_EXT);
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

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImageLayout-00128");
    m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    // Final src error is due to bad layout type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-srcImageLayout-00129");
    m_errorMonitor->SetUnexpectedError(
        "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    // Now verify same checks for dst
    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
    m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    // Now cause error due to src image layout changing
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImageLayout-00133");
    m_errorMonitor->SetUnexpectedError(
        "is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL.");
    m_commandBuffer->CopyImage(src_image.handle(), VK_IMAGE_LAYOUT_GENERAL, dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyImage-dstImageLayout-00134");
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
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, transfer_dst_image_barrier);
    transfer_dst_image_barrier[0].image = depth_image.handle();
    transfer_dst_image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-00005");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-00004");
    m_commandBuffer->ClearColorImage(dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for color clear.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-00004");
    m_commandBuffer->ClearColorImage(dst_image.handle(), VK_IMAGE_LAYOUT_GENERAL, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();

    VkClearDepthStencilValue depth_clear_value = {};
    clear_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // Fail due to explicitly prohibited layout for depth clear (only GENERAL and TRANSFER_DST are permitted).
    // Since the image is currently not in UNDEFINED layout, this will emit two errors.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-imageLayout-00012");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
    m_commandBuffer->ClearDepthStencilImage(depth_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &depth_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
    // Fail due to provided layout not matching actual current layout for depth clear.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearDepthStencilImage-imageLayout-00011");
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-01197");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier-oldLayout-01210");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, image_barrier);
    m_errorMonitor->VerifyFound();

    // Finally some layout errors at RenderPass create time
    // Just hacking in specific state to get to the errors we want so don't copy this unless you know what you're doing.
    VkAttachmentReference attach = {};
    VkSubpassDescription subpass = {};
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &attach;
    VkRenderPassCreateInfo rpci = {};
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkRenderPass rp;
    // error w/ non-general layout
    attach.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-None-04437");
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();

    subpass.inputAttachmentCount = 0;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    // error w/ non-color opt or GENERAL layout for color attachment
    attach.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-None-04437");
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();

    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &attach;
    attach_desc.format = VK_FORMAT_D16_UNORM;
    // error w/ non-ds opt or GENERAL layout for color attachment
    attach.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-None-04437");
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription-format-03283");
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidStorageImageLayout) {
    TEST_DESCRIPTION("Attempt to update a STORAGE_IMAGE descriptor w/o GENERAL layout.");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkFormat tex_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageTiling tiling;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), tex_format, &format_properties);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-04152");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ClearColorImageInvalidImageLayout) {
    TEST_DESCRIPTION("Check ClearImage layouts with SHARED_PRESENTABLE_IMAGE extension active.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    } else {
        printf("%s %s is not supported on this platform, skipping test.\n", kSkipPrefix,
               VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageObj dst_image(m_device);
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
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.flags = 0;

    dst_image.init(&image_create_info);
    m_commandBuffer->begin();

    VkClearColorValue color_clear_value = {};
    VkImageSubresourceRange clear_range;
    clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clear_range.baseMipLevel = 0;
    clear_range.baseArrayLayer = 0;
    clear_range.layerCount = 1;
    clear_range.levelCount = 1;

    // Fail by using bad layout for color clear (GENERAL, SHARED_PRESENT or TRANSFER_DST are permitted).
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdClearColorImage-imageLayout-01394");
    m_commandBuffer->ClearColorImage(dst_image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, &color_clear_value, 1, &clear_range);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateImageViewBreaksParameterCompatibilityRequirements) {
    TEST_DESCRIPTION(
        "Attempts to create an Image View with a view type that does not match the image type it is being created from.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties memProps;
    vk::GetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &memProps);

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
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(
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
    res = vk::CreateImage(m_device->device(), &imgInfo, NULL, &imageSparse);
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
    vk::DestroyImage(m_device->device(), imageSparse, nullptr);
}

TEST_F(VkLayerTest, CreateImageViewFormatFeatureMismatch) {
    TEST_DESCRIPTION("Create view with a format that does not have the same features as the image format.");

    // Used to force format to have feature bits to enable test can run on any device
    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
    }

    uint32_t feature_count = 5;
    // List of features to be tested
    VkFormatFeatureFlagBits features[] = {
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,            // 02274
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,         // 02652 - only need one of 2 features
        VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,            // 02275
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,         // 02276
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT  // 02277
    };
    // List of usage cases for each feature test
    VkImageUsageFlags usages[] = {
        VK_IMAGE_USAGE_SAMPLED_BIT,                  // 02274
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,         // 02652
        VK_IMAGE_USAGE_STORAGE_BIT,                  // 02275
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,         // 02276
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  // 02277
    };
    // List of errors that will be thrown in order of tests run
    // Order is done to make sure adjacent format features are different
    std::string optimal_error_codes[] = {
        "VUID-VkImageViewCreateInfo-usage-02274", "VUID-VkImageViewCreateInfo-usage-02652",
        "VUID-VkImageViewCreateInfo-usage-02275", "VUID-VkImageViewCreateInfo-usage-02276",
        "VUID-VkImageViewCreateInfo-usage-02277",  // Needs to be last since needs special format
    };

    VkFormatProperties formatProps;

    // All but one test in this loop and do last test after for special format case
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

TEST_F(VkLayerTest, CreateImageViewNoSeparateStencilUsage) {
    TEST_DESCRIPTION("Verify CreateImageView create info for the case VK_EXT_separate_stencil_usage is not supported.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    // without VK_EXT_separate_stencil_usage explicitly enabled
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageSubresourceRange range = {aspect, 0, 1, 0, 1};

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = nullptr;
    image_view_create_info.flags = 0;
    image_view_create_info.image = VK_NULL_HANDLE;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = depth_format;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    image_view_create_info.subresourceRange = range;

    VkImageViewUsageCreateInfo image_view_usage_create_info = {};
    image_view_usage_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
    image_view_usage_create_info.pNext = nullptr;
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    image_view_create_info.pNext = &image_view_usage_create_info;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());
    image_view_create_info.image = image.handle();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // Extra flag

    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in VkImageCreateInfo::usage
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewCreateInfo-pNext-02661");
}

TEST_F(VkLayerTest, CreateImageViewStencilUsageCreateInfo) {
    TEST_DESCRIPTION("Verify CreateImageView with stencil usage.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE2_EXTENSION_NAME)) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME)) {
        printf("%s VK_EXT_separate_stencil_usage Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageSubresourceRange range = {aspect, 0, 1, 0, 1};

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_format;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = nullptr;
    image_view_create_info.flags = 0;
    image_view_create_info.image = VK_NULL_HANDLE;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = depth_format;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    image_view_create_info.subresourceRange = range;

    VkImageViewUsageCreateInfo image_view_usage_create_info = {};
    image_view_usage_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
    image_view_usage_create_info.pNext = nullptr;
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    image_view_create_info.pNext = &image_view_usage_create_info;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());
    image_view_create_info.image = image.handle();

    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // Extra flag

    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in VkImageCreateInfo::usage
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewCreateInfo-pNext-02662");

    VkImageStencilUsageCreateInfoEXT image_stencil_create_info = {};
    image_stencil_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO_EXT;
    image_stencil_create_info.pNext = nullptr;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_SAMPLED_BIT;

    image_create_info.pNext = &image_stencil_create_info;

    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // Extra flag
    image_view_create_info.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;  // Flag other than VK_IMAGE_ASPECT_STENCIL_BIT

    VkImageObj image2(m_device);
    image2.init(&image_create_info);
    ASSERT_TRUE(image2.initialized());
    image_view_create_info.image = image2.handle();

    VkImageView view = VK_NULL_HANDLE;
    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in
    // VkImageStencilUsageCreateInfo::stencilUsage
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-02663");
    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in VkImageCreateInfo::usage
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-02664");
    VkResult err = vk::CreateImageView(m_device->device(), &image_view_create_info, nullptr, &view);
    m_errorMonitor->VerifyFound();
    if (VK_SUCCESS == err) {
        vk::DestroyImageView(m_device->device(), view, nullptr);
    }
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

    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
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
    vk::DestroyImage(m_device->device(), image, NULL);
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

    CreateImageViewTest(*this, &image_view_create_info, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-image-00996");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-aspectMask-00997");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-mipLevel-01716");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
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

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-arrayLayer-01717");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Load required functions
    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT =
        (PFN_vkSetPhysicalDeviceFormatPropertiesEXT)vk::GetInstanceProcAddr(instance(), "vkSetPhysicalDeviceFormatPropertiesEXT");
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT =
        (PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT)vk::GetInstanceProcAddr(
            instance(), "vkGetOriginalPhysicalDeviceFormatPropertiesEXT");

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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    if (!(m_device->format_properties(VK_FORMAT_R8_UINT).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)) {
        printf("%s Device does not support R8_UINT as color attachment; skipped", kSkipPrefix);
        return;
    }

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   nullptr,
                                   0,
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

    imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    VkImageObj mutImage(m_device);
    mutImage.init(&imageInfo);
    ASSERT_TRUE(mutImage.initialized());

    VkImageViewCreateInfo imgViewInfo = {};
    imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;  // different than createImage
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewInfo.image = mutImage.handle();

    // Create mutable format image that is not compatiable
    bool ycbcr_support = (DeviceExtensionEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) ||
                          (DeviceValidationVersion() >= VK_API_VERSION_1_1));
    bool maintenance2_support =
        (DeviceExtensionEnabled(VK_KHR_MAINTENANCE2_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));
    const char *error_vuid;
    if ((!maintenance2_support) && (!ycbcr_support)) {
        error_vuid = "VUID-VkImageViewCreateInfo-image-01018";
    } else if ((maintenance2_support) && (!ycbcr_support)) {
        error_vuid = "VUID-VkImageViewCreateInfo-image-01759";
    } else if ((!maintenance2_support) && (ycbcr_support)) {
        error_vuid = "VUID-VkImageViewCreateInfo-image-01760";
    } else {
        // both enabled
        error_vuid = "VUID-VkImageViewCreateInfo-image-01761";
    }
    CreateImageViewTest(*this, &imgViewInfo, error_vuid);

    // Use CUBE_ARRAY without feature enabled
    if (device_features.imageCubeArray == false) {
        VkImageCreateInfo cubeImageInfo = imageInfo;
        cubeImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        VkImageObj cubeImage(m_device);
        cubeImage.init(&cubeImageInfo);
        ASSERT_TRUE(cubeImage.initialized());

        VkImageViewCreateInfo cubeImgViewInfo = imgViewInfo;
        cubeImgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        cubeImgViewInfo.format = VK_FORMAT_R8_UINT;  // compatiable format
        cubeImgViewInfo.image = cubeImage.handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-viewType-02961");
        CreateImageViewTest(*this, &cubeImgViewInfo, "VUID-VkImageViewCreateInfo-viewType-01004");
    }
}

TEST_F(VkLayerTest, MultiplaneIncompatibleViewFormat) {
    TEST_DESCRIPTION("Postive/negative tests of multiplane imageview format compatibility");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

    const VkFormatFeatureFlags features = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    // Verify format 3 Plane format
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
    } else {
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

    ci.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    supported = ImageFormatAndFeaturesSupported(instance(), gpu(), ci, features);
    // Verify format 2 Plane format
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
    } else {
        VkImageObj image_obj(m_device);
        image_obj.init(&ci);
        ASSERT_TRUE(image_obj.initialized());

        VkImageViewCreateInfo ivci = {};
        ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image = image_obj.image();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;

        // Plane 0 is compatible with VK_FORMAT_R8_UNORM
        // Plane 1 is compatible with VK_FORMAT_R8G8_UNORM

        // Correct format succeeds
        ivci.format = VK_FORMAT_R8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        CreateImageViewTest(*this, &ivci);

        ivci.format = VK_FORMAT_R8G8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        CreateImageViewTest(*this, &ivci);

        // Incompatible format error
        ivci.format = VK_FORMAT_R8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT;
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01586");

        ivci.format = VK_FORMAT_R8G8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01586");
    }
}

TEST_F(VkLayerTest, CreateImageViewInvalidSubresourceRange) {
    TEST_DESCRIPTION("Passing bad image subrange to CreateImageView");

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01478");
    }

    // Try levelCount = 0
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageSubresourceRange-levelCount-01720");
    }

    // Try baseMipLevel + levelCount > image.mipLevels
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, 0, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
    }

    // These tests rely on having the Maintenance1 extension not being enabled, and are invalid on all but version 1.0
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
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
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-01719");
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01480");
        }

        // Try layerCount = 0
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0};
            VkImageViewCreateInfo img_view_info = img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageSubresourceRange-layerCount-01721");
        }

        // Try baseArrayLayer + layerCount > image.arrayLayers
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
            VkImageViewCreateInfo img_view_info = img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01719");
        }
    }

    {
        VkImageObj cubeArrayImg(m_device);
        auto image_ci = vk_testing::Image::create_info();
        image_ci.arrayLayers = 18;
        image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        cubeArrayImg.init(&image_ci);

        VkImageViewCreateInfo cube_img_view_info_template = {};
        cube_img_view_info_template.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        cube_img_view_info_template.image = cubeArrayImg.handle();
        cube_img_view_info_template.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        cube_img_view_info_template.format = cubeArrayImg.format();
        // subresourceRange to be filled later for the purposes of this test
        cube_img_view_info_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cube_img_view_info_template.subresourceRange.baseMipLevel = 0;
        cube_img_view_info_template.subresourceRange.levelCount = 0;
        cube_img_view_info_template.subresourceRange.baseArrayLayer = 0;
        cube_img_view_info_template.subresourceRange.layerCount = 0;

        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info);
        }
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 5};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02960");
        }
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 12, VK_REMAINING_ARRAY_LAYERS};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info);
        }
        {
            const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 6, VK_REMAINING_ARRAY_LAYERS};
            VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
            img_view_info.subresourceRange = range;
            CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02962");
        }

        if (device_features.imageCubeArray == VK_TRUE) {
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 12};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 13};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02961");
            }
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 6, VK_REMAINING_ARRAY_LAYERS};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 11, VK_REMAINING_ARRAY_LAYERS};
                VkImageViewCreateInfo img_view_info = cube_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-viewType-02963");
            }
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-usage-00963");
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

    // Storage usage can't be multisample if feature not set
    {
        // Feature should not have been set for these tests
        ASSERT_TRUE(features.shaderStorageImageMultisample == VK_FALSE);
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_STORAGE_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00968");
    }
}

TEST_F(VkLayerTest, CreateImageMinLimitsViolation) {
    TEST_DESCRIPTION("Create invalid image with invalid parameters violation minimum limit, such as being zero.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkImage null_image;  // throwaway target for all the vk::CreateImage

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

    for (std::underlying_type<Dimension>::type bad_dimensions = 0x1; bad_dimensions < 0x8; ++bad_dimensions) {
        VkExtent3D extent = {1, 1, 1};

        if (bad_dimensions & kWidth) {
            extent.width = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-00944");
        }

        if (bad_dimensions & kHeight) {
            extent.height = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-00945");
        }

        if (bad_dimensions & kDepth) {
            extent.depth = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-00946");
        }

        VkImageCreateInfo bad_image_ci = safe_image_ci;
        bad_image_ci.imageType = VK_IMAGE_TYPE_3D;  // has to be 3D otherwise it might trigger the non-1 error instead
        bad_image_ci.extent = extent;

        vk::CreateImage(m_device->device(), &bad_image_ci, NULL, &null_image);

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

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

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
        image_ci.imageType = VK_IMAGE_TYPE_3D;

        VkImageFormatProperties img_limits;
        ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));

        image_ci.extent = {img_limits.maxExtent.width + 1, 1, 1};
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-extent-02252");

        image_ci.extent = {1, img_limits.maxExtent.height + 1, 1};
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-extent-02253");

        image_ci.extent = {1, 1, img_limits.maxExtent.depth + 1};
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-extent-02254");
    }

    {
        VkImageCreateInfo image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  // (any attachment bit)

        VkImageFormatProperties img_limits;
        ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));

        if (dev_limits.maxFramebufferWidth != UINT32_MAX) {
            image_ci.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
            if (dev_limits.maxFramebufferWidth + 1 > img_limits.maxExtent.width) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02252");
            }
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-00964");
        } else {
            printf("%s VkPhysicalDeviceLimits::maxFramebufferWidth is already UINT32_MAX; skipping part of test.\n", kSkipPrefix);
        }

        if (dev_limits.maxFramebufferHeight != UINT32_MAX) {
            image_ci.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;  // try different one too
            image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
            if (dev_limits.maxFramebufferHeight + 1 > img_limits.maxExtent.height) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02253");
            }
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
            if (dev_limits.maxFramebufferWidth + 1 > img_limits.maxExtent.width) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02252");
            }
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-02559");

            image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
            if (dev_limits.maxFramebufferHeight + 1 > img_limits.maxExtent.height) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02253");
            }
            CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-usage-02560");
        }
    }
}

TEST_F(VkLayerTest, SamplerImageViewFormatUnsupportedFilter) {
    TEST_DESCRIPTION(
        "Create sampler with a filter and use with image view using a format that does not support the sampler filter.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool cubic_support = false;
    if (DeviceExtensionSupported(gpu(), nullptr, "VK_IMG_filter_cubic")) {
        m_device_extension_names.push_back("VK_IMG_filter_cubic");
        cubic_support = true;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, 0));

    enum FormatTypes { FLOAT, SINT, UINT };

    struct TestFilterType {
        VkFilter filter = VK_FILTER_LINEAR;
        VkFormatFeatureFlagBits required_format_feature = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        VkImageTiling tiling = VK_IMAGE_TILING_LINEAR;
        VkFormat format = VK_FORMAT_UNDEFINED;
        FormatTypes format_type;
        std::string err_msg;
    };

    std::vector<std::pair<VkFormat, FormatTypes>> formats_to_check({{VK_FORMAT_R8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R8G8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R8G8B8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R8G8B8A8_UNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8A8_SNORM, FLOAT},
                                                                    {VK_FORMAT_R8G8B8A8_SRGB, FLOAT},
                                                                    {VK_FORMAT_B8G8R8A8_UNORM, FLOAT},
                                                                    {VK_FORMAT_B8G8R8A8_SNORM, FLOAT},
                                                                    {VK_FORMAT_B8G8R8A8_SRGB, FLOAT},
                                                                    {VK_FORMAT_R16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R16G16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R16G16B16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R16G16B16A16_UNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16A16_SNORM, FLOAT},
                                                                    {VK_FORMAT_R16G16B16A16_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32G32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32G32B32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R32G32B32A32_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64G64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64G64B64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R64G64B64A64_SFLOAT, FLOAT},
                                                                    {VK_FORMAT_R8_SINT, SINT},
                                                                    {VK_FORMAT_R8G8_SINT, SINT},
                                                                    {VK_FORMAT_R8G8B8_SINT, SINT},
                                                                    {VK_FORMAT_R8G8B8A8_SINT, SINT},
                                                                    {VK_FORMAT_B8G8R8A8_SINT, SINT},
                                                                    {VK_FORMAT_R16_SINT, SINT},
                                                                    {VK_FORMAT_R16G16_SINT, SINT},
                                                                    {VK_FORMAT_R16G16B16_SINT, SINT},
                                                                    {VK_FORMAT_R16G16B16A16_SINT, SINT},
                                                                    {VK_FORMAT_R32_SINT, SINT},
                                                                    {VK_FORMAT_R32G32_SINT, SINT},
                                                                    {VK_FORMAT_R32G32B32_SINT, SINT},
                                                                    {VK_FORMAT_R32G32B32A32_SINT, SINT},
                                                                    {VK_FORMAT_R64_SINT, SINT},
                                                                    {VK_FORMAT_R64G64_SINT, SINT},
                                                                    {VK_FORMAT_R64G64B64_SINT, SINT},
                                                                    {VK_FORMAT_R64G64B64A64_SINT, SINT},
                                                                    {VK_FORMAT_R8_UINT, UINT},
                                                                    {VK_FORMAT_R8G8_UINT, UINT},
                                                                    {VK_FORMAT_R8G8B8_UINT, UINT},
                                                                    {VK_FORMAT_R8G8B8A8_UINT, UINT},
                                                                    {VK_FORMAT_B8G8R8A8_UINT, UINT},
                                                                    {VK_FORMAT_R16_UINT, UINT},
                                                                    {VK_FORMAT_R16G16_UINT, UINT},
                                                                    {VK_FORMAT_R16G16B16_UINT, UINT},
                                                                    {VK_FORMAT_R16G16B16A16_UINT, UINT},
                                                                    {VK_FORMAT_R32_UINT, UINT},
                                                                    {VK_FORMAT_R32G32_UINT, UINT},
                                                                    {VK_FORMAT_R32G32B32_UINT, UINT},
                                                                    {VK_FORMAT_R32G32B32A32_UINT, UINT},
                                                                    {VK_FORMAT_R64_UINT, UINT},
                                                                    {VK_FORMAT_R64G64_UINT, UINT},
                                                                    {VK_FORMAT_R64G64B64_UINT, UINT},
                                                                    {VK_FORMAT_R64G64B64A64_UINT, UINT}});

    std::vector<struct TestFilterType> tests(2);
    tests[0].err_msg = "VUID-vkCmdDraw-None-02690";

    tests[1].filter = VK_FILTER_CUBIC_IMG;
    tests[1].required_format_feature = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG;
    tests[1].err_msg = "VUID-vkCmdDraw-None-02692";

    for (auto &test_struct : tests) {
        for (std::pair<VkFormat, FormatTypes> cur_format_pair : formats_to_check) {
            VkFormatProperties props = {};
            vk::GetPhysicalDeviceFormatProperties(gpu(), cur_format_pair.first, &props);
            if (test_struct.format == VK_FORMAT_UNDEFINED && props.linearTilingFeatures != 0 &&
                (props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
                !(props.linearTilingFeatures & test_struct.required_format_feature)) {
                test_struct.format = cur_format_pair.first;
                test_struct.format_type = cur_format_pair.second;
            } else if (test_struct.format == VK_FORMAT_UNDEFINED && props.optimalTilingFeatures != 0 &&
                       (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
                       !(props.optimalTilingFeatures & test_struct.required_format_feature)) {
                test_struct.format = cur_format_pair.first;
                test_struct.format_type = cur_format_pair.second;
                test_struct.tiling = VK_IMAGE_TILING_OPTIMAL;
            }

            if (test_struct.format != VK_FORMAT_UNDEFINED) {
                break;
            }
        }
    }

    const char bindStateFragiSamplerShaderText[] =
        "#version 450\n"
        "layout(set=0, binding=0) uniform isampler2D s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "}\n";

    const char bindStateFraguSamplerShaderText[] =
        "#version 450\n"
        "layout(set=0, binding=0) uniform usampler2D s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "}\n";

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    for (auto test_struct : tests) {
        if (test_struct.format == VK_FORMAT_UNDEFINED) {
            printf("%s Could not find a testable format for filter %d.  Skipping test for said filter.\n", kSkipPrefix,
                   test_struct.filter);
            continue;
        }

        VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo();

        sci.magFilter = test_struct.filter;
        sci.minFilter = test_struct.filter;

        if (test_struct.filter == VK_FILTER_CUBIC_IMG) {
            if (cubic_support) {
                sci.anisotropyEnable = VK_FALSE;
            } else {
                printf("%s VK_FILTER_CUBIC_IMG not supported.  Skipping use of VK_FILTER_CUBIC_IMG this test.\n", kSkipPrefix);
                continue;
            }
        }

        VkSampler sampler;
        VkResult err = vk::CreateSampler(m_device->device(), &sci, nullptr, &sampler);
        ASSERT_VK_SUCCESS(err);

        VkImageObj mpimage(m_device);
        mpimage.Init(128, 128, 1, test_struct.format, VK_IMAGE_USAGE_SAMPLED_BIT, test_struct.tiling);
        ASSERT_TRUE(mpimage.initialized());

        VkImageView view = mpimage.targetView(test_struct.format);

        CreatePipelineHelper pipe(*this);
        VkShaderObj *fs = nullptr;

        pipe.InitInfo();

        if (test_struct.format_type == FLOAT) {
            fs = new VkShaderObj(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
        } else if (test_struct.format_type == SINT) {
            fs = new VkShaderObj(m_device, bindStateFragiSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
        } else if (test_struct.format_type == UINT) {
            fs = new VkShaderObj(m_device, bindStateFraguSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);
        }

        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs->GetStageCreateInfo()};
        pipe.dsl_bindings_ = {
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
        };
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        pipe.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        VkViewport viewport = {0, 0, 16, 16, 0, 1};
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        VkRect2D scissor = {{0, 0}, {16, 16}};
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_struct.err_msg.c_str());
        m_commandBuffer->Draw(1, 0, 0, 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        delete fs;
        vk::DestroySampler(m_device->device(), sampler, nullptr);
    }
}

TEST_F(VkLayerTest, IllegalAddressModeWithCornerSampledNV) {
    TEST_DESCRIPTION(
        "Create image with VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV flag and sample it with something other than "
        "VK_SAMPLER_ADDRESS_MODE_CLAMP_EDGE.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, "VK_NV_corner_sampled_image")) {
        m_device_extension_names.push_back("VK_NV_corner_sampled_image");
    } else {
        printf("%s VK_NV_corner_sampled_image not supported.  Skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, 0));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj test_image(m_device);
    VkImageCreateInfo image_info = VkImageObj::create_info();
    image_info.flags = VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    // If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV,
    // imageType must be VK_IMAGE_TYPE_2D or VK_IMAGE_TYPE_3D
    image_info.imageType = VK_IMAGE_TYPE_2D;
    // If flags contains VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and imageType is VK_IMAGE_TYPE_2D,
    // extent.width and extent.height must be greater than 1.
    image_info.extent = {2, 2, 1};
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    test_image.init(&image_info);
    ASSERT_TRUE(test_image.initialized());

    VkSamplerCreateInfo sci = SafeSaneSamplerCreateInfo();
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSampler sampler;
    VkResult err = vk::CreateSampler(m_device->device(), &sci, nullptr, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkImageView view = test_image.targetView(image_info.format);

    CreatePipelineHelper pipe(*this);
    VkShaderObj fs(m_device, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    pipe.InitInfo();

    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-flags-02696");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroySampler(m_device->device(), sampler, nullptr);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

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
        vkCreateSamplerYcbcrConversionFunction = vk::CreateSamplerYcbcrConversion;
        vkDestroySamplerYcbcrConversionFunction = vk::DestroySamplerYcbcrConversion;
    } else {
        vkCreateSamplerYcbcrConversionFunction =
            (PFN_vkCreateSamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
        vkDestroySamplerYcbcrConversionFunction =
            (PFN_vkDestroySamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkDestroySamplerYcbcrConversionKHR");
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
                                  VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,  // need for multi-planar
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
    ycbcr_create_info.components.a = VK_COMPONENT_SWIZZLE_ZERO;  // Just anything different than above
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
    VkResult err = vk::CreateSampler(m_device->device(), &sci, NULL, &samplers[0]);
    ASSERT_VK_SUCCESS(err);
    ycbcr_info.conversion = conversions[1];  // Need two samplers with different conversions
    err = vk::CreateSampler(m_device->device(), &sci, NULL, &samplers[1]);
    ASSERT_VK_SUCCESS(err);

    VkSampler BadSampler;
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01646");
    err = vk::CreateSampler(m_device->device(), &sci, NULL, &BadSampler);
    m_errorMonitor->VerifyFound();

    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.unnormalizedCoordinates = VK_TRUE;
    sci.minLod = 0.0;
    sci.maxLod = 0.0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01646");
    err = vk::CreateSampler(m_device->device(), &sci, NULL, &BadSampler);
    m_errorMonitor->VerifyFound();

    if (device_features.samplerAnisotropy == VK_TRUE) {
        sci.unnormalizedCoordinates = VK_FALSE;
        sci.anisotropyEnable = VK_TRUE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01646");
        err = vk::CreateSampler(m_device->device(), &sci, NULL, &BadSampler);
        m_errorMonitor->VerifyFound();
    }

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
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-01948");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02738");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    vkDestroySamplerYcbcrConversionFunction(m_device->device(), conversions[0], nullptr);
    vkDestroySamplerYcbcrConversionFunction(m_device->device(), conversions[1], nullptr);
    vk::DestroyImageView(m_device->device(), view, NULL);
    vk::DestroySampler(m_device->device(), samplers[0], nullptr);
    vk::DestroySampler(m_device->device(), samplers[1], nullptr);
}

TEST_F(VkLayerTest, DepthStencilImageViewWithColorAspectBitError) {
    // Create a single Image descriptor and cause it to first hit an error due
    //  to using a DS format, then cause it to hit error due to COLOR_BIT not
    //  set in aspect
    // The image format check comes 2nd in validation so we trigger it first,
    //  then when we cause aspect fail next, bad format check will be preempted

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "Combination depth/stencil image formats can have only the ");

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
    vk::CreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

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
        (PFN_vkCreateSamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
    if (vkCreateSamplerYcbcrConversionKHR == nullptr) {
        printf("%s VK_KHR_sampler_ycbcr_conversion not supported by device; skipped.\n", kSkipPrefix);
        return;
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GeneralParameterError-ExtensionNotEnabled");
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

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // VK_KHR_get_physical_device_properties2 promoted to 1.1
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s VK_KHR_get_physical_device_properties2 requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceProperties2-sType-unique");
    // in VkPhysicalDeviceProperties2 create a chain of pNext of type A -> B -> A
    // Also using different instance of struct to not trip the cycle checkings
    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_0 = {};
    protected_memory_properties_0.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES;
    protected_memory_properties_0.pNext = nullptr;

    VkPhysicalDeviceIDProperties id_properties = {};
    id_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
    id_properties.pNext = &protected_memory_properties_0;

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_1 = {};
    protected_memory_properties_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_PROPERTIES;
    protected_memory_properties_1.pNext = &id_properties;

    VkPhysicalDeviceProperties2 physical_device_properties2 = {};
    physical_device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physical_device_properties2.pNext = &protected_memory_properties_1;

    vk::GetPhysicalDeviceProperties2(gpu(), &physical_device_properties2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DedicatedAllocationBinding) {
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
    m_errorMonitor->ExpectSuccess();
    vk::BindBufferMemory(m_device->handle(), buffer.handle(), dedicated_buffer_memory.handle(), 0);
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
    m_errorMonitor->ExpectSuccess();
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, DedicatedAllocationImageAliasing) {
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) &&
        DeviceExtensionSupported(gpu(), nullptr, VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Dedicated allocation extension not supported, skipping test\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto aliasing_features = lvl_init_struct<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&aliasing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    aliasing_features.dedicatedAllocationImageAliasing = VK_TRUE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkMemoryPropertyFlags mem_flags = 0;
    const VkDeviceSize resource_size = 1024;

    VkImageObj image(m_device);
    VkImageObj identical_image(m_device);
    auto image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image.init_no_mem(*m_device, image_info);
    identical_image.init_no_mem(*m_device, image_info);

    auto image_dedicated_info = lvl_init_struct<VkMemoryDedicatedAllocateInfoKHR>();
    image_dedicated_info.image = image.handle();
    auto image_alloc_info = vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), mem_flags);
    image_alloc_info.pNext = &image_dedicated_info;
    vk_testing::DeviceMemory dedicated_image_memory;
    dedicated_image_memory.init(*m_device, image_alloc_info);

    // Bind with different but identical image
    m_errorMonitor->ExpectSuccess();
    vk::BindImageMemory(m_device->handle(), identical_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();

    VkImageObj smaller_image(m_device);
    image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size - 1;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    smaller_image.init_no_mem(*m_device, image_info);

    // Bind with a smaller image
    m_errorMonitor->ExpectSuccess();
    vk::BindImageMemory(m_device->handle(), smaller_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();

    VkImageObj larger_image(m_device);
    image_info = VkImageObj::create_info();
    image_info.extent.width = resource_size + 1;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    larger_image.init_no_mem(*m_device, image_info);

    // Bind with a larger image (not supported, and not enough memory)
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02629");
    if (larger_image.memory_requirements().size > image.memory_requirements().size) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-size-01049");
    }
    vk::BindImageMemory(m_device->handle(), larger_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyFound();

    // Bind with non-zero offset
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-memory-02629");  // offset must be zero
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-vkBindImageMemory-size-01049");  // offset pushes us past size
    auto image_offset = image.memory_requirements().alignment;
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), image_offset);
    m_errorMonitor->VerifyFound();

    // Bind correctly (depends on the "skip" above)
    m_errorMonitor->ExpectSuccess();
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
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

TEST_F(VkLayerTest, ImageStencilCreate) {
    TEST_DESCRIPTION("Verify ImageStencil create info.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    device_features.shaderStorageImageMultisample = VK_FALSE;  // Force multisampled storage images off

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME)) {
        printf("%s VK_EXT_separate_stencil_usage Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageStencilUsageCreateInfoEXT image_stencil_create_info = {};
    image_stencil_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO_EXT;
    image_stencil_create_info.pNext = nullptr;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT;

    image_create_info.pNext = &image_stencil_create_info;

    VkPhysicalDeviceImageFormatInfo2 image_format_info2 = {};
    image_format_info2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    image_format_info2.pNext = &image_stencil_create_info;
    image_format_info2.format = image_create_info.format;
    image_format_info2.type = image_create_info.imageType;
    image_format_info2.tiling = image_create_info.tiling;
    image_format_info2.usage = image_create_info.usage;
    image_format_info2.flags = image_create_info.flags;

    VkImageFormatProperties2 image_format_properties2 = {};
    image_format_properties2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    image_format_properties2.pNext = nullptr;
    image_format_properties2.imageFormatProperties = {};

    // when including VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, must not include bits other than
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539");
    vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info2, &image_format_properties2);
    m_errorMonitor->VerifyFound();
    // test vkCreateImage as well for this case
    CreateImageTest(*this, &image_create_info, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539");

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;

    if (dev_limits.maxFramebufferWidth != UINT32_MAX) {
        // depth-stencil format image with VkImageStencilUsageCreateInfo with
        // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT set cannot have image width exceeding device maximum
        image_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        image_create_info.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
        image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-Format-02536");
    } else {
        printf("%s VkPhysicalDeviceLimits::maxFramebufferWidth is already UINT32_MAX; skipping part of test.\n", kSkipPrefix);
    }

    if (dev_limits.maxFramebufferHeight != UINT32_MAX) {
        // depth-stencil format image with VkImageStencilUsageCreateInfo with
        // VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT set cannot have image height exceeding device maximum
        image_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        image_create_info.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
        image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02537");
    } else {
        printf("%s VkPhysicalDeviceLimits::maxFramebufferHeight is already UINT32_MAX; skipping part of test.\n", kSkipPrefix);
    }

    // depth-stencil format image with VkImageStencilUsageCreateInfo with
    // VK_IMAGE_USAGE_STORAGE_BIT and the multisampled storage images feature
    // is not enabled, image samples must be VK_SAMPLE_COUNT_1_BIT
    image_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    image_create_info.extent = {64, 64, 1};
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02538");

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage includes
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02795");

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage does not include
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also not include VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02796");

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage includes
    // VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02797");

    // depth-stencil format image with VkImageStencilUsageCreateInfo, usage does not include
    // VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, so VkImageStencilUsageCreateInfo::stencilUsage
    // must also not include VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02798");
}

TEST_F(VkLayerTest, CreateYCbCrSampler) {
    TEST_DESCRIPTION("Verify YCbCr sampler creation.");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Test requires DeviceProfileLayer, unavailable - skipped.\n", kSkipPrefix);
        return;
    }

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        return;
    }

    // Need to enable YCbCr feature
    auto ycbcr_features = lvl_init_struct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ycbcr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    ycbcr_features.samplerYcbcrConversion = VK_TRUE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Required extensions are not avaiable.\n", kSkipPrefix);
        return;
    }

    PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionFunction = nullptr;
    PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionFunction = nullptr;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkCreateSamplerYcbcrConversionFunction = vk::CreateSamplerYcbcrConversion;
        vkDestroySamplerYcbcrConversionFunction = vk::DestroySamplerYcbcrConversion;
    } else {
        vkCreateSamplerYcbcrConversionFunction =
            (PFN_vkCreateSamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
        vkDestroySamplerYcbcrConversionFunction =
            (PFN_vkDestroySamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkDestroySamplerYcbcrConversionKHR");
    }

    if (!vkCreateSamplerYcbcrConversionFunction || !vkDestroySamplerYcbcrConversionFunction) {
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
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_NEAREST;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;

    // test non external conversion with a VK_FORMAT_UNDEFINED
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-04060");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test for non unorm
    sycci.format = VK_FORMAT_R8G8B8A8_SNORM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-04060");
    m_errorMonitor->SetUnexpectedError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Force the multi-planar format support desired format features
    VkFormat mp_format = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, &formatProps);
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);

    // Check that errors are caught when format feature don't exist
    sycci.format = mp_format;

    // No Chroma Sampler Bit set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    // 01651 set off twice for both xChromaOffset and yChromaOffset
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Cosited feature supported, but midpoint samples set
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);
    sycci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Moving support to Linear to test that it checks either linear or optimal
    formatProps.linearTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);
    sycci.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Using forceExplicitReconstruction without feature bit
    sycci.forceExplicitReconstruction = VK_TRUE;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-forceExplicitReconstruction-01656");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Linear chroma filtering without feature bit
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-chromaFilter-01657");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // Add linear feature bit so can create valid SamplerYcbcrConversion
    formatProps.linearTilingFeatures = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;
    formatProps.optimalTilingFeatures = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);
    m_errorMonitor->ExpectSuccess();
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyNotFound();

    // Try to create a Sampler with non-matching filters without feature bit set
    VkSamplerYcbcrConversionInfo sampler_ycbcr_info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO, nullptr, ycbcr_conv};
    VkSampler sampler;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.minFilter = VK_FILTER_NEAREST;  // Different than chromaFilter
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.pNext = (void *)&sampler_ycbcr_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-minFilter-01645");
    vk::CreateSampler(device(), &sampler_info, nullptr, &sampler);
    m_errorMonitor->VerifyFound();

    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-minFilter-01645");
    vk::CreateSampler(device(), &sampler_info, nullptr, &sampler);
    m_errorMonitor->VerifyFound();

    vkDestroySamplerYcbcrConversionFunction(device(), ycbcr_conv, nullptr);
}

TEST_F(VkLayerTest, InvalidSwizzleYCbCr) {
    TEST_DESCRIPTION("Verify Invalid use of siwizzle components when dealing with YCbCr.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        return;
    }

    // Need to enable YCbCr feature
    auto ycbcr_features = lvl_init_struct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&ycbcr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    ycbcr_features.samplerYcbcrConversion = VK_TRUE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionFunction = nullptr;
    PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionFunction = nullptr;
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkCreateSamplerYcbcrConversionFunction = vk::CreateSamplerYcbcrConversion;
        vkDestroySamplerYcbcrConversionFunction = vk::DestroySamplerYcbcrConversion;
    } else {
        vkCreateSamplerYcbcrConversionFunction =
            (PFN_vkCreateSamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
        vkDestroySamplerYcbcrConversionFunction =
            (PFN_vkDestroySamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkDestroySamplerYcbcrConversionKHR");
    }
    if (!vkCreateSamplerYcbcrConversionFunction || !vkDestroySamplerYcbcrConversionFunction) {
        printf("%s Did not find required device support for YcbcrSamplerConversion; test skipped.\n", kSkipPrefix);
        return;
    }

    const VkFormat mp_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;

    // Make sure components doesn't affect _444 formats
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), mp_format, &format_props);
    if ((format_props.optimalTilingFeatures &
         (VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT | VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT)) == 0) {
        printf("%s Device does not support chroma sampling of 3plane 420 format; test skipped.\n", kSkipPrefix);
        return;
    }

    const VkComponentMapping identity = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

    VkSamplerYcbcrConversion ycbcr_conv = VK_NULL_HANDLE;
    VkSamplerYcbcrConversionCreateInfo sycci = {};
    sycci.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO;
    sycci.format = mp_format;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    sycci.forceExplicitReconstruction = VK_FALSE;
    sycci.chromaFilter = VK_FILTER_NEAREST;
    sycci.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    sycci.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;

    // test components.g
    // This test is also serves as positive form of VU 01655 since SWIZZLE_A is considered only valid with this format because
    // ycbcrModel RGB_IDENTITY
    sycci.components = identity;
    sycci.components.g = VK_COMPONENT_SWIZZLE_A;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.a
    sycci.components = identity;
    sycci.components.a = VK_COMPONENT_SWIZZLE_R;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02582");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // make sure zero and one are allowed for components.a
    m_errorMonitor->ExpectSuccess();
    sycci.components.a = VK_COMPONENT_SWIZZLE_ONE;
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    vkDestroySamplerYcbcrConversionFunction(device(), ycbcr_conv, nullptr);
    sycci.components.a = VK_COMPONENT_SWIZZLE_ZERO;
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    vkDestroySamplerYcbcrConversionFunction(device(), ycbcr_conv, nullptr);
    m_errorMonitor->VerifyNotFound();

    // test components.r
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_G;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02583");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.b
    sycci.components = identity;
    sycci.components.b = VK_COMPONENT_SWIZZLE_G;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02584");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.r and components.b together
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_B;
    sycci.components.b = VK_COMPONENT_SWIZZLE_B;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // make sure components.r and components.b can be swapped
    m_errorMonitor->ExpectSuccess();
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_B;
    sycci.components.b = VK_COMPONENT_SWIZZLE_R;
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
    vkDestroySamplerYcbcrConversionFunction(device(), ycbcr_conv, nullptr);
    m_errorMonitor->VerifyNotFound();

    // Non RGB Identity model
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY;
    {
        // Non RGB Identity can't have a explicit zero swizzle
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_ZERO;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
        m_errorMonitor->VerifyFound();

        // Non RGB Identity can't have a explicit one swizzle
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_ONE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
        m_errorMonitor->VerifyFound();

        // VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM has 3 component so RGBA conversion has implicit A as one
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_A;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
        m_errorMonitor->VerifyFound();
    }
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;  // reset

    // Make sure components doesn't affect _444 formats
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, &format_props);
    if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT) != 0) {
        sycci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_R;
        m_errorMonitor->ExpectSuccess();
        vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);
        vkDestroySamplerYcbcrConversionFunction(device(), ycbcr_conv, nullptr);
        m_errorMonitor->VerifyNotFound();
    }

    // Create a valid conversion with guaranteed support
    sycci.format = mp_format;
    sycci.components = identity;
    vkCreateSamplerYcbcrConversionFunction(device(), &sycci, NULL, &ycbcr_conv);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkSamplerYcbcrConversionInfo conversion_info = {};
    conversion_info.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
    conversion_info.pNext = nullptr;
    conversion_info.conversion = ycbcr_conv;

    VkImageView image_view;
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = &conversion_info;
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.components = identity;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_B;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-pNext-01970");
    vk::CreateImageView(m_device->device(), &image_view_create_info, nullptr, &image_view);
    m_errorMonitor->VerifyFound();

    vkDestroySamplerYcbcrConversionFunction(device(), ycbcr_conv, nullptr);
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables buffer_device_address
    auto buffer_device_address_features = lvl_init_struct<VkPhysicalDeviceBufferAddressFeaturesEXT>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressEXT vkGetBufferDeviceAddressEXT =
        (PFN_vkGetBufferDeviceAddressEXT)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressEXT");

    VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_EXT;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferDeviceAddressCreateInfoEXT addr_ci = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT};
    addr_ci.deviceAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-deviceAddress-02604");

    buffer_create_info.pNext = nullptr;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoEXT info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT};
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vkGetBufferDeviceAddressEXT(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = {};
    buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(m_device->device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->ExpectSuccess();
    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    m_errorMonitor->VerifyNotFound();
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
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
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
        (PFN_vkGetBufferDeviceAddressEXT)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressEXT");

    VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoEXT info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_EXT};
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

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables buffer_device_address
    auto buffer_device_address_features = lvl_init_struct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");
    PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR vkGetDeviceMemoryOpaqueCaptureAddressKHR =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetDeviceMemoryOpaqueCaptureAddressKHR");

    VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    buffer_create_info.flags = VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-flags-03338");

    buffer_create_info.flags = 0;
    VkBufferOpaqueCaptureAddressCreateInfoKHR addr_ci = {VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO_KHR};
    addr_ci.opaqueCaptureAddress = 1;
    buffer_create_info.pNext = &addr_ci;
    CreateBufferTest(*this, &buffer_create_info, "VUID-VkBufferCreateInfo-opaqueCaptureAddress-03337");

    buffer_create_info.pNext = nullptr;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoKHR info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
    info.buffer = buffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
    vkGetBufferDeviceAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
        auto fpGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddress");
        if (nullptr == fpGetBufferDeviceAddress) {
            m_errorMonitor->ExpectSuccess();
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkGetBufferDeviceAddress");
            m_errorMonitor->VerifyNotFound();
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferDeviceAddressInfo-buffer-02600");
            fpGetBufferDeviceAddress(m_device->device(), &info);
            m_errorMonitor->VerifyFound();
        }
    }

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = {};
    buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-bufferDeviceAddress-03339");
    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);
    m_errorMonitor->VerifyFound();

    VkDeviceMemoryOpaqueCaptureAddressInfoKHR mem_opaque_addr_info = {
        VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO_KHR};
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vkGetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
        auto fpGetDeviceMemoryOpaqueCaptureAddress =
            (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)vk::GetDeviceProcAddr(device(), "vkGetDeviceMemoryOpaqueCaptureAddress");
        if (nullptr == fpGetDeviceMemoryOpaqueCaptureAddress) {
            m_errorMonitor->ExpectSuccess();
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkGetDeviceMemoryOpaqueCaptureAddress");
            m_errorMonitor->VerifyNotFound();
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
            fpGetDeviceMemoryOpaqueCaptureAddress(m_device->device(), &mem_opaque_addr_info);
            m_errorMonitor->VerifyFound();
        }
    }

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);

    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->ExpectSuccess();
    vkGetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->ExpectSuccess();
    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->ExpectSuccess();
    vkGetBufferDeviceAddressKHR(m_device->device(), &info);
    m_errorMonitor->VerifyNotFound();

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, BufferDeviceAddressKHRDisabled) {
    TEST_DESCRIPTION("Test VK_KHR_buffer_device_address.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 1> required_device_extensions = {{VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that disables buffer_device_address
    auto buffer_device_address_features = lvl_init_struct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR =
        (PFN_vkGetBufferDeviceAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferDeviceAddressKHR");
    PFN_vkGetBufferOpaqueCaptureAddressKHR vkGetBufferOpaqueCaptureAddressKHR =
        (PFN_vkGetBufferOpaqueCaptureAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetBufferOpaqueCaptureAddressKHR");
    PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR vkGetDeviceMemoryOpaqueCaptureAddressKHR =
        (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)vk::GetDeviceProcAddr(device(), "vkGetDeviceMemoryOpaqueCaptureAddressKHR");

    VkBufferCreateInfo buffer_create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = sizeof(uint32_t);
    buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBuffer buffer;
    VkResult err = vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferDeviceAddressInfoKHR info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
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
            m_errorMonitor->ExpectSuccess();
            m_errorMonitor->SetError("No ProcAddr for 1.2 core vkGetBufferOpaqueCaptureAddress");
            m_errorMonitor->VerifyNotFound();
        } else {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferOpaqueCaptureAddress-None-03326");
            fpGetBufferOpaqueCaptureAddress(m_device->device(), &info);
            m_errorMonitor->VerifyFound();
        }
    }

    VkMemoryRequirements buffer_mem_reqs = {};
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_alloc_info = {};
    buffer_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    buffer_alloc_info.allocationSize = buffer_mem_reqs.size;
    m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &buffer_alloc_info, 0);
    VkDeviceMemory buffer_mem;
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);

    VkDeviceMemoryOpaqueCaptureAddressInfoKHR mem_opaque_addr_info = {
        VK_STRUCTURE_TYPE_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS_INFO_KHR};
    mem_opaque_addr_info.memory = buffer_mem;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDeviceMemoryOpaqueCaptureAddress-None-03334");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceMemoryOpaqueCaptureAddressInfo-memory-03336");
    vkGetDeviceMemoryOpaqueCaptureAddressKHR(m_device->device(), &mem_opaque_addr_info);
    m_errorMonitor->VerifyFound();

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);

    VkMemoryAllocateFlagsInfo alloc_flags = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
    alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT_KHR;
    buffer_alloc_info.pNext = &alloc_flags;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-flags-03330");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryAllocateInfo-flags-03331");
    err = vk::AllocateMemory(device(), &buffer_alloc_info, NULL, &buffer_mem);
    m_errorMonitor->VerifyFound();

    vk::DestroyBuffer(m_device->device(), buffer, NULL);
}

TEST_F(VkLayerTest, CreateImageYcbcrFormats) {
    TEST_DESCRIPTION("Creating images with Ycbcr Formats.");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
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

    bool ycbcr_array_extension = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME);
    if (ycbcr_array_extension) {
        m_device_extension_names.push_back(VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
    }

    // Set format features as needed for tests
    VkFormatProperties formatProps;
    const VkFormat mp_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, &formatProps);
    formatProps.optimalTilingFeatures |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
    formatProps.optimalTilingFeatures = formatProps.optimalTilingFeatures & ~VK_FORMAT_FEATURE_DISJOINT_BIT;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), mp_format, formatProps);

    // Create ycbcr image with all valid values
    // Each test changes needed values and returns them back after
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = mp_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.arrayLayers = 1;
    VkImageCreateInfo reset_create_info = image_create_info;

    VkImageFormatProperties img_limits;
    ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_create_info, &img_limits));

    // invalid arrayLayers
    if (img_limits.maxArrayLayers == 1) {
        printf("%s Multiplane image maxArrayLayers is already 1.  Skipping test.\n", kSkipPrefix);
    } else {
        image_create_info.arrayLayers = img_limits.maxArrayLayers;
        const char *error_vuid =
            (ycbcr_array_extension) ? "VUID-VkImageCreateInfo-format-02653" : "VUID-VkImageCreateInfo-format-02564";
        CreateImageTest(*this, &image_create_info, error_vuid);
        image_create_info = reset_create_info;
    }

    // invalid mipLevels
    if (img_limits.maxMipLevels == 1) {
        printf("%s Multiplane image maxMipLevels is already 1.  Skipping test.\n", kSkipPrefix);
    } else {
        // needs to be 2
        // if more then 2 the VU since its larger the (depth^2 + 1)
        // if up the depth the VU for IMAGE_TYPE_2D and depth != 1 hits
        image_create_info.mipLevels = 2;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02561");
        image_create_info = reset_create_info;
    }

    // invalid samples count
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    // Might need to add extra validation because implementation probably doesn't support YUV
    VkImageFormatProperties image_format_props;
    vk::GetPhysicalDeviceImageFormatProperties(gpu(), mp_format, image_create_info.imageType, image_create_info.tiling,
                                               image_create_info.usage, image_create_info.flags, &image_format_props);
    if ((image_format_props.sampleCounts & VK_SAMPLE_COUNT_4_BIT) == 0) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-samples-02258");
    }
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02562");
    image_create_info = reset_create_info;

    // invalid imageType
    image_create_info.extent.height = 1;  // to satisfy 1D requriments
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-02563");
    image_create_info = reset_create_info;

    // Test using a format that doesn't support disjoint
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageCreateFormatFeatures-02260");
    image_create_info = reset_create_info;
}

TEST_F(VkLayerTest, InvalidSamplerFilterMinmax) {
    TEST_DESCRIPTION("Invalid uses of VK_EXT_sampler_filter_minmax.");

    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
    } else {
        printf("%s test requires Sampler Filter MinMax extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

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
        vkCreateSamplerYcbcrConversionFunction = vk::CreateSamplerYcbcrConversion;
        vkDestroySamplerYcbcrConversionFunction = vk::DestroySamplerYcbcrConversion;
    } else {
        vkCreateSamplerYcbcrConversionFunction =
            (PFN_vkCreateSamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
        vkDestroySamplerYcbcrConversionFunction =
            (PFN_vkDestroySamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkDestroySamplerYcbcrConversionKHR");
    }

    if (!vkCreateSamplerYcbcrConversionFunction || !vkDestroySamplerYcbcrConversionFunction) {
        printf("%s Did not find required device extension %s; test skipped.\n", kSkipPrefix,
               VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        return;
    }

    VkSampler sampler;

    // Create Ycbcr conversion
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
                                                            NULL,
                                                            VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
                                                            VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY,
                                                            VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                            {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                             VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_CHROMA_LOCATION_COSITED_EVEN,
                                                            VK_FILTER_NEAREST,
                                                            false};
    VkSamplerYcbcrConversion conversion;
    vkCreateSamplerYcbcrConversionFunction(m_device->handle(), &ycbcr_create_info, nullptr, &conversion);

    VkSamplerYcbcrConversionInfo ycbcr_info = {};
    ycbcr_info.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
    ycbcr_info.conversion = conversion;

    VkSamplerReductionModeCreateInfo reduction_info = {};
    reduction_info.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
    reduction_info.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.pNext = &reduction_info;

    // Wrong mode with a YCbCr Conversion used
    reduction_info.pNext = &ycbcr_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-None-01647");
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();

    // Wrong mode with compareEnable
    reduction_info.pNext = nullptr;
    sampler_info.compareEnable = VK_TRUE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-compareEnable-01423");
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();

    vkDestroySamplerYcbcrConversionFunction(m_device->handle(), conversion, nullptr);
}

TEST_F(VkLayerTest, BindImageMemorySwapchain) {
    TEST_DESCRIPTION("Invalid bind image with a swapchain");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping BindSwapchainImageMemory test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

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
    vk::CreateImage(device(), &image_create_info, NULL, &image_from_swapchain);

    VkMemoryRequirements mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image_from_swapchain, &mem_reqs);

    auto alloc_info = lvl_init_struct<VkMemoryAllocateInfo>();
    alloc_info.memoryTypeIndex = 0;
    alloc_info.allocationSize = mem_reqs.size;

    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    VkDeviceMemory mem;
    VkResult err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    auto bind_info = lvl_init_struct<VkBindImageMemoryInfo>();
    bind_info.image = image_from_swapchain;
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-image-01630");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01632");
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    auto bind_swapchain_info = lvl_init_struct<VkBindImageMemorySwapchainInfoKHR>();
    bind_swapchain_info.swapchain = VK_NULL_HANDLE;
    bind_swapchain_info.imageIndex = 0;
    bind_info.pNext = &bind_swapchain_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-GeneralParameterError-RequiredParameter");
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    bind_info.memory = mem;
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = UINT32_MAX;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01631");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemorySwapchainInfoKHR-imageIndex-01644");
    vk::BindImageMemory2(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), image_from_swapchain, NULL);
    vk::FreeMemory(m_device->device(), mem, NULL);
    DestroySwapchain();
}

TEST_F(VkLayerTest, ValidSwapchainImage) {
    TEST_DESCRIPTION("Swapchain images with invalid parameters");
    const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (IsPlatform(kGalaxyS10)) {
        printf("%s This test should not run on Galaxy S10\n", kSkipPrefix);
        return;
    }

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSwapchain()) {
        printf("%s Cannot create surface or swapchain, skipping test\n", kSkipPrefix);
        return;
    }

    VkImage image;
    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = {};
    image_swapchain_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR;
    image_swapchain_create_info.pNext = nullptr;
    image_swapchain_create_info.swapchain = m_swapchain;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = &image_swapchain_create_info;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImageCreateInfo good_create_info = image_create_info;

    // imageType
    image_create_info = good_create_info;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // mipLevels
    image_create_info = good_create_info;
    image_create_info.mipLevels = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // samples
    image_create_info = good_create_info;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // tiling
    image_create_info = good_create_info;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // initialLayout
    image_create_info = good_create_info;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    vk::CreateImage(device(), &image_create_info, NULL, &image);
    m_errorMonitor->VerifyFound();

    // flags
    if (m_device->phy().features().sparseBinding) {
        image_create_info = good_create_info;
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        vk::CreateImage(device(), &image_create_info, NULL, &image);
        m_errorMonitor->VerifyFound();
    }

    DestroySwapchain();
}

TEST_F(VkLayerTest, TransferImageToSwapchainWithInvalidLayoutDeviceGroup) {
    TEST_DESCRIPTION("Transfer an image to a swapchain's image with a invalid layout between device group");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    printf(
        "%s According to valid usage, VkBindImageMemoryInfo-memory should be NULL. But Android will crash if memory is NULL, "
        "skipping test\n",
        kSkipPrefix);
    return;
#endif

    SetTargetApiVersion(VK_API_VERSION_1_1);

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s VkBindImageMemoryInfo requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        printf("%s physical_device_group_count is 0, skipping test\n", kSkipPrefix);
        return;
    }

    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
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
    vk::CreateImage(device(), &image_create_info, NULL, &peer_image);

    auto bind_devicegroup_info = lvl_init_struct<VkBindImageMemoryDeviceGroupInfo>();
    bind_devicegroup_info.deviceIndexCount = 2;
    std::array<uint32_t, 2> deviceIndices = {{0, 0}};
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

    vk::BindImageMemory2(m_device->device(), 1, &bind_info);

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());

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
    vk::CmdCopyImage(m_commandBuffer->handle(), src_Image.handle(), VK_IMAGE_LAYOUT_GENERAL, peer_image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    m_commandBuffer->end();

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    // Both images have incorrect layouts
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    vk::DestroyImage(m_device->device(), peer_image, NULL);
    DestroySwapchain();
}

TEST_F(VkLayerTest, InvalidMemoryType) {
    // Attempts to allocate from a memory type that doesn't exist

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-pAllocateInfo-01714");

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
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

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.memoryTypeIndex = 0;
    mem_alloc.allocationSize = memory_info.memoryHeaps[memory_info.memoryTypes[0].heapIndex].size + 1;

    VkDeviceMemory mem;
    vk::AllocateMemory(m_device->device(), &mem_alloc, NULL, &mem);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DeviceCoherentMemoryDisabledAMD) {
    // Attempts to allocate device coherent memory without enabling the extension/feature

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support the necessary memory type, skipping test\n", kSkipPrefix);
        return;
    }

    // Check extension support but do not enable it
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME)) {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
        return;
    }

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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAllocateMemory-deviceCoherentMemory-02790");

    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
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
        printf("%s test requires Vulkan 1.1 extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    const VkFormat disjoint_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    const VkFormat normal_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), disjoint_format, &format_properties);

    bool sparse_support = (m_device->phy().features().sparseBinding == VK_TRUE);
    bool disjoint_support = ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) != 0);

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = NULL;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.size = 2048;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = NULL;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
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
    VkMemoryDedicatedAllocateInfo dedicated_allocate_info = {};
    dedicated_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicated_allocate_info.pNext = nullptr;

    VkMemoryAllocateInfo memory_allocate_info = {};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.pNext = (void *)&dedicated_allocate_info;
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
        VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
        VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
        mem_req_info2.pNext = &image_plane_req;
        mem_req_info2.image = disjoint_image;
        VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};

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
    const char *image_vuid = DeviceExtensionEnabled(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)
                                 ? "VUID-VkMemoryDedicatedAllocateInfo-image-02964"
                                 : "VUID-VkMemoryDedicatedAllocateInfo-image-01433";
    const char *buffer_vuid = DeviceExtensionEnabled(VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME)
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
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    mp_extensions = mp_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (mp_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (!mp_extensions) {
        printf("%s test requires KHR YCbCr extensions, not available.  Skipping.\n", kSkipPrefix);
    } else {
        // Need to make sure disjoint is supported for format
        // Also need to support an arbitrary image usage feature
        VkFormatProperties format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &format_properties);
        if (0 ==
            (format_properties.optimalTilingFeatures & (VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            printf("%s test requires disjoint/sampled feature bit on format.  Skipping.\n", kSkipPrefix);
        } else {
            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.pNext = NULL;
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

            VkImageMemoryRequirementsInfo2 mem_req_info2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2};
            mem_req_info2.pNext = nullptr;
            mem_req_info2.image = image;
            VkMemoryRequirements2 mem_req2 = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-01589");
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
            m_errorMonitor->VerifyFound();

            // Point to a 3rd plane for a 2-plane format
            VkImagePlaneMemoryRequirementsInfo image_plane_req = {VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO};
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
            m_errorMonitor->SetUnexpectedError("VUID-VkImageMemoryRequirementsInfo2-image-01590");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryRequirementsInfo2-image-02280");
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
            m_errorMonitor->VerifyFound();

            vk::DestroyImage(m_device->device(), image, nullptr);
        }
    }
}

TEST_F(VkLayerTest, FragmentDensityMapEnabled) {
    TEST_DESCRIPTION("Validation must check several conditions that apply only when Fragment Density Maps are used.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool fdmSupported = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    bool fdm2Supported = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);

    // Check extension support
    if (!(fdmSupported || fdm2Supported)) {
        printf("%s test requires %s or %s extensions. Skipping.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        return;
    }

    if (fdmSupported) {
        m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    }

    if (fdm2Supported) {
        m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT density_map2_features =
        lvl_init_struct<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT>();
    VkPhysicalDeviceFeatures2KHR features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&density_map2_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT density_map2_properties =
        lvl_init_struct<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>();
    VkPhysicalDeviceProperties2KHR properties2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&density_map2_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Test sampler parameters

    VkSamplerCreateInfo sampler_info_ref = SafeSaneSamplerCreateInfo();
    sampler_info_ref.maxLod = 0.0;
    sampler_info_ref.flags |= VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT;
    VkSamplerCreateInfo sampler_info = sampler_info_ref;

    // min max filters must match
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02574");
    sampler_info.minFilter = sampler_info_ref.minFilter;
    sampler_info.magFilter = sampler_info_ref.magFilter;

    // mipmapMode must be SAMPLER_MIPMAP_MODE_NEAREST
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02575");
    sampler_info.mipmapMode = sampler_info_ref.mipmapMode;

    // minLod and maxLod must be 0.0
    sampler_info.minLod = 1.0;
    sampler_info.maxLod = 1.0;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02576");
    sampler_info.minLod = sampler_info_ref.minLod;
    sampler_info.maxLod = sampler_info_ref.maxLod;

    // addressMode must be CLAMP_TO_EDGE or CLAMP_TO_BORDER
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02577");
    sampler_info.addressModeU = sampler_info_ref.addressModeU;

    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02577");
    sampler_info.addressModeV = sampler_info_ref.addressModeV;

    // some features cannot be enabled for subsampled samplers
    if (features2.features.samplerAnisotropy == VK_TRUE) {
        sampler_info.anisotropyEnable = VK_TRUE;
        CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02578");
        sampler_info.anisotropyEnable = sampler_info_ref.anisotropyEnable;
        sampler_info.anisotropyEnable = VK_FALSE;
    }

    sampler_info.compareEnable = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02579");
    sampler_info.compareEnable = sampler_info_ref.compareEnable;

    sampler_info.unnormalizedCoordinates = VK_TRUE;
    CreateSamplerTest(*this, &sampler_info, "VUID-VkSamplerCreateInfo-flags-02580");
    sampler_info.unnormalizedCoordinates = sampler_info_ref.unnormalizedCoordinates;

    // Test image parameters

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.flags = 0;

    // only VK_IMAGE_TYPE_2D is supported
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.extent.height = 1;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02557");

    // only VK_SAMPLE_COUNT_1_BIT is supported
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-samples-02558");

    // tiling must be VK_IMAGE_TILING_OPTIMAL
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02565");

    // only 2D
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02566");

    // no cube maps
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 64;
    image_create_info.arrayLayers = 6;
    image_create_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02567");

    // mipLevels must be 1
    image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    image_create_info.arrayLayers = 1;
    image_create_info.mipLevels = 2;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-02568");

    // Test image view parameters

    // create a valid density map image
    image_create_info.flags = 0;
    image_create_info.mipLevels = 1;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    VkImageObj densityImage(m_device);
    densityImage.init(&image_create_info);
    ASSERT_TRUE(densityImage.initialized());

    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = densityImage.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // density maps can't be sparse (or protected)
    if (m_device->phy().features().sparseResidencyImage2D) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
        image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
        VkImageObj image(m_device);
        image.init(&image_create_info);
        ASSERT_TRUE(image.initialized());

        ivci.image = image.handle();
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-04116");
    }

    if (fdm2Supported) {
        if (!density_map2_features.fragmentDensityMapDeferred) {
            ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT;
            ivci.image = densityImage.handle();
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-03567");
        } else {
            ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DEFERRED_BIT_EXT;
            ivci.flags |= VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
            ivci.image = densityImage.handle();
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-03568");
        }
        if (density_map2_properties.maxSubsampledArrayLayers < properties2.properties.limits.maxImageArrayLayers) {
            image_create_info.flags = VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
            image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
            image_create_info.arrayLayers = density_map2_properties.maxSubsampledArrayLayers + 1;
            VkImageObj image(m_device);
            image.init(&image_create_info);
            ASSERT_TRUE(image.initialized());
            ivci.image = image.handle();
            ivci.flags = 0;
            ivci.subresourceRange.layerCount = density_map2_properties.maxSubsampledArrayLayers + 1;
            CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-03569");
        }
    }
}

TEST_F(VkLayerTest, FragmentDensityMapDisabled) {
    TEST_DESCRIPTION("Checks for when the fragment density map features are not enabled.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool fdmSupported = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    bool fdm2Supported = DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);

    // Check extension support
    if (!(fdmSupported || fdm2Supported)) {
        printf("%s test requires %s or %s extensions. Skipping.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    VkImageObj image2D(m_device);
    image2D.init(&image_create_info);
    ASSERT_TRUE(image2D.initialized());

    VkImageViewCreateInfo ivci = {};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Flags must not be set if the feature is not enabled
    ivci.flags = VK_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-flags-02572");
}

TEST_F(VkLayerTest, AstcDecodeMode) {
    TEST_DESCRIPTION("Tests for VUs for VK_EXT_astc_decode_mode");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_decode_features = lvl_init_struct<VkPhysicalDeviceASTCDecodeFeaturesEXT>();
    astc_decode_features.pNext = nullptr;
    VkPhysicalDeviceFeatures2KHR features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&astc_decode_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!features2.features.textureCompressionASTC_LDR) {
        printf("%s  textureCompressionASTC_LDR feature not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    // Disable feature
    astc_decode_features.decodeModeSharedExponent = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    const VkFormat rgba_format = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat ldr_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;

    VkImageObj image(m_device);
    image.Init(128, 128, 1, rgba_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());
    VkImageObj astc_image(m_device);
    astc_image.Init(128, 128, 1, ldr_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(astc_image.initialized());

    VkImageViewASTCDecodeModeEXT astc_decode_mode = {};
    astc_decode_mode.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ASTC_DECODE_MODE_EXT;
    astc_decode_mode.pNext = nullptr;
    astc_decode_mode.decodeMode = VK_FORMAT_R16G16B16A16_SFLOAT;

    VkImageView image_view;
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = &astc_decode_mode;
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = rgba_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // image view format is not ASTC
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewASTCDecodeModeEXT-format-04084");
    vk::CreateImageView(m_device->device(), &image_view_create_info, nullptr, &image_view);
    m_errorMonitor->VerifyFound();

    // Non-valid decodeMode
    image_view_create_info.image = astc_image.handle();
    image_view_create_info.format = ldr_format;
    astc_decode_mode.decodeMode = ldr_format;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02230");
    vk::CreateImageView(m_device->device(), &image_view_create_info, nullptr, &image_view);
    m_errorMonitor->VerifyFound();

    // decodeModeSharedExponent not enabled
    astc_decode_mode.decodeMode = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02231");
    vk::CreateImageView(m_device->device(), &image_view_create_info, nullptr, &image_view);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CustomBorderColor) {
    TEST_DESCRIPTION("Tests for VUs for VK_EXT_custom_border_color");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
        return;
    }
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceCustomBorderColorFeaturesEXT border_color_features =
        lvl_init_struct<VkPhysicalDeviceCustomBorderColorFeaturesEXT>();
    VkPhysicalDeviceFeatures2KHR features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&border_color_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!border_color_features.customBorderColors) {
        printf("%s Custom border color feature not supported, skipping tests\n", kSkipPrefix);
        return;
    }
    // Disable without format
    border_color_features.customBorderColorWithoutFormat = 0;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkSampler sampler;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.borderColor = VK_BORDER_COLOR_INT_CUSTOM_EXT;
    // No SCBCCreateInfo in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-borderColor-04011");
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();

    VkSamplerCustomBorderColorCreateInfoEXT custom_color_cinfo = {};
    custom_color_cinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
    custom_color_cinfo.format = VK_FORMAT_R32_SFLOAT;
    sampler_info.pNext = &custom_color_cinfo;
    // Format mismatch
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04013");
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();

    custom_color_cinfo.format = VK_FORMAT_UNDEFINED;
    // Format undefined with no customBorderColorWithoutFormat
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04014");
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyFound();

    custom_color_cinfo.format = VK_FORMAT_R8G8B8A8_UINT;
    m_errorMonitor->ExpectSuccess();
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyNotFound();

    VkDescriptorSetLayoutBinding dsl_binding = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, NULL, 0, 1, &dsl_binding};
    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-pImmutableSamplers-04009");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    assert(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceCustomBorderColorPropertiesEXT custom_properties =
        lvl_init_struct<VkPhysicalDeviceCustomBorderColorPropertiesEXT>();
    auto prop2 = lvl_init_struct<VkPhysicalDeviceProperties2KHR>(&custom_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);
    if (custom_properties.maxCustomBorderColorSamplers <= 0xFFFF) {
        VkSampler samplers[0xFFFF];
        // Still have one custom border color sampler from above, so this should exceed max
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-None-04012");
        if (prop2.properties.limits.maxSamplerAllocationCount <= custom_properties.maxCustomBorderColorSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "Number of currently valid sampler objects is not less than the maximum allowed");
        }
        for (uint32_t i = 0; i < custom_properties.maxCustomBorderColorSamplers; i++) {
            vk::CreateSampler(m_device->device(), &sampler_info, NULL, &samplers[i]);
        }
        m_errorMonitor->VerifyFound();
        for (uint32_t i = 0; i < custom_properties.maxCustomBorderColorSamplers - 1; i++) {
            vk::DestroySampler(m_device->device(), samplers[i], nullptr);
        }
    }
    vk::DestroySampler(m_device->device(), sampler, nullptr);
}

TEST_F(VkLayerTest, CustomBorderColorFormatUndefined) {
    TEST_DESCRIPTION("Tests for VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04015");
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    } else {
        printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
        return;
    }
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceCustomBorderColorFeaturesEXT border_color_features =
        lvl_init_struct<VkPhysicalDeviceCustomBorderColorFeaturesEXT>();
    VkPhysicalDeviceFeatures2KHR features2 = lvl_init_struct<VkPhysicalDeviceFeatures2KHR>(&border_color_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (!border_color_features.customBorderColors || !border_color_features.customBorderColorWithoutFormat) {
        printf("%s Custom border color feature not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkSampler sampler;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.borderColor = VK_BORDER_COLOR_INT_CUSTOM_EXT;
    VkSamplerCustomBorderColorCreateInfoEXT custom_color_cinfo = {};
    custom_color_cinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
    custom_color_cinfo.format = VK_FORMAT_UNDEFINED;
    sampler_info.pNext = &custom_color_cinfo;
    m_errorMonitor->ExpectSuccess();
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);
    m_errorMonitor->VerifyNotFound();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B4G4R4A4_UNORM_PACK16, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    image.Layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    vk_testing::ImageView view;
    auto image_view_create_info = SafeSaneImageViewCreateInfo(image, VK_FORMAT_B4G4R4A4_UNORM_PACK16, VK_IMAGE_ASPECT_COLOR_BIT);
    view.init(*m_device, image_view_create_info);

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler;
    img_info.imageView = view.handle();
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].pImageInfo = &img_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);
    char const *fsSource =
        "#version 450\n"
        "\n"
        "layout(set=0, binding=0) uniform sampler2D s;\n"
        "layout(location=0) out vec4 x;\n"
        "void main(){\n"
        "   x = texture(s, vec2(1));\n"
        "}\n";
    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    VkViewport viewport = m_viewports[0];
    VkRect2D scissor = m_scissors[0];
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04015");
    m_commandBuffer->Draw(3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    vk::DestroySampler(m_device->device(), sampler, nullptr);
}

TEST_F(VkLayerTest, InvalidExportExternalImageHandleType) {
    TEST_DESCRIPTION("Test exporting memory with mismatching handleTypes.");

#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

    // Check for external memory instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External memory extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool bind_memory2 = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    if (DeviceExtensionSupported(gpu(), nullptr, ext_mem_extension_name)) {
        m_device_extension_names.push_back(ext_mem_extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        if (bind_memory2) {
            m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        }
    } else {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix, ext_mem_extension_name);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);

    // Create Export Image
    VkImage image_export = VK_NULL_HANDLE;
    VkExternalMemoryImageCreateInfo external_image_info = {};
    external_image_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    external_image_info.pNext = nullptr;
    external_image_info.handleTypes = handle_type;
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = &external_image_info;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    vk::CreateImage(device(), &image_info, NULL, &image_export);

    // Create export memory with different handleType
    VkExportMemoryAllocateInfo export_memory_info = {};
    export_memory_info.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    export_memory_info.pNext = nullptr;
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT_KHR;

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = &export_memory_info;

    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image_export, &mem_reqs);
    alloc_info.allocationSize = mem_reqs.size;
    VkMemoryPropertyFlagBits property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    alloc_info.memoryTypeIndex = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        if ((mem_reqs.memoryTypeBits & (1 << i)) && ((phys_mem_props.memoryTypes[i].propertyFlags & property) == property)) {
            alloc_info.memoryTypeIndex = i;
            break;
        }
    }
    if (alloc_info.memoryTypeIndex >= phys_mem_props.memoryTypeCount) {
        printf("%s No valid memory type index could be found; skipped.\n", kSkipPrefix);
        vk::DestroyImage(device(), image_export, nullptr);
        return;
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-memory-02728");
    vk::BindImageMemory(device(), image_export, memory, 0);
    m_errorMonitor->VerifyFound();

    if (bind_memory2 == true) {
        PFN_vkBindImageMemory2KHR vkBindImageMemory2Function =
            (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");

        VkBindImageMemoryInfo bind_image_info = {};
        bind_image_info.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
        bind_image_info.pNext = nullptr;
        bind_image_info.image = image_export;
        bind_image_info.memory = memory;
        bind_image_info.memoryOffset = 0;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-memory-02728");
        vkBindImageMemory2Function(device(), 1, &bind_image_info);
        m_errorMonitor->VerifyFound();
    }

    vk::FreeMemory(device(), memory, nullptr);
    vk::DestroyImage(device(), image_export, nullptr);
}

TEST_F(VkLayerTest, InvalidExportExternalBufferHandleType) {
    TEST_DESCRIPTION("Test exporting memory with mismatching handleTypes.");

#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
#endif

    // Check for external memory instance extensions
    if (InstanceExtensionSupported(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s External memory extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool bind_memory2 = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    if (DeviceExtensionSupported(gpu(), nullptr, ext_mem_extension_name)) {
        m_device_extension_names.push_back(ext_mem_extension_name);
        m_device_extension_names.push_back(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        if (bind_memory2) {
            m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        }
    } else {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix, ext_mem_extension_name);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);

    // Create Export Buffer
    VkBuffer buffer_export = VK_NULL_HANDLE;
    VkExternalMemoryBufferCreateInfo external_buffer_info = {};
    external_buffer_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
    external_buffer_info.pNext = nullptr;
    external_buffer_info.handleTypes = handle_type;
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.pNext = &external_buffer_info;
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.size = 4096;
    vk::CreateBuffer(device(), &buffer_info, NULL, &buffer_export);

    // Create export memory with different handleType
    VkExportMemoryAllocateInfo export_memory_info = {};
    export_memory_info.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
    export_memory_info.pNext = nullptr;
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT_KHR;

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = &export_memory_info;

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), buffer_export, &mem_reqs);
    alloc_info.allocationSize = mem_reqs.size;
    VkMemoryPropertyFlagBits property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    alloc_info.memoryTypeIndex = phys_mem_props.memoryTypeCount + 1;
    for (uint32_t i = 0; i < phys_mem_props.memoryTypeCount; i++) {
        if ((mem_reqs.memoryTypeBits & (1 << i)) && ((phys_mem_props.memoryTypes[i].propertyFlags & property) == property)) {
            alloc_info.memoryTypeIndex = i;
            break;
        }
    }
    if (alloc_info.memoryTypeIndex >= phys_mem_props.memoryTypeCount) {
        printf("%s No valid memory type index could be found; skipped.\n", kSkipPrefix);
        vk::DestroyBuffer(device(), buffer_export, nullptr);
        return;
    }

    VkDeviceMemory memory = VK_NULL_HANDLE;
    vk::AllocateMemory(device(), &alloc_info, NULL, &memory);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-memory-02726");
    vk::BindBufferMemory(device(), buffer_export, memory, 0);
    m_errorMonitor->VerifyFound();

    if (bind_memory2 == true) {
        PFN_vkBindBufferMemory2KHR vkBindBufferMemory2Function =
            (PFN_vkBindBufferMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindBufferMemory2KHR");

        VkBindBufferMemoryInfo bind_buffer_info = {};
        bind_buffer_info.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
        bind_buffer_info.pNext = nullptr;
        bind_buffer_info.buffer = buffer_export;
        bind_buffer_info.memory = memory;
        bind_buffer_info.memoryOffset = 0;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemoryInfo-memory-02726");
        vkBindBufferMemory2Function(device(), 1, &bind_buffer_info);
        m_errorMonitor->VerifyFound();
    }

    vk::FreeMemory(device(), memory, nullptr);
    vk::DestroyBuffer(device(), buffer_export, nullptr);
}

TEST_F(VkLayerTest, ValidSwapchainImageParams) {
    TEST_DESCRIPTION("Swapchain with invalid implied image creation parameters");
    const char *vuid = "VUID-VkSwapchainCreateInfoKHR-imageFormat-01778";

    if (!AddSurfaceInstanceExtension()) {
        printf("%s surface extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AddSwapchainDeviceExtension()) {
        printf("%s swapchain extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    VkDeviceGroupDeviceCreateInfo device_group_ci = {};
    device_group_ci.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
    device_group_ci.physicalDeviceCount = 1;
    VkPhysicalDevice pdev = gpu();
    device_group_ci.pPhysicalDevices = &pdev;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &device_group_ci));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    if (!InitSurface()) {
        printf("%s Cannot create surface, skipping test\n", kSkipPrefix);
        return;
    }

    VkSurfaceCapabilitiesKHR capabilities;
    vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->phy().handle(), m_surface, &capabilities);

    uint32_t format_count;
    vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, nullptr);
    vector<VkSurfaceFormatKHR> formats;
    if (format_count != 0) {
        formats.resize(format_count);
        vk::GetPhysicalDeviceSurfaceFormatsKHR(m_device->phy().handle(), m_surface, &format_count, formats.data());
    }

    uint32_t present_mode_count;
    vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), m_surface, &present_mode_count, nullptr);
    vector<VkPresentModeKHR> present_modes;
    if (present_mode_count != 0) {
        present_modes.resize(present_mode_count);
        vk::GetPhysicalDeviceSurfacePresentModesKHR(m_device->phy().handle(), m_surface, &present_mode_count, present_modes.data());
    }

    VkSwapchainCreateInfoKHR good_create_info = {};
    good_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    good_create_info.pNext = 0;
    good_create_info.surface = m_surface;
    good_create_info.minImageCount = capabilities.minImageCount;
    good_create_info.imageFormat = formats[0].format;
    good_create_info.imageColorSpace = formats[0].colorSpace;
    good_create_info.imageExtent = {capabilities.minImageExtent.width, capabilities.minImageExtent.height};
    good_create_info.imageArrayLayers = 1;
    good_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    good_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    good_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    good_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
#else
    good_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#endif
    good_create_info.presentMode = present_modes[0];
    good_create_info.clipped = VK_FALSE;
    good_create_info.oldSwapchain = 0;

    VkSwapchainCreateInfoKHR create_info_bad_usage = good_create_info;
    bool found_bad_usage = false;
    // Trying to find format+usage combination supported by surface, but not supported by image.
    const std::array<VkImageUsageFlags, 5> kImageUsageFlags = {{
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
    }};

    for (uint32_t i = 0; i < kImageUsageFlags.size() && !found_bad_usage; ++i) {
        if ((capabilities.supportedUsageFlags & kImageUsageFlags[i]) != 0) {
            for (uint32_t j = 0; j < format_count; ++j) {
                VkImageFormatProperties image_format_properties = {};
                VkResult image_format_properties_result = vk::GetPhysicalDeviceImageFormatProperties(
                    m_device->phy().handle(), formats[j].format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, kImageUsageFlags[i], 0,
                    &image_format_properties);

                if (image_format_properties_result != VK_SUCCESS) {
                    create_info_bad_usage.imageFormat = formats[j].format;
                    create_info_bad_usage.imageUsage = kImageUsageFlags[i];
                    found_bad_usage = true;
                    break;
                }
            }
        }
    }
    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        printf("%s Graphics queue does not support present, skipping test\n", kSkipPrefix);
        return;
    }

    if (found_bad_usage) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        vk::CreateSwapchainKHR(device(), &create_info_bad_usage, nullptr, &m_swapchain);
        m_errorMonitor->VerifyFound();
    } else {
        printf(
            "%s could not find imageFormat and imageUsage values, supported by "
            "surface but unsupported by image, skipping test\n",
            kSkipPrefix);
    }
    vk::DestroySwapchainKHR(device(), m_swapchain, nullptr);

    VkImageFormatProperties props;
    VkResult res = vk::GetPhysicalDeviceImageFormatProperties(gpu(), good_create_info.imageFormat, VK_IMAGE_TYPE_2D,
                                                              VK_IMAGE_TILING_OPTIMAL, good_create_info.imageUsage,
                                                              VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT, &props);
    if (res != VK_SUCCESS) {
        printf("%s Swapchain image format does not support SPLIT_INSTANCE_BIND_REGIONS, skipping test\n", kSkipPrefix);
        return;
    }

    VkSwapchainCreateInfoKHR create_info_bad_flags = good_create_info;
    create_info_bad_flags.flags = VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSwapchainCreateInfoKHR-physicalDeviceCount-01429");
    vk::CreateSwapchainKHR(device(), &create_info_bad_flags, nullptr, &m_swapchain);
    m_errorMonitor->VerifyFound();
    DestroySwapchain();
}

TEST_F(VkSyncValTest, SyncBufferCopyHazards) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    if (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_BUFFER_MARKER_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_AMD_BUFFER_MARKER_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    bool has_amd_buffer_maker = DeviceExtensionEnabled(VK_AMD_BUFFER_MARKER_EXTENSION_NAME);

    VkBufferObj buffer_a;
    VkBufferObj buffer_b;
    VkBufferObj buffer_c;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    buffer_a.init_as_src_and_dst(*m_device, 256, mem_prop);
    buffer_b.init_as_src_and_dst(*m_device, 256, mem_prop);
    buffer_c.init_as_src_and_dst(*m_device, 256, mem_prop);

    VkBufferCopy region = {0, 0, 256};
    VkBufferCopy front2front = {0, 0, 128};
    VkBufferCopy front2back = {0, 128, 128};
    VkBufferCopy back2back = {128, 128, 128};

    auto cb = m_commandBuffer->handle();
    m_commandBuffer->begin();

    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto buffer_barrier = lvl_init_struct<VkBufferMemoryBarrier>();
    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.buffer = buffer_a.handle();
    buffer_barrier.offset = 0;
    buffer_barrier.size = 256;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2front);
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &back2back);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_a.handle(), 1, &front2back);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyBuffer(cb, buffer_c.handle(), buffer_b.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto mem_barrier = lvl_init_struct<VkMemoryBarrier>();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->ExpectSuccess();

    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_c.handle(), buffer_b.handle(), 1, &region);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protect C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_b.handle(), buffer_c.handle(), 1, &region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // CmdFillBuffer
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, 256, 1);
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_b.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdFillBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, 256, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // CmdUpdateBuffer
    int i = 10;
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, sizeof(i), &i);
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_b.handle(), buffer_a.handle(), 1, &region);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdUpdateBuffer(m_commandBuffer->handle(), buffer_a.handle(), 0, sizeof(i), &i);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // CmdWriteBufferMarkerAMD
    if (has_amd_buffer_maker) {
        auto fpCmdWriteBufferMarkerAMD =
            (PFN_vkCmdWriteBufferMarkerAMD)vk::GetDeviceProcAddr(m_device->device(), "vkCmdWriteBufferMarkerAMD");
        if (!fpCmdWriteBufferMarkerAMD) {
            printf("%s Test requires unsupported vkCmdWriteBufferMarkerAMD feature. Skipped.\n", kSkipPrefix);
        } else {
            m_errorMonitor->ExpectSuccess();
            m_commandBuffer->reset();
            m_commandBuffer->begin();
            fpCmdWriteBufferMarkerAMD(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, buffer_a.handle(), 0, 1);
            m_commandBuffer->end();
            m_errorMonitor->VerifyNotFound();

            m_commandBuffer->reset();
            m_commandBuffer->begin();
            vk::CmdCopyBuffer(cb, buffer_b.handle(), buffer_a.handle(), 1, &region);
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
            fpCmdWriteBufferMarkerAMD(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, buffer_a.handle(), 0, 1);
            m_errorMonitor->VerifyFound();
            m_commandBuffer->end();
        }
    } else {
        printf("%s Test requires unsupported vkCmdWriteBufferMarkerAMD feature. Skipped.\n", kSkipPrefix);
    }
}

TEST_F(VkSyncValTest, SyncCopyOptimalImageHazards) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    ASSERT_TRUE(image_a.initialized());

    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    ASSERT_TRUE(image_b.initialized());

    VkImageObj image_c(m_device);
    image_c.Init(image_ci);
    ASSERT_TRUE(image_c.initialized());

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 2};
    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_0_to_0 = {layers_0, zero_offset, layers_0, zero_offset, full_extent};
    VkImageCopy region_0_to_1 = {layers_0, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_1_to_1 = {layers_1, zero_offset, layers_1, zero_offset, full_extent};
    VkImageCopy region_0_front = {layers_0, zero_offset, layers_0, zero_offset, half_extent};
    VkImageCopy region_0_back = {layers_0, half_offset, layers_0, half_offset, half_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto image_barrier = lvl_init_struct<VkImageMemoryBarrier>();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_1_to_1);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_to_1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto mem_barrier = lvl_init_struct<VkMemoryBarrier>();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyNotFound();

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_0_back);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();

    // CmdResolveImage
    VkImageFormatProperties formProps = {{0, 0, 0}, 0, 0, 0, 0};
    vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TYPE_2D,
                                               VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 0, &formProps);

    if (!(formProps.sampleCounts & VK_SAMPLE_COUNT_2_BIT)) {
        printf("%s CmdResolveImage Test requires unsupported VK_SAMPLE_COUNT_2_BIT feature. Skipped.\n", kSkipPrefix);
    } else {
        m_errorMonitor->ExpectSuccess();
        VkImageObj image_s2_a(m_device), image_s2_b(m_device);
        image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
        image_s2_a.Init(image_ci);
        ASSERT_TRUE(image_s2_a.initialized());

        image_s2_b.Init(image_ci);
        ASSERT_TRUE(image_s2_b.initialized());

        VkImageResolve r_full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        image_s2_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        image_s2_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
        vk::CmdResolveImage(cb, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_commandBuffer->end();
        m_errorMonitor->VerifyNotFound();

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdCopyImage(cb, image_s2_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                         &full_region);
        vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
        vk::CmdResolveImage(cb, image_s2_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_errorMonitor->VerifyFound();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
        vk::CmdResolveImage(cb, image_s2_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                            &r_full_region);
        m_errorMonitor->VerifyFound();
        m_commandBuffer->end();
    }
}

TEST_F(VkSyncValTest, SyncCopyOptimalMultiPlanarHazards) {
    // TODO: Add code to enable sync validation
    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
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

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    VkImageObj image_a(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    // Verify format
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), image_ci,
                                                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    image_a.Init(image_ci);
    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    VkImageObj image_c(m_device);
    image_c.Init(image_ci);

    VkImageSubresourceLayers layer_all_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT_KHR, 0, 0, 2};
    VkImageSubresourceLayers layer0_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT_KHR, 0, 0, 1};
    VkImageSubresourceLayers layer0_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT_KHR, 0, 0, 1};
    VkImageSubresourceLayers layer1_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT_KHR, 0, 1, 1};
    VkImageSubresourceRange full_subresource_range{
        VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR, 0, 1, 0, 2};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D one_four_offset{32, 32, 0};
    VkExtent3D full_extent{128, 128, 1};    // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};      // <-- image type is 2D
    VkExtent3D one_four_extent{32, 32, 1};  // <-- image type is 2D

    VkImageCopy region_all_plane0_to_all_plane0 = {layer_all_plane0, zero_offset, layer_all_plane0, zero_offset, full_extent};
    VkImageCopy region_layer0_plane0_to_layer0_plane0 = {layer0_plane0, zero_offset, layer0_plane0, zero_offset, full_extent};
    VkImageCopy region_layer0_plane0_to_layer0_plane1 = {layer0_plane0, zero_offset, layer0_plane1, zero_offset, half_extent};
    VkImageCopy region_layer1_plane1_to_layer1_plane1_front = {layer1_plane1, zero_offset, layer1_plane1, zero_offset,
                                                               one_four_extent};
    VkImageCopy region_layer1_plane1_to_layer1_plane1_back = {layer1_plane1, one_four_offset, layer1_plane1, one_four_offset,
                                                              one_four_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto image_barrier = lvl_init_struct<VkImageMemoryBarrier>();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane1);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer0_plane0_to_layer0_plane1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto mem_barrier = lvl_init_struct<VkMemoryBarrier>();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyNotFound();

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_all_plane0_to_all_plane0);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_layer1_plane1_to_layer1_plane1_back);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();
}

TEST_F(VkSyncValTest, SyncCopyLinearImageHazards) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_LINEAR);
    image_a.Init(image_ci);
    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    VkImageObj image_c(m_device);
    image_c.Init(image_ci);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{64, 64, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};    // <-- image type is 2D

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};
    VkImageCopy region_front = {layers_all, zero_offset, layers_all, zero_offset, half_extent};
    VkImageCopy region_back = {layers_all, half_offset, layers_all, half_offset, half_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto image_barrier = lvl_init_struct<VkImageMemoryBarrier>();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_b.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyNotFound();

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_back);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkSyncValTest, SyncCopyLinearMultiPlanarHazards) {
    // TODO: Add code to enable sync validation
    // Enable KHR multiplane req'd extensions
    bool mp_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
                                                    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION);
    if (mp_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
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

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    VkImageObj image_a(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_LINEAR);
    // Verify format
    bool supported = ImageFormatAndFeaturesSupported(instance(), gpu(), image_ci,
                                                     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
    if (!supported) {
        printf("%s Multiplane image format not supported.  Skipping test.\n", kSkipPrefix);
        return;  // Assume there's low ROI on searching for different mp formats
    }

    image_a.Init(image_ci);
    VkImageObj image_b(m_device);
    image_b.Init(image_ci);
    VkImageObj image_c(m_device);
    image_c.Init(image_ci);

    VkImageSubresourceLayers layer_all_plane0{VK_IMAGE_ASPECT_PLANE_0_BIT_KHR, 0, 0, 1};
    VkImageSubresourceLayers layer_all_plane1{VK_IMAGE_ASPECT_PLANE_1_BIT_KHR, 0, 0, 1};
    VkImageSubresourceRange full_subresource_range{
        VK_IMAGE_ASPECT_PLANE_0_BIT_KHR | VK_IMAGE_ASPECT_PLANE_1_BIT_KHR | VK_IMAGE_ASPECT_PLANE_2_BIT_KHR, 0, 1, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D one_four_offset{32, 32, 0};
    VkExtent3D full_extent{128, 128, 1};    // <-- image type is 2D
    VkExtent3D half_extent{64, 64, 1};      // <-- image type is 2D
    VkExtent3D one_four_extent{32, 32, 1};  // <-- image type is 2D

    VkImageCopy region_plane0_to_plane0 = {layer_all_plane0, zero_offset, layer_all_plane0, zero_offset, full_extent};
    VkImageCopy region_plane0_to_plane1 = {layer_all_plane0, zero_offset, layer_all_plane1, zero_offset, half_extent};
    VkImageCopy region_plane1_to_plane1_front = {layer_all_plane1, zero_offset, layer_all_plane1, zero_offset, one_four_extent};
    VkImageCopy region_plane1_to_plane1_back = {layer_all_plane1, one_four_offset, layer_all_plane1, one_four_offset,
                                                one_four_extent};

    m_commandBuffer->begin();

    image_c.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto image_barrier = lvl_init_struct<VkImageMemoryBarrier>();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.image = image_a.handle();
    image_barrier.subresourceRange = full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane1);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane1);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    // NOTE: Since the previous command skips in validation, the state update is never done, and the validation layer thus doesn't
    //       record the write operation to b.  So we'll need to repeat it successfully to set up for the *next* test.

    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    auto mem_barrier = lvl_init_struct<VkMemoryBarrier>();
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyNotFound();

    // Use barrier to protect last reader, but not last writer...
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    mem_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;  // Protects C but not B
    mem_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mem_barrier, 0, nullptr, 0,
                           nullptr);
    vk::CmdCopyImage(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane0_to_plane0);
    m_errorMonitor->VerifyFound();

    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_front);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImage(cb, image_c.handle(), VK_IMAGE_LAYOUT_GENERAL, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_plane1_to_plane1_back);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();
}

TEST_F(VkSyncValTest, SyncCopyBufferImageHazards) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkBufferObj buffer_a, buffer_b;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    buffer_a.init_as_src_and_dst(*m_device, 2048, mem_prop);
    buffer_b.init_as_src_and_dst(*m_device, 2048, mem_prop);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_offset{16, 16, 0};
    VkExtent3D half_extent{16, 16, 1};  // <-- image type is 2D

    VkBufferImageCopy region_buffer_front_image_0_front = {0, 16, 16, layers_0, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_front_image_1_front = {0, 16, 16, layers_1, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_front_image_1_back = {0, 16, 16, layers_1, half_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_0_front = {1024, 16, 16, layers_0, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_0_back = {1024, 16, 16, layers_0, half_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_1_front = {1024, 16, 16, layers_1, zero_offset, half_extent};
    VkBufferImageCopy region_buffer_back_image_1_back = {1024, 16, 16, layers_1, half_offset, half_extent};

    m_commandBuffer->begin();
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();
    vk::CmdCopyBufferToImage(cb, buffer_a.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_a.handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_back_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_front_image_1_back);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1, &region_buffer_back_image_0_back);
    m_errorMonitor->VerifyNotFound();

    auto buffer_barrier = lvl_init_struct<VkBufferMemoryBarrier>();
    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    buffer_barrier.buffer = buffer_a.handle();
    buffer_barrier.offset = 1024;
    buffer_barrier.size = 2048;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1,
                             &region_buffer_back_image_1_front);
    m_errorMonitor->VerifyNotFound();

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyImageToBuffer(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_a.handle(), 1, &region_buffer_back_image_1_back);
    m_errorMonitor->VerifyNotFound();

    vk::CmdCopyImageToBuffer(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_b.handle(), 1,
                             &region_buffer_front_image_0_front);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyImageToBuffer(cb, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, buffer_b.handle(), 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_back_image_0_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_1_front);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_front_image_1_back);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_buffer_back_image_0_back);
    m_errorMonitor->VerifyNotFound();

    buffer_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buffer_barrier.buffer = buffer_b.handle();
    buffer_barrier.offset = 1024;
    buffer_barrier.size = 2048;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                             &region_buffer_back_image_1_front);
    m_errorMonitor->VerifyNotFound();

    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0,
                           nullptr);

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyBufferToImage(cb, buffer_b.handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_buffer_back_image_1_back);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();
}

TEST_F(VkSyncValTest, SyncBlitImageHazards) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkImageSubresourceLayers layers_0{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageSubresourceLayers layers_1{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkOffset3D half_0_offset{16, 16, 0};
    VkOffset3D half_1_offset{16, 16, 1};
    VkOffset3D full_offset{32, 32, 1};
    VkImageBlit region_0_front_1_front = {layers_0, {zero_offset, half_1_offset}, layers_1, {zero_offset, half_1_offset}};
    VkImageBlit region_1_front_0_front = {layers_1, {zero_offset, half_1_offset}, layers_0, {zero_offset, half_1_offset}};
    VkImageBlit region_1_back_0_back = {layers_1, {half_0_offset, full_offset}, layers_0, {half_0_offset, full_offset}};

    m_commandBuffer->begin();
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();

    vk::CmdBlitImage(cb, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front_1_front, VK_FILTER_NEAREST);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdBlitImage(cb, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_0_front_1_front, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdBlitImage(cb, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_1_front_0_front, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    vk::CmdBlitImage(cb, image_b.image(), VK_IMAGE_LAYOUT_GENERAL, image_a.image(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &region_1_back_0_back, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->end();
}

TEST_F(VkSyncValTest, SyncRenderPassBeginTransitionHazard) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(2));

    // Render Target Information
    auto width = static_cast<uint32_t>(m_width);
    auto height = static_cast<uint32_t>(m_height);
    auto *rt_0 = m_renderTargets[0].get();
    auto *rt_1 = m_renderTargets[1].get();

    // Other buffers with which to interact
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    const auto image_ci = VkImageObj::ImageCreateInfo2D(width, height, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{width, height, 1};  // <-- image type is 2D
    VkImageSubresourceLayers layer_color{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkImageCopy region_to_copy = {layer_color, zero_offset, layer_color, zero_offset, full_extent};

    auto cb = m_commandBuffer->handle();

    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->begin();
    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    rt_0->SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    rt_1->SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    rt_0->SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, rt_0->handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_to_copy);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);  // This fails so the driver call is skip and no end is valid
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    // Use the barrier to clean up the WAW, and try again. (and show that validation is accounting for the barrier effect too.)
    VkImageSubresourceRange rt_full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    auto image_barrier = lvl_init_struct<VkImageMemoryBarrier>();
    image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    image_barrier.dstAccessMask = 0;
    image_barrier.image = rt_0->handle();
    image_barrier.subresourceRange = rt_full_subresource_range;
    image_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &image_barrier);
    vk::CmdCopyImage(cb, rt_1->handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region_to_copy);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);  // This fails so the driver call is skip and no end is valid
    m_errorMonitor->VerifyFound();

    m_errorMonitor->ExpectSuccess();
    // A global execution barrier that the implict external dependency can chain with should work...
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 0,
                           nullptr);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkSyncValTest, SyncCmdDispatchDrawHazards) {
    // TODO: Add code to enable sync validation
    SetTargetApiVersion(VK_API_VERSION_1_2);

    // Enable VK_KHR_draw_indirect_count for KHR variants
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    VkPhysicalDeviceVulkan12Features features12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr};
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        if (DeviceValidationVersion() >= VK_API_VERSION_1_2) {
            features12.drawIndirectCount = VK_TRUE;
        }
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features12, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    bool has_khr_indirect = DeviceExtensionEnabled(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageUsageFlags image_usage_combine = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_c_a(m_device), image_c_b(m_device);
    const auto image_c_ci = VkImageObj::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_combine, VK_IMAGE_TILING_OPTIMAL);
    image_c_a.Init(image_c_ci);
    image_c_b.Init(image_c_ci);

    VkImageView imageview_c = image_c_a.targetView(format);
    VkImageUsageFlags image_usage_storage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageObj image_s_a(m_device), image_s_b(m_device);
    const auto image_s_ci = VkImageObj::ImageCreateInfo2D(16, 16, 1, 1, format, image_usage_storage, VK_IMAGE_TILING_OPTIMAL);
    image_s_a.Init(image_s_ci);
    image_s_b.Init(image_s_ci);
    image_s_a.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_s_b.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkImageView imageview_s = image_s_a.targetView(format);

    VkSampler sampler_s, sampler_c;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkResult err = vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler_s);
    ASSERT_VK_SUCCESS(err);
    err = vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler_c);
    ASSERT_VK_SUCCESS(err);

    VkBufferObj buffer_a, buffer_b;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlags buffer_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_a.init(*m_device, buffer_a.create_info(2048, buffer_usage, nullptr), mem_prop);
    buffer_b.init(*m_device, buffer_b.create_info(2048, buffer_usage, nullptr), mem_prop);

    VkBufferView bufferview;
    auto bvci = lvl_init_struct<VkBufferViewCreateInfo>();
    bvci.buffer = buffer_a.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.offset = 0;
    bvci.range = VK_WHOLE_SIZE;

    err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &bufferview);
    ASSERT_VK_SUCCESS(err);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {3, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a.handle(), 2048);
    descriptor_set.WriteDescriptorImageInfo(1, imageview_c, sampler_c, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                            VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorImageInfo(2, imageview_s, sampler_s, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
    descriptor_set.WriteDescriptorBufferView(3, bufferview);
    descriptor_set.UpdateDescriptorSets();

    // Dispatch
    std::string csSource =
        "#version 450\n"
        "layout(set=0, binding=0) uniform foo { float x; } ub0;\n"
        "layout(set=0, binding=1) uniform sampler2D cis1;\n"
        "layout(set=0, binding=2, rgba8) uniform readonly image2D si2;\n"
        "layout(set=0, binding=3, r32f) uniform readonly imageBuffer stb3;\n"
        "void main(){\n"
        "    vec4 vColor4;\n"
        "    vColor4.x = ub0.x;\n"
        "    vColor4 = texture(cis1, vec2(0));\n"
        "    vColor4 = imageLoad(si2, ivec2(0));\n"
        "    vColor4 = imageLoad(stb3, 0);\n"
        "}\n";

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cs_.reset(new VkShaderObj(m_device, csSource.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, this));
    pipe.InitState();
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    pipe.CreateComputePipeline();

    m_commandBuffer->begin();

    VkBufferCopy buffer_region = {0, 0, 2048};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_b.handle(), buffer_a.handle(), 1, &buffer_region);

    VkImageSubresourceLayers layer{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{16, 16, 1};
    VkImageCopy image_region = {layer, zero_offset, layer, zero_offset, full_extent};
    vk::CmdCopyImage(m_commandBuffer->handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    vk::CmdCopyImage(m_commandBuffer->handle(), image_s_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_s_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
    m_commandBuffer->reset();
    m_commandBuffer->begin();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_b.handle(), buffer_a.handle(), 1, &buffer_region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyImage(m_commandBuffer->handle(), image_c_b.handle(), VK_IMAGE_LAYOUT_GENERAL, image_c_a.handle(),
                     VK_IMAGE_LAYOUT_GENERAL, 1, &image_region);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
    m_commandBuffer->reset();

    // DispatchIndirect
    m_errorMonitor->ExpectSuccess();
    VkBufferObj buffer_dispatchIndirect, buffer_dispatchIndirect2;
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_dispatchIndirect.init(
        *m_device, buffer_dispatchIndirect.create_info(sizeof(VkDispatchIndirectCommand), buffer_usage, nullptr), mem_prop);
    buffer_dispatchIndirect2.init(
        *m_device, buffer_dispatchIndirect2.create_info(sizeof(VkDispatchIndirectCommand), buffer_usage, nullptr), mem_prop);
    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), buffer_dispatchIndirect.handle(), 0);
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(VkDispatchIndirectCommand)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_dispatchIndirect2.handle(), buffer_dispatchIndirect.handle(), 1,
                      &buffer_region);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdDispatchIndirect(m_commandBuffer->handle(), buffer_dispatchIndirect.handle(), 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();

    // Draw
    m_errorMonitor->ExpectSuccess();
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkVertexInputAttributeDescription VertexInputAttributeDescription = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(vbo_data)};
    VkVertexInputBindingDescription VertexInputBindingDescription = {0, sizeof(vbo_data), VK_VERTEX_INPUT_RATE_VERTEX};
    VkBufferObj vbo, vbo2;
    buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vbo.init(*m_device, vbo.create_info(sizeof(vbo_data), buffer_usage, nullptr), mem_prop);
    vbo2.init(*m_device, vbo2.create_info(sizeof(vbo_data), buffer_usage, nullptr), mem_prop);

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, csSource.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.InitState();
    g_pipe.vi_ci_.pVertexBindingDescriptions = &VertexInputBindingDescription;
    g_pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    g_pipe.vi_ci_.pVertexAttributeDescriptions = &VertexInputAttributeDescription;
    g_pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(vbo_data)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), vbo2.handle(), vbo.handle(), 1, &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // DrawIndexed
    m_errorMonitor->ExpectSuccess();
    const float ibo_data[3] = {0.f, 0.f, 0.f};
    VkBufferObj ibo, ibo2;
    buffer_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    ibo.init(*m_device, ibo.create_info(sizeof(ibo_data), buffer_usage, nullptr), mem_prop);
    ibo2.init(*m_device, ibo2.create_info(sizeof(ibo_data), buffer_usage, nullptr), mem_prop);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_commandBuffer->DrawIndexed(3, 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(ibo_data)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), ibo2.handle(), ibo.handle(), 1, &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    m_commandBuffer->DrawIndexed(3, 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // DrawIndirect
    m_errorMonitor->ExpectSuccess();
    VkBufferObj buffer_drawIndirect, buffer_drawIndirect2;
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_drawIndirect.init(*m_device, buffer_drawIndirect.create_info(sizeof(VkDrawIndirectCommand), buffer_usage, nullptr),
                             mem_prop);
    buffer_drawIndirect2.init(*m_device, buffer_drawIndirect2.create_info(sizeof(VkDrawIndirectCommand), buffer_usage, nullptr),
                              mem_prop);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(VkDrawIndirectCommand)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_drawIndirect2.handle(), buffer_drawIndirect.handle(), 1, &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdDrawIndirect(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // DrawIndexedIndirect
    m_errorMonitor->ExpectSuccess();
    VkBufferObj buffer_drawIndexedIndirect, buffer_drawIndexedIndirect2;
    buffer_usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_drawIndexedIndirect.init(
        *m_device, buffer_drawIndexedIndirect.create_info(sizeof(VkDrawIndexedIndirectCommand), buffer_usage, nullptr), mem_prop);
    buffer_drawIndexedIndirect2.init(
        *m_device, buffer_drawIndexedIndirect2.create_info(sizeof(VkDrawIndexedIndirectCommand), buffer_usage, nullptr), mem_prop);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, 1, sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    buffer_region = {0, 0, sizeof(VkDrawIndexedIndirectCommand)};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_drawIndexedIndirect2.handle(), buffer_drawIndexedIndirect.handle(), 1,
                      &buffer_region);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
    vk::CmdDrawIndexedIndirect(m_commandBuffer->handle(), buffer_drawIndexedIndirect.handle(), 0, 1,
                               sizeof(VkDrawIndexedIndirectCommand));
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    if (has_khr_indirect) {
        // DrawIndirectCount
        auto fpCmdDrawIndirectCountKHR =
            (PFN_vkCmdDrawIndirectCount)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndirectCountKHR");
        if (!fpCmdDrawIndirectCountKHR) {
            printf("%s Test requires unsupported vkCmdDrawIndirectCountKHR feature. Skipped.\n", kSkipPrefix);
        } else {
            m_errorMonitor->ExpectSuccess();
            VkBufferObj buffer_count, buffer_count2;
            buffer_usage =
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            buffer_count.init(*m_device, buffer_count.create_info(sizeof(uint32_t), buffer_usage, nullptr), mem_prop);
            buffer_count2.init(*m_device, buffer_count2.create_info(sizeof(uint32_t), buffer_usage, nullptr), mem_prop);

            m_commandBuffer->reset();
            m_commandBuffer->begin();
            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
            vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);
            fpCmdDrawIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, buffer_count.handle(), 0, 1,
                                      sizeof(VkDrawIndirectCommand));
            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();
            m_errorMonitor->VerifyNotFound();

            m_commandBuffer->reset();
            m_commandBuffer->begin();

            buffer_region = {0, 0, sizeof(uint32_t)};
            vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_count2.handle(), buffer_count.handle(), 1, &buffer_region);

            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
            vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
            fpCmdDrawIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndirect.handle(), 0, buffer_count.handle(), 0, 1,
                                      sizeof(VkDrawIndirectCommand));
            m_errorMonitor->VerifyFound();

            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();
        }

        // DrawIndexedIndirectCount
        auto fpCmdDrawIndexIndirectCountKHR =
            (PFN_vkCmdDrawIndirectCount)vk::GetDeviceProcAddr(m_device->device(), "vkCmdDrawIndexedIndirectCountKHR");
        if (!fpCmdDrawIndexIndirectCountKHR) {
            printf("%s Test requires unsupported vkCmdDrawIndexedIndirectCountKHR feature. Skipped.\n", kSkipPrefix);
        } else {
            m_errorMonitor->ExpectSuccess();
            VkBufferObj buffer_count, buffer_count2;
            buffer_usage =
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            buffer_count.init(*m_device, buffer_count.create_info(sizeof(uint32_t), buffer_usage, nullptr), mem_prop);
            buffer_count2.init(*m_device, buffer_count2.create_info(sizeof(uint32_t), buffer_usage, nullptr), mem_prop);

            m_commandBuffer->reset();
            m_commandBuffer->begin();
            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
            vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
            vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);
            fpCmdDrawIndexIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndexedIndirect.handle(), 0, buffer_count.handle(),
                                           0, 1, sizeof(VkDrawIndexedIndirectCommand));
            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();
            m_errorMonitor->VerifyNotFound();

            m_commandBuffer->reset();
            m_commandBuffer->begin();

            buffer_region = {0, 0, sizeof(uint32_t)};
            vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_count2.handle(), buffer_count.handle(), 1, &buffer_region);

            m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
            vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT16);
            vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
            vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
            vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
            vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(),
                                      0, 1, &descriptor_set.set_, 0, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-READ_AFTER_WRITE");
            fpCmdDrawIndexIndirectCountKHR(m_commandBuffer->handle(), buffer_drawIndexedIndirect.handle(), 0, buffer_count.handle(),
                                           0, 1, sizeof(VkDrawIndexedIndirectCommand));
            m_errorMonitor->VerifyFound();

            m_commandBuffer->EndRenderPass();
            m_commandBuffer->end();
        }
    } else {
        printf("%s Test requires unsupported vkCmdDrawIndirectCountKHR & vkDrawIndexedIndirectCountKHR feature. Skipped.\n",
               kSkipPrefix);
    }
}

TEST_F(VkSyncValTest, SyncCmdClear) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    // CmdClearColorImage
    m_errorMonitor->ExpectSuccess();
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_a(m_device), image_b(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_a.Init(image_ci);
    image_b.Init(image_ci);

    VkImageSubresourceLayers layers_all{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    VkOffset3D zero_offset{0, 0, 0};
    VkExtent3D full_extent{128, 128, 1};  // <-- image type is 2D
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageCopy full_region = {layers_all, zero_offset, layers_all, zero_offset, full_extent};

    m_commandBuffer->begin();

    image_b.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    image_a.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    auto cb = m_commandBuffer->handle();
    VkClearColorValue ccv = {};
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyImage(cb, image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_b.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1, &full_subresource_range);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

    // CmdClearDepthStencilImage
    format = FindSupportedDepthStencilFormat(gpu());
    if (!format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    m_errorMonitor->ExpectSuccess();
    VkImageObj image_ds_a(m_device), image_ds_b(m_device);
    image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image_ds_a.Init(image_ci);
    image_ds_b.Init(image_ci);

    const VkImageAspectFlags ds_aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    image_ds_a.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_GENERAL);
    image_ds_b.SetLayout(ds_aspect, VK_IMAGE_LAYOUT_GENERAL);

    m_commandBuffer->begin();
    const VkClearDepthStencilValue clear_value = {};
    VkImageSubresourceRange ds_range = {ds_aspect, 0, 1, 0, 1};

    vk::CmdClearDepthStencilImage(cb, image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1, &ds_range);
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    VkImageSubresourceLayers ds_layers_all{ds_aspect, 0, 0, 1};
    VkImageCopy ds_full_region = {ds_layers_all, zero_offset, ds_layers_all, zero_offset, full_extent};

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyImage(cb, image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, image_ds_b.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &ds_full_region);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image_ds_a.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &ds_range);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), image_ds_b.handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_value, 1,
                                  &ds_range);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkSyncValTest, SyncCmdQuery) {
    // CmdCopyQueryPoolResults
    m_errorMonitor->ExpectSuccess();
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    if ((m_device->queue_props.empty()) || (m_device->queue_props[0].queueCount < 2)) {
        printf("%s Queue family needs to have multiple queues to run this test.\n", kSkipPrefix);
        return;
    }
    uint32_t queue_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, NULL);
    VkQueueFamilyProperties *queue_props = new VkQueueFamilyProperties[queue_count];
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_count, queue_props);
    if (queue_props[m_device->graphics_queue_node_index_].timestampValidBits == 0) {
        printf("%s Device graphic queue has timestampValidBits of 0, skipping.\n", kSkipPrefix);
        return;
    }

    VkQueryPool query_pool;
    VkQueryPoolCreateInfo query_pool_create_info{};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = 1;
    vk::CreateQueryPool(m_device->device(), &query_pool_create_info, nullptr, &query_pool);

    VkBufferObj buffer_a, buffer_b;
    VkMemoryPropertyFlags mem_prop = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    buffer_a.init_as_src_and_dst(*m_device, 256, mem_prop);
    buffer_b.init_as_src_and_dst(*m_device, 256, mem_prop);

    VkBufferCopy region = {0, 0, 256};

    auto cb = m_commandBuffer->handle();
    m_commandBuffer->begin();
    vk::CmdResetQueryPool(cb, query_pool, 0, 1);
    vk::CmdWriteTimestamp(cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
    vk::CmdCopyQueryPoolResults(cb, query_pool, 0, 1, buffer_a.handle(), 0, 0, VK_QUERY_RESULT_WAIT_BIT);
    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    vk::CmdCopyBuffer(cb, buffer_a.handle(), buffer_b.handle(), 1, &region);
    vk::CmdResetQueryPool(cb, query_pool, 0, 1);
    vk::CmdWriteTimestamp(cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, query_pool, 0);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    vk::CmdCopyQueryPoolResults(cb, query_pool, 0, 1, buffer_a.handle(), 0, 256, VK_QUERY_RESULT_WAIT_BIT);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdCopyQueryPoolResults(cb, query_pool, 0, 1, buffer_b.handle(), 0, 256, VK_QUERY_RESULT_WAIT_BIT);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();

    // TODO:Track VkQueryPool
    // TODO:CmdWriteTimestamp
    vk::DestroyQueryPool(m_device->device(), query_pool, nullptr);
}

TEST_F(VkSyncValTest, SyncCmdDrawDepthStencil) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    m_errorMonitor->ExpectSuccess();

    const auto format_ds = FindSupportedDepthStencilFormat(gpu());
    if (!format_ds) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    const auto format_dp = FindSupportedDepthOnlyFormat(gpu());
    if (!format_dp) {
        printf("%s No only Depth format found. Skipped.\n", kSkipPrefix);
        return;
    }
    const auto format_st = FindSupportedStencilOnlyFormat(gpu());
    if (!format_st) {
        printf("%s No only Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    VkDepthStencilObj image_ds(m_device), image_dp(m_device), image_st(m_device);
    image_ds.Init(m_device, 16, 16, format_ds);
    image_dp.Init(m_device, 16, 16, format_dp);
    image_st.Init(m_device, 16, 16, format_st);

    VkRenderpassObj rp_ds(m_device, format_ds, true), rp_dp(m_device, format_dp, true), rp_st(m_device, format_st, true);

    VkFramebuffer fb_ds, fb_dp, fb_st;
    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp_ds.handle(), 1, image_ds.BindInfo(), 16, 16, 1};
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &fbci, nullptr, &fb_ds));
    fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp_dp.handle(), 1, image_dp.BindInfo(), 16, 16, 1};
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &fbci, nullptr, &fb_dp));
    fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp_st.handle(), 1, image_st.BindInfo(), 16, 16, 1};
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &fbci, nullptr, &fb_st));

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    auto ds_ci = lvl_init_struct<VkPipelineDepthStencilStateCreateInfo>();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    CreatePipelineHelper g_pipe_ds(*this), g_pipe_dp(*this), g_pipe_st(*this);
    g_pipe_ds.InitInfo();
    g_pipe_ds.gp_ci_.renderPass = rp_ds.handle();
    g_pipe_ds.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_ds.InitState();
    ASSERT_VK_SUCCESS(g_pipe_ds.CreateGraphicsPipeline());
    g_pipe_dp.InitInfo();
    g_pipe_dp.gp_ci_.renderPass = rp_dp.handle();
    ds_ci.stencilTestEnable = VK_FALSE;
    g_pipe_dp.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_dp.InitState();
    ASSERT_VK_SUCCESS(g_pipe_dp.CreateGraphicsPipeline());
    g_pipe_st.InitInfo();
    g_pipe_st.gp_ci_.renderPass = rp_st.handle();
    ds_ci.depthTestEnable = VK_FALSE;
    ds_ci.stencilTestEnable = VK_TRUE;
    g_pipe_st.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe_st.InitState();
    ASSERT_VK_SUCCESS(g_pipe_st.CreateGraphicsPipeline());

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {16, 16}};
    m_renderPassBeginInfo.pClearValues = nullptr;
    m_renderPassBeginInfo.clearValueCount = 0;

    m_renderPassBeginInfo.renderPass = rp_ds.handle();
    m_renderPassBeginInfo.framebuffer = fb_ds;
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_ds.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_dp.handle();
    m_renderPassBeginInfo.framebuffer = fb_dp;
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_dp.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_st.handle();
    m_renderPassBeginInfo.framebuffer = fb_st;
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_st.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
    m_errorMonitor->VerifyNotFound();

    m_commandBuffer->reset();
    m_commandBuffer->begin();

    VkImageCopy copyRegion;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent = {16, 16, 1};

    m_commandBuffer->CopyImage(image_ds.handle(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, image_dp.handle(),
                               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, &copyRegion);

    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    m_commandBuffer->CopyImage(image_ds.handle(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, image_st.handle(),
                               VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, &copyRegion);
    m_renderPassBeginInfo.renderPass = rp_ds.handle();
    m_renderPassBeginInfo.framebuffer = fb_ds;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_READ");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_ds.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_dp.handle();
    m_renderPassBeginInfo.framebuffer = fb_dp;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_dp.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.renderPass = rp_st.handle();
    m_renderPassBeginInfo.framebuffer = fb_st;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe_st.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();
    vk::DestroyFramebuffer(m_device->device(), fb_ds, nullptr);
    vk::DestroyFramebuffer(m_device->device(), fb_dp, nullptr);
    vk::DestroyFramebuffer(m_device->device(), fb_st, nullptr);
}

TEST_F(VkSyncValTest, SyncRenderPassWithWrongInitialLayout) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageUsageFlags usage_color = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageUsageFlags usage_input = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_color(m_device), image_input(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 1, format, usage_color, VK_IMAGE_TILING_OPTIMAL);
    image_color.Init(image_ci);
    image_ci.usage = usage_input;
    image_input.Init(image_ci);
    VkImageView attachments[] = {image_color.targetView(format), image_input.targetView(format)};

    const VkAttachmentDescription attachmentDescriptions[] = {
        // Result attachment
        {(VkAttachmentDescriptionFlags)0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED,  // Here causes DesiredError that SYNC-HAZARD-NONE in BeginRenderPass.
                                     // It should be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // Input attachment
        {(VkAttachmentDescriptionFlags)0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD,
         VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};

    const VkAttachmentReference resultAttachmentRef = {0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference inputAttachmentRef = {1u, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    const VkSubpassDescription subpassDescription = {(VkSubpassDescriptionFlags)0,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     1u,
                                                     &inputAttachmentRef,
                                                     1u,
                                                     &resultAttachmentRef,
                                                     0,
                                                     0,
                                                     0u,
                                                     0};

    const VkSubpassDependency subpassDependency = {VK_SUBPASS_EXTERNAL,
                                                   0,
                                                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                   VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
                                                   VK_DEPENDENCY_BY_REGION_BIT};

    const VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                   0,
                                                   (VkRenderPassCreateFlags)0,
                                                   2u,
                                                   attachmentDescriptions,
                                                   1u,
                                                   &subpassDescription,
                                                   1u,
                                                   &subpassDependency};
    VkRenderPass rp;
    ASSERT_VK_SUCCESS(vk::CreateRenderPass(device(), &renderPassInfo, nullptr, &rp));

    VkFramebuffer fb;
    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 2, attachments, 32, 32, 1};
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &fbci, nullptr, &fb));

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {32, 32}};
    m_renderPassBeginInfo.renderPass = rp;
    m_renderPassBeginInfo.framebuffer = fb;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkSyncValTest, SyncRenderPassWithWrongDepthStencilInitialLayout) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkFormat color_format = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (!ds_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }
    VkImageUsageFlags usage_color = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageUsageFlags usage_ds = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkImageObj image_color(m_device), image_color2(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(32, 32, 1, 1, color_format, usage_color, VK_IMAGE_TILING_OPTIMAL);
    image_color.Init(image_ci);
    image_color2.Init(image_ci);
    VkDepthStencilObj image_ds(m_device);
    image_ds.Init(m_device, 32, 32, ds_format, usage_ds);

    const VkAttachmentDescription colorAttachmentDescription = {(VkAttachmentDescriptionFlags)0,
                                                                color_format,
                                                                VK_SAMPLE_COUNT_1_BIT,
                                                                VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                VK_ATTACHMENT_STORE_OP_STORE,
                                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkAttachmentDescription depthStencilAttachmentDescription = {
        (VkAttachmentDescriptionFlags)0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_IMAGE_LAYOUT_UNDEFINED,  // Here causes DesiredError that SYNC-HAZARD-WRITE_AFTER_WRITE in BeginRenderPass.
                                    // It should be VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::vector<VkAttachmentDescription> attachmentDescriptions;
    attachmentDescriptions.push_back(colorAttachmentDescription);
    attachmentDescriptions.push_back(depthStencilAttachmentDescription);

    const VkAttachmentReference colorAttachmentRef = {0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkAttachmentReference depthStencilAttachmentRef = {1u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    const VkSubpassDescription subpassDescription = {(VkSubpassDescriptionFlags)0,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     0u,
                                                     0,
                                                     1u,
                                                     &colorAttachmentRef,
                                                     0,
                                                     &depthStencilAttachmentRef,
                                                     0u,
                                                     0};

    const VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                   0,
                                                   (VkRenderPassCreateFlags)0,
                                                   (uint32_t)attachmentDescriptions.size(),
                                                   &attachmentDescriptions[0],
                                                   1u,
                                                   &subpassDescription,
                                                   0u,
                                                   0};
    VkRenderPass rp;
    ASSERT_VK_SUCCESS(vk::CreateRenderPass(device(), &renderPassInfo, nullptr, &rp));

    VkImageView fb_attachments[] = {image_color.targetView(color_format),
                                    image_ds.targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)};
    const VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, 0, 0u, rp, 2u, fb_attachments, 32, 32, 1u,
    };
    VkFramebuffer fb;
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &fbci, nullptr, &fb));
    fb_attachments[0] = image_color2.targetView(color_format);
    VkFramebuffer fb1;
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &fbci, nullptr, &fb1));

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.gp_ci_.renderPass = rp;

    VkStencilOpState stencil = {};
    stencil.failOp = VK_STENCIL_OP_KEEP;
    stencil.passOp = VK_STENCIL_OP_KEEP;
    stencil.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil.compareOp = VK_COMPARE_OP_NEVER;

    auto ds_ci = lvl_init_struct<VkPipelineDepthStencilStateCreateInfo>();
    ds_ci.depthTestEnable = VK_TRUE;
    ds_ci.depthWriteEnable = VK_TRUE;
    ds_ci.depthCompareOp = VK_COMPARE_OP_NEVER;
    ds_ci.stencilTestEnable = VK_TRUE;
    ds_ci.front = stencil;
    ds_ci.back = stencil;

    g_pipe.gp_ci_.pDepthStencilState = &ds_ci;
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {32, 32}};
    m_renderPassBeginInfo.renderPass = rp;

    m_renderPassBeginInfo.framebuffer = fb;
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();

    m_renderPassBeginInfo.framebuffer = fb1;

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkSyncValTest, SyncLayoutTransition) {
    ASSERT_NO_FATAL_FAILURE(InitSyncValFramework());
    ASSERT_NO_FATAL_FAILURE(InitState());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageUsageFlags usage_color = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkImageUsageFlags usage_input =
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_color(m_device), image_input(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(64, 64, 1, 1, format, usage_input, VK_IMAGE_TILING_OPTIMAL);
    image_input.InitNoLayout(image_ci);
    image_ci.usage = usage_color;
    image_color.InitNoLayout(image_ci);
    VkImageView view_input = image_input.targetView(format);
    VkImageView view_color = image_color.targetView(format);
    VkImageView attachments[] = {view_color, view_input};

    const VkAttachmentDescription fbAttachment = {
        0u,
        format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    std::vector<VkAttachmentDescription> attachmentDescs;
    attachmentDescs.push_back(fbAttachment);

    // Add it as a frame buffer attachment.
    const VkAttachmentDescription inputAttachment = {
        0u,
        format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_LOAD,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
    };
    attachmentDescs.push_back(inputAttachment);

    std::vector<VkAttachmentReference> inputAttachments;
    const VkAttachmentReference inputRef = {
        1u,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    inputAttachments.push_back(inputRef);

    const VkAttachmentReference colorRef = {
        0u,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    const std::vector<VkAttachmentReference> colorAttachments(1u, colorRef);

    const VkSubpassDescription subpass = {
        0u,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        static_cast<uint32_t>(inputAttachments.size()),
        inputAttachments.data(),
        static_cast<uint32_t>(colorAttachments.size()),
        colorAttachments.data(),
        0u,
        nullptr,
        0u,
        nullptr,
    };
    const std::vector<VkSubpassDescription> subpasses(1u, subpass);

    const VkRenderPassCreateInfo renderPassInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0u,
        static_cast<uint32_t>(attachmentDescs.size()),
        attachmentDescs.data(),
        static_cast<uint32_t>(subpasses.size()),
        subpasses.data(),
        0u,
        nullptr,
    };
    VkRenderPass rp;
    ASSERT_VK_SUCCESS(vk::CreateRenderPass(device(), &renderPassInfo, nullptr, &rp));

    const VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, 0, 0u, rp, 2u, attachments, 64, 64, 1u,
    };
    VkFramebuffer fb;
    ASSERT_VK_SUCCESS(vk::CreateFramebuffer(device(), &fbci, nullptr, &fb));

    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);

    char const *fsSource =
        "#version 450\n"
        "layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;\n"
        "void main() {\n"
        "   vec4 color = subpassLoad(x);\n"
        "}\n";

    VkShaderObj vs(m_device, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT, this);
    VkShaderObj fs(m_device, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, this);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.gp_ci_.renderPass = rp;
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_input, sampler, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    auto cb = m_commandBuffer->handle();
    VkClearColorValue ccv = {};
    VkImageSubresourceRange full_subresource_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    const VkImageMemoryBarrier preClearBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, 0, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   0, 0, image_input.handle(),         full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u,
                           &preClearBarrier);

    vk::CmdClearColorImage(m_commandBuffer->handle(), image_input.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &ccv, 1,
                           &full_subresource_range);

    const VkImageMemoryBarrier postClearBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        0,
        0,
        image_input.handle(),
        full_subresource_range,
    };
    vk::CmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr,
                           1u, &postClearBarrier);

    m_renderPassBeginInfo.renderArea = {{0, 0}, {64, 64}};
    m_renderPassBeginInfo.renderPass = rp;
    m_renderPassBeginInfo.framebuffer = fb;

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    // Positive test for ordering rules between load and input attachment usage
    m_errorMonitor->ExpectSuccess();
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

    // Positive test for store ordering vs. input attachment and dependency *to* external for layout transition
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyNotFound();

    // Catch a conflict with the input attachment final layout transition
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "SYNC-HAZARD-WRITE_AFTER_WRITE");
    vk::CmdClearColorImage(m_commandBuffer->handle(), image_input.handle(), VK_IMAGE_LAYOUT_GENERAL, &ccv, 1,
                           &full_subresource_range);
    m_errorMonitor->VerifyFound();
}
