/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
 * Author: Tobias Hector <tobias.hector@amd.com>
 */

#include <type_traits>

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, BufferExtents) {
    TEST_DESCRIPTION("Perform copies across a buffer, provoking out-of-range errors.");

    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool copy_commands2 = CanEnableDeviceExtension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdCopyBuffer2KHR vkCmdCopyBuffer2Function = nullptr;
    if (copy_commands2) {
        vkCmdCopyBuffer2Function = (PFN_vkCmdCopyBuffer2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyBuffer2KHR");
    }

    const VkDeviceSize buffer_size = 2048;

    VkBufferObj buffer_one;
    VkBufferObj buffer_two;
    VkMemoryPropertyFlags reqs = 0;
    buffer_one.init_as_src_and_dst(*m_device, buffer_size, reqs);
    buffer_two.init_as_src_and_dst(*m_device, buffer_size, reqs);

    VkBufferCopy copy_info = {4096, 256, 256};

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-srcOffset-00113");
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyBuffer2Function) {
        const VkBufferCopy2KHR copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                             copy_info.size};
        const VkCopyBufferInfo2KHR copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferInfo2-srcOffset-00113");
        vkCmdCopyBuffer2Function(m_commandBuffer->handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-dstOffset-00114");
    copy_info = {256, 4096, 256};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyBuffer2Function) {
        const VkBufferCopy2KHR copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                             copy_info.size};
        const VkCopyBufferInfo2KHR copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferInfo2-dstOffset-00114");
        vkCmdCopyBuffer2Function(m_commandBuffer->handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-size-00115");
    copy_info = {1024, 256, 1280};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyBuffer2Function) {
        const VkBufferCopy2KHR copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                             copy_info.size};
        const VkCopyBufferInfo2KHR copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferInfo2-size-00115");
        vkCmdCopyBuffer2Function(m_commandBuffer->handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-size-00116");
    copy_info = {256, 1024, 1280};
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer_one.handle(), buffer_two.handle(), 1, &copy_info);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyBuffer2Function) {
        const VkBufferCopy2KHR copy_info2 = {VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR, NULL, copy_info.srcOffset, copy_info.dstOffset,
                                             copy_info.size};
        const VkCopyBufferInfo2KHR copy_buffer_info2 = {
            VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR, NULL, buffer_one.handle(), buffer_two.handle(), 1, &copy_info2};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferInfo2-size-00116");
        vkCmdCopyBuffer2Function(m_commandBuffer->handle(), &copy_buffer_info2);
        m_errorMonitor->VerifyFound();
    }

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

    SetTargetApiVersion(VK_API_VERSION_1_0);
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

TEST_F(VkLayerTest, MirrorClampToEdgeNotEnabled12) {
    TEST_DESCRIPTION("Validation using CLAMP_TO_EDGE for Vulkan 1.2 without the samplerMirrorClampToEdge feature enabled.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s Tests requires Vulkan 1.2+, skipping test\n", kSkipPrefix);
        return;
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-addressModeU-01079");
    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

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
    AddRequiredExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    // These tests require that the device support anisotropic filtering
    if (VK_TRUE != device_features.samplerAnisotropy) {
        printf("%s Test requires unsupported samplerAnisotropy feature. Skipped.\n", kSkipPrefix);
        return;
    }

    const bool cubic_support = CanEnableDeviceExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);

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

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkBindSparseInfo bind_info = LvlInitStruct<VkBindSparseInfo>();
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

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buf_info.size = 256;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);
    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    alloc_info.memoryTypeIndex = 0;

    // Ensure memory is big enough for both bindings
    // Want to make sure entire allocation is aligned to atom size
    static const VkDeviceSize allocation_size = atom_size * 64;
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
    m_errorMonitor->SetUnexpectedError("VUID-vkMapMemory-size-00681");
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
    VkMappedMemoryRange mmr = LvlInitStruct<VkMappedMemoryRange>();
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

        // Now with VK_WHOLE_SIZE and a mapping that does not end at a multiple of atom_size nor at the end of the memory.
        vk::UnmapMemory(m_device->device(), mem);
        err = vk::MapMemory(m_device->device(), mem, 0, 4 * atom_size + 1, 0, (void **)&pData);
        ASSERT_VK_SUCCESS(err);
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
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        vk::DestroyBuffer(m_device->device(), buffer, NULL);
        return;
    }
    ASSERT_VK_SUCCESS(vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &mem));
    ASSERT_VK_SUCCESS(vk::MapMemory(m_device->device(), mem, 0, VK_WHOLE_SIZE, 0, (void **)&pData));
    // Some platforms have an atomsize of 1 which makes the test meaningless
    if (atom_size > 1) {
        // Offset is atom size, but total memory range is not atom size
        mmr.memory = mem;
        mmr.offset = atom_size;
        mmr.size = VK_WHOLE_SIZE;
        m_errorMonitor->ExpectSuccess();
        vk::FlushMappedMemoryRanges(m_device->device(), 1, &mmr);
        m_errorMonitor->VerifyNotFound();
    }

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

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
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

TEST_F(VkLayerTest, InvalidSparseImageUsageBits) {
    TEST_DESCRIPTION("Try to use VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT with sparse image");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    if (!device_features.sparseBinding) {
        printf("%s No sparseBinding feature. Skipped.\n", kSkipPrefix);
        return;
    }

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-None-01925");
    VkImage img = image.image();
    vk::CreateImage(m_device->device(), &image_create_info, nullptr, &img);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidUsageBits) {
    TEST_DESCRIPTION(
        "Specify wrong usage for image then create conflicting view of image Initialize buffer with wrong usage then perform copy "
        "expecting errors from both the image and the buffer (2 calls)");

    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool copy_commands2 = CanEnableDeviceExtension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdCopyBufferToImage2KHR vkCmdCopyBufferToImage2Function = nullptr;
    if (copy_commands2) {
        vkCmdCopyBufferToImage2Function =
            (PFN_vkCmdCopyBufferToImage2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyBufferToImage2KHR");
    }

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
    VkImageViewCreateInfo dsvci = LvlInitStruct<VkImageViewCreateInfo>();
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

    // equvalent test using using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyBufferToImage2Function) {
        const VkBufferImageCopy2KHR region2 = {VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2_KHR,
                                               NULL,
                                               region.bufferRowLength,
                                               region.bufferImageHeight,
                                               region.bufferImageHeight,
                                               region.imageSubresource,
                                               region.imageOffset,
                                               region.imageExtent};
        VkCopyBufferToImageInfo2KHR buffer_to_image_info2 = {VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2_KHR,
                                                             NULL,
                                                             buffer.handle(),
                                                             image.handle(),
                                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                             1,
                                                             &region2};
        // two separate errors from this call:
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-dstImage-00177");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyBufferToImageInfo2-srcBuffer-00174");
        vkCmdCopyBufferToImage2Function(m_commandBuffer->handle(), &buffer_to_image_info2);
        m_errorMonitor->VerifyFound();
    }
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-pRegions-06218");
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), width_image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1, &region);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBufferToImage-imageOffset-00200");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdCopyBufferToImage-pRegions-06217");

    VkResult err;
    VkImageCreateInfo depth_image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
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
        printf("%s No invalid memory type index could be found; skipped.\n", kSkipPrefix);
        vk::DestroyImage(m_device->device(), image, nullptr);
        return;
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

    ASSERT_NO_FATAL_FAILURE(Init());

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
                printf("%s Sparse image format not supported; skipped.\n", kSkipPrefix);
                return;
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

TEST_F(VkLayerTest, BindInvalidMemoryYcbcr) {
    // Enable KHR YCbCr req'd extensions for Disjoint Bit
    if (!AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
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
    constexpr VkFormatFeatureFlags disjoint_sampled = VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    if (disjoint_sampled != (format_properties.optimalTilingFeatures & disjoint_sampled)) {
        printf("%s test requires disjoint and sampled feature bit on format.  Skipping.\n", kSkipPrefix);
    } else {
        VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

        VkImagePlaneMemoryRequirementsInfo image_plane_req = LvlInitStruct<VkImagePlaneMemoryRequirementsInfo>();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

        // Find a valid memory type index to memory to be allocated from
        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = mem_req2.memoryRequirements.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));

        VkDeviceMemory image_memory;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &image_memory));

        // Bind disjoint with BindImageMemory instead of BindImageMemory2
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-image-01608");
        vk::BindImageMemory(device(), image, image_memory, 0);
        m_errorMonitor->VerifyFound();

        VkBindImagePlaneMemoryInfo plane_memory_info = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
        ASSERT_TRUE(FormatPlaneCount(mp_format) == 2);
        plane_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;

        VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info);
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
        VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

        VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>();
        mem_req_info2.image = image;
        VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

        // Find a valid memory type index to memory to be allocated from
        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
        alloc_info.allocationSize = mem_req2.memoryRequirements.size;
        ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));

        VkDeviceMemory image_memory;
        ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &image_memory));

        VkBindImagePlaneMemoryInfo plane_memory_info = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
        plane_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
        VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info);
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
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool mp_extensions = CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const bool bind_memory_2_extension = CanEnableDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

    if (!bind_memory_2_extension) {
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
        if ((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
            (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
            mp_disjoint_support = true;
        }
    }

    // Try to bind memory to an object with an invalid memoryOffset

    VkImage image = VK_NULL_HANDLE;
    ASSERT_VK_SUCCESS(vk::CreateImage(device(), &image_create_info, NULL, &image));
    VkMemoryRequirements image_mem_reqs = {};
    vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
    VkMemoryAllocateInfo image_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
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
    plane_memory_info[0] = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
    plane_memory_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    plane_memory_info[1] = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
    plane_memory_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

    // Test unaligned memory offset

    // single-plane image
    if (bind_memory_2_extension == true) {
        VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
        bind_image_info.image = image;
        bind_image_info.memory = image_mem;
        bind_image_info.memoryOffset = 1;  // off alignment

        if (mp_disjoint_support == true) {
            VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>();
            mem_req_info2.image = image;
            VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();
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
            bind_image_info[0] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[0]);
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = 1;  // off alignment
            bind_image_info[1] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[1]);
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
        VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
        bind_image_info.image = image;
        bind_image_info.memory = image_mem;

        if (mp_disjoint_support == true) {
            VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>();
            mem_req_info2.image = image;
            VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();
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
            bind_image_info[0] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[0]);
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = mp_image_offset;  // mis-offset
            bind_image_info[1] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[1]);
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
        VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
        bind_image_info.image = image;
        bind_image_info.memoryOffset = 0;

        if (mp_disjoint_support == true) {
            VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>();
            mem_req_info2.image = image;
            VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();
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
        VkImagePlaneMemoryRequirementsInfo image_plane_req = LvlInitStruct<VkImagePlaneMemoryRequirementsInfo>();
        image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

        VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
        mem_req_info2.image = mp_image;
        vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mp_image_mem_reqs2[0]);

        uint32_t mp_image_unsupported_mem_type_bits =
            ((1 << memory_properties.memoryTypeCount) - 1) & ~mp_image_mem_reqs2[0].memoryRequirements.memoryTypeBits;
        if (mp_image_unsupported_mem_type_bits != 0) {
            mp_image_alloc_info[0].allocationSize = mp_image_mem_reqs2[0].memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mp_image_unsupported_mem_type_bits, &mp_image_alloc_info[0], 0));
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &mp_image_alloc_info[0], NULL, &mp_image_mem[0]));

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[0]);
            bind_image_info[0].image = mp_image;
            bind_image_info[0].memory = mp_image_mem[0];
            bind_image_info[0].memoryOffset = 0;
            bind_image_info[1] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_memory_info[1]);
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
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool mp_extensions = CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

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
        printf("%s Rest of test rely on YCbCr Multi-planar support.\n", kSkipPrefix);
        return;
    } else {
        // Check for support of format used by all multi-planar tests
        const VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        VkFormatProperties mp_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &mp_format_properties);
        if (!((mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
              (mp_format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            printf("%s Rest of test rely on a supported disjoint format.\n", kSkipPrefix);
            return;
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
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool mp_extensions = CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const bool bind_memory_2_extension = CanEnableDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

    if (!bind_memory_2_extension) {
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
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateSampler-maxSamplerAllocationCount-04110");

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
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool copy_commands2 = CanEnableDeviceExtension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdBlitImage2KHR vkCmdBlitImage2Function = nullptr;
    if (copy_commands2) {
        vkCmdBlitImage2Function = (PFN_vkCmdBlitImage2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBlitImage2KHR");
    }

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

    // equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdBlitImage2Function) {
        const VkImageBlit2KHR blitRegion2 = {
            VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR, NULL,
            blitRegion.srcSubresource,          {blitRegion.srcOffsets[0], blitRegion.srcOffsets[1]},
            blitRegion.dstSubresource,          {blitRegion.dstOffsets[0], blitRegion.dstOffsets[1]}};
        const VkBlitImageInfo2KHR blit_image_info2 = {VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      unsigned_image.image(),
                                                      unsigned_image.Layout(),
                                                      float_image.image(),
                                                      float_image.Layout(),
                                                      1,
                                                      &blitRegion2,
                                                      VK_FILTER_NEAREST};
        // Unsigned int vs not an int
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBlitImageInfo2-srcImage-00230");
        if (usrc) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-srcImage-01999");
        if (fdst) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-dstImage-02000");
        vkCmdBlitImage2Function(m_commandBuffer->handle(), &blit_image_info2);
        m_errorMonitor->VerifyFound();
    }

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00230");
    if (fsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (udst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), float_image.image(), float_image.Layout(), unsigned_image.image(),
                     unsigned_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdBlitImage2Function) {
        const VkImageBlit2KHR blitRegion2 = {
            VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR, NULL,
            blitRegion.srcSubresource,          {blitRegion.srcOffsets[0], blitRegion.srcOffsets[1]},
            blitRegion.dstSubresource,          {blitRegion.dstOffsets[0], blitRegion.dstOffsets[1]}};
        const VkBlitImageInfo2KHR blit_image_info2 = {VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      float_image.image(),
                                                      float_image.Layout(),
                                                      unsigned_image.image(),
                                                      unsigned_image.Layout(),
                                                      1,
                                                      &blitRegion2,
                                                      VK_FILTER_NEAREST};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBlitImageInfo2-srcImage-00230");
        if (fsrc) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-srcImage-01999");
        if (udst) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-dstImage-02000");
        vkCmdBlitImage2Function(m_commandBuffer->handle(), &blit_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Signed int vs not an int,
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-00229");
    if (ssrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
    if (fdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
    vk::CmdBlitImage(m_commandBuffer->handle(), signed_image.image(), signed_image.Layout(), float_image.image(),
                     float_image.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
    m_errorMonitor->VerifyFound();

    // equivalent test using KHR_copy_commands2
    if (copy_commands2 && vkCmdBlitImage2Function) {
        const VkImageBlit2KHR blitRegion2 = {
            VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR, NULL,
            blitRegion.srcSubresource,          {blitRegion.srcOffsets[0], blitRegion.srcOffsets[1]},
            blitRegion.dstSubresource,          {blitRegion.dstOffsets[0], blitRegion.dstOffsets[1]}};
        const VkBlitImageInfo2KHR blit_image_info2 = {VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2_KHR,
                                                      NULL,
                                                      signed_image.image(),
                                                      signed_image.Layout(),
                                                      float_image.image(),
                                                      float_image.Layout(),
                                                      1,
                                                      &blitRegion2,
                                                      VK_FILTER_NEAREST};
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBlitImageInfo2-srcImage-00229");
        if (ssrc) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-srcImage-01999");
        if (fdst) m_errorMonitor->SetUnexpectedError("VUID-VkBlitImageInfo2-dstImage-02000");
        vkCmdBlitImage2Function(m_commandBuffer->handle(), &blit_image_info2);
        m_errorMonitor->VerifyFound();
    }

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
        ycbcr_image.Init(64, 64, 1, f_ycbcr, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ycbcr_image.initialized());
        ycbcr_image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

        VkImageObj ycbcr_image_2(m_device);
        ycbcr_image_2.Init(64, 64, 1, f_ycbcr, VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(ycbcr_image_2.initialized());
        ycbcr_image_2.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

        // Src, dst is ycbcr format
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-srcImage-06421");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-dstImage-06422");
        if (ycbcrsrc) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-srcImage-01999");
        if (ycbcrdst) m_errorMonitor->SetUnexpectedError("VUID-vkCmdBlitImage-dstImage-02000");
        vk::CmdBlitImage(m_commandBuffer->handle(), ycbcr_image.image(), ycbcr_image.Layout(), ycbcr_image_2.image(),
                         ycbcr_image_2.Layout(), 1, &blitRegion, VK_FILTER_NEAREST);
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
    AddRequiredExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool cubic_support = CanEnableDeviceExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
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
    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = fmt;
    ci.extent = {64, 64, 4};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

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
        vk::CmdBlitImage(m_commandBuffer->handle(), src2D.image(), src2D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
                         VK_FILTER_CUBIC_IMG);
        m_errorMonitor->VerifyFound();

        // Invalid filter CUBIC_IMG + invalid 2D source image
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-02002");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-filter-00237");
        vk::CmdBlitImage(m_commandBuffer->handle(), src3D.image(), src3D.Layout(), dst2D.image(), dst2D.Layout(), 1, &blitRegion,
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

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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

    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
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

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
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

TEST_F(VkLayerTest, BlitImageOverlap) {
    TEST_DESCRIPTION("Try to blit an image on same region.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkFormat fmt = VK_FORMAT_R8G8B8A8_UNORM;
    if (!ImageFormatAndFeaturesSupported(gpu(), fmt, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        printf("%s No blit feature bits - BlitImageOverlap skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = fmt;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.queueFamilyIndexCount = 0;
    ci.pQueueFamilyIndices = nullptr;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageObj image_2D(m_device);
    image_2D.init(&ci);
    ASSERT_TRUE(image_2D.initialized());

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

    blit_region.srcOffsets[0] = {0, 0, 0};
    blit_region.srcOffsets[1] = {31, 31, 1};
    blit_region.dstOffsets[0] = {15, 15, 0};
    blit_region.dstOffsets[1] = {47, 47, 1};
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBlitImage-pRegions-00217");
    vk::CmdBlitImage(m_commandBuffer->handle(), image_2D.image(), image_2D.Layout(), image_2D.image(), image_2D.Layout(), 1,
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

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
    VkMemoryBarrier mem_barrier = LvlInitStruct<VkMemoryBarrier>();
    mem_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1,
                           &mem_barrier, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_commandBuffer->handle());

    rpbi.renderPass = rp;
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
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
    mem_barrier = LvlInitStruct<VkMemoryBarrier>();
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
    VkBufferMemoryBarrier bmb = LvlInitStruct<VkBufferMemoryBarrier>();
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

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
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

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    vk::FreeMemory(m_device->handle(), mem, NULL);
}

TEST_F(VkLayerTest, InvalidCmdBarrierBufferDestroyed) {
    ASSERT_NO_FATAL_FAILURE(Init());

    VkBuffer buffer;
    VkDeviceMemory buffer_mem;
    VkMemoryRequirements mem_reqs;

    auto buf_info = lvl_init_struct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    auto alloc_info = lvl_init_struct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;

    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &buffer_mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    auto buf_barrier = lvl_init_struct<VkBufferMemoryBarrier>();
    buf_barrier.buffer = buffer;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                           0, 0, NULL, 1, &buf_barrier, 0, NULL);
    m_commandBuffer->end();

    auto submit_info = lvl_init_struct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeMemory-memory-00677");
    vk::FreeMemory(m_device->handle(), buffer_mem, NULL);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyBuffer(m_device->handle(), buffer, NULL);
}

TEST_F(VkLayerTest, InvalidCmdBarrierImageDestroyed) {
    ASSERT_NO_FATAL_FAILURE(Init());

    vk_testing::Image image;
    vk_testing::DeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;

    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);

    image.init_no_mem(*m_device, image_ci);

    vk::GetImageMemoryRequirements(device(), image.handle(), &mem_reqs);

    auto alloc_info = lvl_init_struct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    image_mem.init(*m_device, alloc_info);

    auto err = vk::BindImageMemory(m_device->device(), image.handle(), image_mem.handle(), 0);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    auto img_barrier = lvl_init_struct<VkImageMemoryBarrier>();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, &img_barrier);
    m_commandBuffer->end();

    auto submit_info = lvl_init_struct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeMemory-memory-00677");
    vk::FreeMemory(m_device->handle(), image_mem.handle(), NULL);
    m_errorMonitor->VerifyFound();

    vk::QueueWaitIdle(m_device->m_queue);
}

TEST_F(VkLayerTest, Sync2InvalidCmdBarrierBufferDestroyed) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }
    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    VkBuffer buffer;
    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;

    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.size = 256;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult err = vk::CreateBuffer(m_device->device(), &buf_info, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    vk::GetBufferMemoryRequirements(m_device->device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    m_commandBuffer->begin();
    auto buf_barrier = lvl_init_struct<VkBufferMemoryBarrier2KHR>();
    buf_barrier.buffer = buffer;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    auto dep_info = lvl_init_struct<VkDependencyInfoKHR>();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &buf_barrier;

    m_commandBuffer->PipelineBarrier2KHR(&dep_info);

    vk::FreeMemory(m_device->handle(), mem, NULL);

    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    auto submit_info = lvl_init_struct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyBuffer(m_device->handle(), buffer, NULL);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Sync2InvalidCmdBarrierImageDestroyed) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }
    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    VkImage image;
    VkDeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;

    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                                  VK_IMAGE_TILING_OPTIMAL);

    auto err = vk::CreateImage(device(), &image_ci, nullptr, &image);
    ASSERT_VK_SUCCESS(err);

    vk::GetImageMemoryRequirements(device(), image, &mem_reqs);

    auto alloc_info = lvl_init_struct<VkMemoryAllocateInfo>();
    alloc_info.allocationSize = mem_reqs.size;
    bool pass = false;
    pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &alloc_info, 0);
    ASSERT_TRUE(pass);

    err = vk::AllocateMemory(m_device->device(), &alloc_info, NULL, &image_mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindImageMemory(m_device->device(), image, image_mem, 0);
    ASSERT_VK_SUCCESS(err);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    m_commandBuffer->begin();
    auto img_barrier = lvl_init_struct<VkImageMemoryBarrier2KHR>();
    img_barrier.image = image;
    img_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    auto dep_info = lvl_init_struct<VkDependencyInfoKHR>();
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &img_barrier;

    m_commandBuffer->PipelineBarrier2KHR(&dep_info);

    vk::FreeMemory(m_device->handle(), image_mem, NULL);

    vk::EndCommandBuffer(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDeviceMemory");
    auto submit_info = lvl_init_struct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyImage(m_device->handle(), image, NULL);
    m_errorMonitor->VerifyFound();
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
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>();
    VkBufferView view;

    {
        uint32_t queue_family_index = 0;
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;
        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        bvci.buffer = buffer.handle();
        bvci.format = VK_FORMAT_R32_SFLOAT;
        bvci.range = VK_WHOLE_SIZE;

        VkResult err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
        ASSERT_VK_SUCCESS(err);

        descriptor_set.WriteDescriptorBufferView(0, view);
        descriptor_set.UpdateDescriptorSets();

        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, r32f) uniform readonly imageBuffer s;
            layout(location=0) out vec4 x;
            void main(){
               x = imageLoad(s, 0);
            }
        )glsl";
        VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

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
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
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
        VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
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
        VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
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
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
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
        VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
        buffer_create_info.size = 1024;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        buffer_create_info.queueFamilyIndexCount = 1;
        buffer_create_info.pQueueFamilyIndices = &queue_family_index;
        VkBufferObj buffer;
        buffer.init(*m_device, buffer_create_info);

        VkBufferViewCreateInfo bvci = LvlInitStruct<VkBufferViewCreateInfo>();
        bvci.buffer = buffer.handle();
        bvci.format = VK_FORMAT_R32_SFLOAT;
        bvci.range = VK_WHOLE_SIZE;

        err = vk::CreateBufferView(m_device->device(), &bvci, NULL, &view);
        ASSERT_VK_SUCCESS(err);
    }
    // First Destroy buffer underlying view which should hit error in CV

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
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
    VkBufferCreateInfo buff_ci = LvlInitStruct<VkBufferCreateInfo>();
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buff_ci.size = 256;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBuffer buffer;
    err = vk::CreateBuffer(m_device->device(), &buff_ci, NULL, &buffer);
    ASSERT_VK_SUCCESS(err);

    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
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
    AddRequiredExtensions(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool texel_buffer_alignment = CanEnableDeviceExtension(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME);
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
    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
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
    auto texel_buffer_alignment_features = LvlInitStruct<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&texel_buffer_alignment_features);
    m_device_extension_names.push_back(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }
    texel_buffer_alignment_features.texelBufferAlignment = VK_TRUE;

    VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT align_props =
        LvlInitStruct<VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&align_props);
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
    VkBufferViewCreateInfo buff_view_ci = LvlInitStruct<VkBufferViewCreateInfo>();
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
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageObj mp_image(m_device);
    VkFormat mp_format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool separate_stencil_usage_supported = CanEnableDeviceExtension(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
        VkImageStencilUsageCreateInfoEXT image_stencil_create_info = LvlInitStruct<VkImageStencilUsageCreateInfoEXT>();
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

TEST_F(VkLayerTest, ClearDepthRangeUnrestricted) {
    TEST_DESCRIPTION("Test clearing without VK_EXT_depth_range_unrestricted");

    // Extension doesn't have feature bit, so not enabling extension invokes restrictions
    ASSERT_NO_FATAL_FAILURE(Init());

    // Need to set format framework uses for InitRenderTarget
    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    if (m_depth_stencil_fmt == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    int depth_attachment_index = 1;
    m_depthStencil->Init(m_device, static_cast<int32_t>(m_width), static_cast<int32_t>(m_height), m_depth_stencil_fmt,
                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearDepthStencilValue-depth-02506");
    const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
    const VkClearDepthStencilValue bad_clear_value = {1.5f, 0};
    vk::CmdClearDepthStencilImage(m_commandBuffer->handle(), m_depthStencil->handle(), VK_IMAGE_LAYOUT_GENERAL, &bad_clear_value, 1,
                                  &range);
    m_errorMonitor->VerifyFound();

    m_renderPassClearValues[depth_attachment_index].depthStencil.depth = 1.5f;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearDepthStencilValue-depth-02506");
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_errorMonitor->VerifyFound();

    // set back to normal
    m_renderPassClearValues[depth_attachment_index].depthStencil.depth = 1.0f;
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkClearDepthStencilValue-depth-02506");
    VkClearAttachment clear_attachment;
    clear_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    clear_attachment.clearValue.depthStencil.depth = 1.5f;
    clear_attachment.clearValue.depthStencil.stencil = 0;
    VkClearRect clear_rect = {{{0, 0}, {32, 32}}, 0, 1};
    vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &clear_attachment, 1, &clear_rect);
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

    VkBufferMemoryBarrier buf_barrier = LvlInitStruct<VkBufferMemoryBarrier>();
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
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    const bool mp_extensions = CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const bool separate_ds_layouts = CanEnableDeviceExtension(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    const bool maintenance2 = CanEnableDeviceExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

    // Check for external memory device extensions
    const bool external_memory = CanEnableDeviceExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);

    // Set separate depth stencil feature bit
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    auto separate_depth_stencil_layouts_features = LvlInitStruct<VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&separate_depth_stencil_layouts_features);
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
    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
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
        conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");

        // No bits other than DEPTH may be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
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
        // must have the VK_IMAGE_ASPECT_STENCIL_BIT set
        conc_test("UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
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
        constexpr VkImageAspectFlags disjoint_sampled = VK_FORMAT_FEATURE_DISJOINT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        if (disjoint_sampled == (format_properties.optimalTilingFeatures & disjoint_sampled)) {
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

            VkImage mp_image;
            VkDeviceMemory plane_0_memory;
            VkDeviceMemory plane_1_memory;
            ASSERT_VK_SUCCESS(vk::CreateImage(m_device->device(), &image_create_info, NULL, &mp_image));

            VkImagePlaneMemoryRequirementsInfo image_plane_req = LvlInitStruct<VkImagePlaneMemoryRequirementsInfo>();
            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;

            VkImageMemoryRequirementsInfo2 mem_req_info2 = LvlInitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
            mem_req_info2.image = mp_image;
            VkMemoryRequirements2 mem_req2 = LvlInitStruct<VkMemoryRequirements2>();
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);

            // Find a valid memory type index to memory to be allocated from
            VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &plane_0_memory));

            image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
            vkGetImageMemoryRequirements2Function(device(), &mem_req_info2, &mem_req2);
            alloc_info.allocationSize = mem_req2.memoryRequirements.size;
            ASSERT_TRUE(m_device->phy().set_memory_type(mem_req2.memoryRequirements.memoryTypeBits, &alloc_info, 0));
            ASSERT_VK_SUCCESS(vk::AllocateMemory(device(), &alloc_info, NULL, &plane_1_memory));

            VkBindImagePlaneMemoryInfo plane_0_memory_info = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
            plane_0_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
            VkBindImagePlaneMemoryInfo plane_1_memory_info = LvlInitStruct<VkBindImagePlaneMemoryInfo>();
            plane_1_memory_info.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;

            VkBindImageMemoryInfo bind_image_info[2];
            bind_image_info[0] = LvlInitStruct<VkBindImageMemoryInfo>(&plane_0_memory_info);
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
            // Skip layouts that require maintenance2 support
            if ((maintenance2 == false) && ((bad_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
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
    // Have bit that's supported (transfer write), and another that isn't to verify multi-bit validation
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
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
    uint32_t queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, false);
    if (queue_family_index == UINT32_MAX) {
        printf("%s No non-graphics queue supporting compute found; skipped.\n", kSkipPrefix);
        return;  // NOTE: this exits the test function!
    }

    VkBufferMemoryBarrier buf_barrier = LvlInitStruct<VkBufferMemoryBarrier>();
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    buf_barrier.buffer = buffer.handle();
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-srcStageMask-06461");

    VkCommandPoolObj command_pool(m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj bad_command_buffer(m_device, &command_pool);

    bad_command_buffer.begin();
    // Set two bits that should both be supported as a bonus positive check
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    vk::CmdPipelineBarrier(bad_command_buffer.handle(), VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &buf_barrier, 0, nullptr);
    m_errorMonitor->VerifyFound();

    // Check for error for trying to wait on pipeline stage not supported by this queue. Specifically since our queue is not a
    // compute queue, vk::CmdWaitEvents cannot have it's source stage mask be VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdWaitEvents-srcStageMask-06459");
    VkEvent event;
    VkEventCreateInfo event_create_info = LvlInitStruct<VkEventCreateInfo>();
    vk::CreateEvent(m_device->device(), &event_create_info, nullptr, &event);
    vk::CmdWaitEvents(bad_command_buffer.handle(), 1, &event, /*source stage mask*/ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                      VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
    m_errorMonitor->VerifyFound();
    bad_command_buffer.end();

    vk::DestroyEvent(m_device->device(), event, nullptr);
}

TEST_F(VkLayerTest, Sync2InvalidBarriers) {
    TEST_DESCRIPTION("Synchronization2 test for invalid Memory Barriers");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!CanEnableDeviceExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }
    const bool separate_ds_layouts = CanEnableDeviceExtension(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    const bool maintenance2 = CanEnableDeviceExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

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
    Barrier2QueueFamilyTestHelper::Context test_context(this, qf_indices);

    // Use image unbound to memory in barrier
    // Use buffer unbound to memory in barrier
    Barrier2QueueFamilyTestHelper conc_test(&test_context);
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

    // New layout can't be PREINITIALIZED
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    conc_test("VUID-VkImageMemoryBarrier2-newLayout-01198", "");

    // Transition image to color attachment optimal
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test("");

    // TODO: this looks vestigal or incomplete...
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Can't send buffer memory barrier during a render pass
    vk::CmdEndRenderPass(m_commandBuffer->handle());

    // Duplicate barriers that change layout
    auto img_barrier = lvl_init_struct<VkImageMemoryBarrier2KHR>();
    img_barrier.image = image.handle();
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    img_barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    VkImageMemoryBarrier2KHR img_barriers[2] = {img_barrier, img_barrier};

    auto dep_info = lvl_init_struct<VkDependencyInfoKHR>();
    dep_info.imageMemoryBarrierCount = 2;
    dep_info.pImageMemoryBarriers = img_barriers;
    dep_info.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Transitions from UNDEFINED  are valid, even if duplicated
    m_errorMonitor->ExpectSuccess();
    m_commandBuffer->PipelineBarrier2KHR(&dep_info);
    m_errorMonitor->VerifyNotFound();

    // Duplication of layout transitions (not from undefined) are not valid
    img_barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    img_barriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barriers[1].oldLayout = img_barriers[0].oldLayout;
    img_barriers[1].newLayout = img_barriers[0].newLayout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageMemoryBarrier2-oldLayout-01197");
    m_commandBuffer->PipelineBarrier2KHR(&dep_info);
    m_errorMonitor->VerifyFound();

    {
        // Transitions to and from EXTERNAL within the same command buffer are valid, if pointless.
        m_errorMonitor->ExpectSuccess();
        dep_info.imageMemoryBarrierCount = 1;
        dep_info.pImageMemoryBarriers = &img_barrier;
        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.srcQueueFamilyIndex = submit_family;
        img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        img_barrier.dstAccessMask = 0;
        img_barrier.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        img_barrier.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        m_commandBuffer->PipelineBarrier2KHR(&dep_info);

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL;
        img_barrier.dstQueueFamilyIndex = submit_family;
        img_barrier.srcAccessMask = 0;
        img_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

        m_commandBuffer->PipelineBarrier2KHR(&dep_info);
        m_errorMonitor->VerifyNotFound();
    }

    // Exceed the buffer size
    conc_test.buffer_barrier_.offset = conc_test.buffer_.create_info().size + 1;
    conc_test("", "VUID-VkBufferMemoryBarrier2-offset-01187");

    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = conc_test.buffer_.create_info().size + 1;
    // Size greater than total size
    conc_test("", "VUID-VkBufferMemoryBarrier2-size-01189");

    conc_test.buffer_barrier_.size = 0;
    // Size is zero
    conc_test("", "VUID-VkBufferMemoryBarrier2-size-01188");

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
    {
        const char *vuid = (separate_ds_layouts == true) ? "VUID-VkImageMemoryBarrier2-image-03320"
                                                         : "VUID-VkImageMemoryBarrier2-image-01207";
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
        conc_test("depth-only image formats must have the VK_IMAGE_ASPECT_DEPTH_BIT set.");

        // No bits other than DEPTH may be set
        conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_COLOR_BIT;
        conc_test("depth-only image formats can have only the VK_IMAGE_ASPECT_DEPTH_BIT set.");
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
        conc_test("stencil-only image formats must have the VK_IMAGE_ASPECT_STENCIL_BIT set.");
    }

    // Finally test color
    VkImageObj c_image(m_device);
    c_image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(c_image.initialized());
    conc_test.image_barrier_.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    conc_test.image_barrier_.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    conc_test.image_barrier_.image = c_image.handle();

    const char *color_vuid = "VUID-VkImageMemoryBarrier2-image-01671";

    // COLOR bit must be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test(color_vuid);

    // No bits other than COLOR may be set
    conc_test.image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
    conc_test(color_vuid);

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
            {img_ds,       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_xfer_src, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_sampled,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            {img_input,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01208"},
            // images _without_ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01209"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  "VUID-VkImageMemoryBarrier2-oldLayout-01210"},
            // images _without_ VK_IMAGE_USAGE_SAMPLED_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
            {img_color,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_ds,       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_xfer_src, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         "VUID-VkImageMemoryBarrier2-oldLayout-01211"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01212"},
            // images _without_ VK_IMAGE_USAGE_TRANSFER_DST_BIT
            {img_color,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_ds,       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_sampled,  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            {img_input,    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             "VUID-VkImageMemoryBarrier2-oldLayout-01213"},
            // images _without_ VK_KHR_maintenance2 added layouts
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01658"},
            {img_color,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_xfer_src, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_sampled,  VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            {img_input,    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, "VUID-VkImageMemoryBarrier2-oldLayout-01659"},
            // clang-format on
        };
        const uint32_t layout_count = sizeof(bad_buffer_layouts) / sizeof(bad_buffer_layouts[0]);

        for (uint32_t i = 0; i < layout_count; ++i) {
            const VkImageLayout bad_layout = bad_buffer_layouts[i].bad_layout;
            // Skip layouts that require maintenance2 support
            if ((maintenance2 == false) && ((bad_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL) ||
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
    // TODO: synchronization2 has a separate VUID for every access flag. Gotta test them all..
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    conc_test.buffer_barrier_.offset = 0;
    conc_test.buffer_barrier_.size = VK_WHOLE_SIZE;
    conc_test("", "VUID-VkBufferMemoryBarrier2-srcAccessMask-03909");

    // Attempt barrier where dstAccessMask is not supported by dstStageMask
    conc_test.buffer_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    conc_test.buffer_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    conc_test("", "VUID-VkBufferMemoryBarrier2-dstAccessMask-03911");

    // Attempt to mismatch barriers/waitEvents calls with incompatible queues
    // Create command pool with incompatible queueflags
    const std::vector<VkQueueFamilyProperties> queue_props = m_device->queue_props;
    uint32_t queue_family_index = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, false);
    if (queue_family_index == UINT32_MAX) {
        printf("%s No non-graphics queue supporting compute found; skipped.\n", kSkipPrefix);
        return;  // NOTE: this exits the test function!
    }
    printf("qfi=%d\n", queue_family_index);

    auto buf_barrier = lvl_init_struct<VkBufferMemoryBarrier2KHR>();
    buf_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    buf_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    buf_barrier.buffer = buffer.handle();
    buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    buf_barrier.offset = 0;
    buf_barrier.size = VK_WHOLE_SIZE;

    dep_info = lvl_init_struct<VkDependencyInfoKHR>();
    dep_info.bufferMemoryBarrierCount = 1;
    dep_info.pBufferMemoryBarriers = &buf_barrier;

    m_commandBuffer->PipelineBarrier2KHR(&dep_info);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier2-srcStageMask-03849");

    VkCommandPoolObj command_pool(m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj bad_command_buffer(m_device, &command_pool);

    bad_command_buffer.begin();
    buf_barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    // Set two bits that should both be supported as a bonus positive check
    buf_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    buf_barrier.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    buf_barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

    bad_command_buffer.PipelineBarrier2KHR(&dep_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidBarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families");
    SetTargetApiVersion(VK_API_VERSION_1_0);
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Find queues of two families
    const uint32_t submit_family = m_device->graphics_queue_node_index_;
    const uint32_t queue_family_count = static_cast<uint32_t>(m_device->queue_props.size());
    const uint32_t other_family = submit_family != 0 ? 0 : 1;
    const bool only_one_family = (queue_family_count == 1) || (m_device->queue_props[other_family].queueCount == 0);

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
            static const char *img_vuid = "VUID-VkImageMemoryBarrier-synchronization2-03856";
            static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-synchronization2-03852";
            conc_test(img_vuid, buf_vuid, VK_QUEUE_FAMILY_IGNORED, submit_family);
            conc_test(img_vuid, buf_vuid, submit_family, VK_QUEUE_FAMILY_IGNORED);
            conc_test(img_vuid, buf_vuid, submit_family, submit_family);
            // true -> positive test
            conc_test(img_vuid, buf_vuid, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, true);
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

        // Although other_family does not match submit_family, because the barrier families are
        // equal here, no ownership transfer actually happens, and this barrier is valid by the spec.
        excl_test("POSITIVE_TEST", "POSITIVE_TEST", other_family, other_family, true, submit_family);

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

        // core_validation::barrier_queue_families::kSubmitQueueMustMatchSrcOrDst
        // Need a third queue family to test this.
        uint32_t third_family = VK_QUEUE_FAMILY_IGNORED;
        for (uint32_t candidate = 0; candidate < queue_family_count; ++candidate) {
            if (candidate != submit_family && candidate != other_family && m_device->queue_props[candidate].queueCount != 0) {
                third_family = candidate;
                break;
            }
        }

        if (third_family == VK_QUEUE_FAMILY_IGNORED) {
            printf("%s No third queue family found -- kSubmitQueueMustMatchSrcOrDst test skipped.\n", kSkipPrefix);
        } else {
            excl_test("UNASSIGNED-CoreValidation-VkImageMemoryBarrier-sharing-mode-exclusive-same-family",
                      "UNASSIGNED-CoreValidation-VkBufferMemoryBarrier-sharing-mode-exclusive-same-family",
                      other_family, third_family, false, submit_family);
        }
    }
}

TEST_F(VkLayerTest, InvalidBarrierQueueFamilyWithMemExt) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families when memory extension is enabled ");
    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    // Check for external memory device extensions
    if (!AreRequestedExtensionsEnabled()) {
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
        static const char *img_vuid = "VUID-VkImageMemoryBarrier-synchronization2-03857";
        static const char *buf_vuid = "VUID-VkBufferMemoryBarrier-synchronization2-03853";
        conc_test(img_vuid, buf_vuid, submit_family, submit_family);
        // true -> positive test
        conc_test(img_vuid, buf_vuid, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, true);
        conc_test(img_vuid, buf_vuid, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
        conc_test(img_vuid, buf_vuid, VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED, true);

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

    VkImageMemoryBarrier img_barrier_template = LvlInitStruct<VkImageMemoryBarrier>();
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
        VkEventCreateInfo eci = LvlInitStruct<VkEventCreateInfo>();
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

TEST_F(VkLayerTest, Sync2InvalidBarrierQueueFamily) {
    TEST_DESCRIPTION("Create and submit barriers with invalid queue families with synchronization2");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }
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
    Barrier2QueueFamilyTestHelper::Context test_context2(this, qf_indices);

    if (only_one_family) {
        printf("%s Single queue family found -- VK_SHARING_MODE_CONCURRENT testcases skipped.\n", kSkipPrefix);
    } else {
        std::vector<uint32_t> families = {submit_family, other_family};
        BarrierQueueFamilyTestHelper conc_test(&test_context);

        // core_validation::barrier_queue_families::kSrcOrDstMustBeIgnore
        conc_test.Init(&families);
        conc_test("VUID-VkImageMemoryBarrier-synchronization2-03857", "VUID-VkBufferMemoryBarrier-synchronization2-03853",
                  submit_family, submit_family, true);
        // true -> positive test
        conc_test("VUID-VkImageMemoryBarrier-synchronization2-03857", "VUID-VkBufferMemoryBarrier-synchronization2-03853",
                  VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, true);
        conc_test("VUID-VkImageMemoryBarrier-synchronization2-03857", "VUID-VkBufferMemoryBarrier-synchronization2-03853",
                  VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
        conc_test("VUID-VkImageMemoryBarrier-synchronization2-03857", "VUID-VkBufferMemoryBarrier-synchronization2-03853",
                  VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED, true);

        Barrier2QueueFamilyTestHelper conc_test2(&test_context2);
        conc_test2.Init(&families);
        conc_test2("POSITIVE_TEST", "POSITIVE_TEST", VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, true);
        conc_test2("POSITIVE_TEST", "POSITIVE_TEST", VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
        conc_test2("POSITIVE_TEST", "POSITIVE_TEST", VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED, true);

        // core_validation::barrier_queue_families::kSpecialOrIgnoreOnly
        conc_test2("VUID-VkImageMemoryBarrier2-image-04071", "VUID-VkBufferMemoryBarrier2-buffer-04088", submit_family,
                   VK_QUEUE_FAMILY_IGNORED);
        conc_test2("VUID-VkImageMemoryBarrier2-image-04071", "VUID-VkBufferMemoryBarrier2-buffer-04088",
                   VK_QUEUE_FAMILY_IGNORED, submit_family);
        // This is to flag the errors that would be considered only "unexpected" in the parallel case above
        // true -> positive test
        conc_test2("VUID-VkImageMemoryBarrier2-image-04071", "VUID-VkBufferMemoryBarrier2-buffer-04088",
                   VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
        conc_test2("VUID-VkImageMemoryBarrier2-image-04071", "VUID-VkBufferMemoryBarrier2-buffer-04088",
                   VK_QUEUE_FAMILY_EXTERNAL_KHR, VK_QUEUE_FAMILY_IGNORED, true);
    }

    Barrier2QueueFamilyTestHelper excl_test(&test_context2);
    excl_test.Init(nullptr);  // no queue families means *exclusive* sharing mode.

    // core_validation::barrier_queue_families::kSrcAndDstValidOrSpecial
    excl_test("VUID-VkImageMemoryBarrier2-image-04072", "VUID-VkBufferMemoryBarrier2-buffer-04089", submit_family, invalid);
    excl_test("VUID-VkImageMemoryBarrier2-image-04072", "VUID-VkBufferMemoryBarrier2-buffer-04089", invalid, submit_family);
    // true -> positive test
    excl_test("VUID-VkImageMemoryBarrier2-image-04072", "VUID-VkBufferMemoryBarrier2-buffer-04089", submit_family,
              submit_family, true);
    excl_test("VUID-VkImageMemoryBarrier2-image-04072", "VUID-VkBufferMemoryBarrier2-buffer-04089", submit_family,
              VK_QUEUE_FAMILY_EXTERNAL_KHR, true);
    excl_test("VUID-VkImageMemoryBarrier2-image-04072", "VUID-VkBufferMemoryBarrier2-buffer-04089",
              VK_QUEUE_FAMILY_EXTERNAL_KHR, submit_family, true);
}

TEST_F(VkLayerTest, IdxBufferAlignmentError) {
    // Bind a BeginRenderPass within an active RenderPass
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t const indices[] = {0};
    VkBufferCreateInfo buf_info = LvlInitStruct<VkBufferCreateInfo>();
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

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
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

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!CanEnableDeviceExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }
    const bool copy_commands2 = CanEnableDeviceExtension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);

    if (!CheckSynchronization2SupportAndInitState(this)) {
        printf("%s Synchronization2 not supported, skipping test\n", kSkipPrefix);
        return;
    }

    PFN_vkCmdCopyImage2KHR vkCmdCopyImage2Function = nullptr;
    if (copy_commands2) {
        vkCmdCopyImage2Function = (PFN_vkCmdCopyImage2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyImage2KHR");
    }

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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    // Equivalent tests using KHR_copy_commands2
    if (copy_commands2 && vkCmdCopyImage2Function) {
        const VkImageCopy2KHR copy_region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                              NULL,
                                              copy_region.srcSubresource,
                                              copy_region.srcOffset,
                                              copy_region.dstSubresource,
                                              copy_region.dstOffset,
                                              copy_region.extent};
        VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                NULL,
                                                src_image.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                dst_image.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                1,
                                                &copy_region2};
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // The first call hits the expected WARNING and skips the call down the chain, so call a second time to call down chain and
        // update layer state
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        // Now cause error due to src image layout changing
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcImageLayout-00128");
        m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // Final src error is due to bad layout type
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-srcImageLayout-00129");
        m_errorMonitor->SetUnexpectedError(
            "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout "
            "VK_IMAGE_LAYOUT_GENERAL.");
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // Now verify same checks for dst
        m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                             "layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL.");
        copy_image_info2.srcImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        // Now cause error due to src image layout changing
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImageLayout-00133");
        m_errorMonitor->SetUnexpectedError(
            "is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL or VK_IMAGE_LAYOUT_GENERAL.");
        copy_image_info2.dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyImageInfo2-dstImageLayout-00134");
        m_errorMonitor->SetUnexpectedError(
            "with specific layout VK_IMAGE_LAYOUT_UNDEFINED that doesn't match the previously used layout "
            "VK_IMAGE_LAYOUT_GENERAL.");
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }

    // Convert dst and depth images to TRANSFER_DST for subsequent tests
    VkImageMemoryBarrier transfer_dst_image_barrier[1] = {};
    transfer_dst_image_barrier[0] = LvlInitStruct<VkImageMemoryBarrier>();
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

    VkImageMemoryBarrier image_barrier[1] = {};
    // In synchronization2, if oldLayout == newLayout, we're not doing an ILT and these fields don't need to match
    // the image's layout.
    image_barrier[0] = LvlInitStruct<VkImageMemoryBarrier>();
    image_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_barrier[0].image = src_image.handle();
    image_barrier[0].subresourceRange.layerCount = image_create_info.arrayLayers;
    image_barrier[0].subresourceRange.levelCount = image_create_info.mipLevels;
    image_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_errorMonitor->ExpectSuccess();
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           NULL, 0, NULL, 1, image_barrier);
    m_errorMonitor->VerifyNotFound();

    // Now cause error due to bad image layout transition in PipelineBarrier
    image_barrier[0] = LvlInitStruct<VkImageMemoryBarrier>();
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
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
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

    AddRequiredExtensions(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported on this platform, skipping test.\n", kSkipPrefix,
               VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageObj dst_image(m_device);
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

TEST_F(VkLayerTest, CopyInvalidImageMemory) {
    TEST_DESCRIPTION("Validate 4 invalid image memory VUIDs ");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool copy_commands2 = CanEnableDeviceExtension(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdCopyImage2KHR vkCmdCopyImage2Function = nullptr;
    if (copy_commands2) {
        vkCmdCopyImage2Function = (PFN_vkCmdCopyImage2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdCopyImage2KHR");
    }
    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>();
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    // Create a small image with a dedicated allocation
    VkImageObj image_no_mem(m_device);
    image_no_mem.init_no_mem(*m_device, image_info);
    VkImageObj image(m_device);
    image.init(&image_info);

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
    copy_region.extent.width = 4;
    copy_region.extent.height = 4;
    copy_region.extent.depth = 1;

    std::string vuid;
    bool ycbcr = (DeviceExtensionEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME) ||
                  (DeviceValidationVersion() >= VK_API_VERSION_1_1));

    m_commandBuffer->begin();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-srcImage-01546" : "VUID-vkCmdCopyImage-srcImage-00127";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
    m_commandBuffer->CopyImage(image_no_mem.handle(), VK_IMAGE_LAYOUT_UNDEFINED, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();
    vuid = ycbcr ? "VUID-vkCmdCopyImage-dstImage-01547" : "VUID-vkCmdCopyImage-dstImage-00132";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
    m_errorMonitor->SetUnexpectedError("is VK_IMAGE_LAYOUT_UNDEFINED but can only be VK_IMAGE_LAYOUT");
    m_commandBuffer->CopyImage(image.handle(), VK_IMAGE_LAYOUT_UNDEFINED, image_no_mem.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                               &copy_region);
    m_errorMonitor->VerifyFound();

    if (copy_commands2 && vkCmdCopyImage2Function) {
        const VkImageCopy2KHR copy_region2 = {VK_STRUCTURE_TYPE_IMAGE_COPY_2_KHR,
                                              NULL,
                                              copy_region.srcSubresource,
                                              copy_region.srcOffset,
                                              copy_region.dstSubresource,
                                              copy_region.dstOffset,
                                              copy_region.extent};
        VkCopyImageInfo2KHR copy_image_info2 = {VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2_KHR,
                                                NULL,
                                                image_no_mem.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                image.handle(),
                                                VK_IMAGE_LAYOUT_GENERAL,
                                                1,
                                                &copy_region2};
        vuid = ycbcr ? "VUID-VkCopyImageInfo2-srcImage-01546" : "VUID-VkCopyImageInfo2-srcImage-00127";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL.");
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
        copy_image_info2.srcImage = image.handle();
        copy_image_info2.dstImage = image_no_mem.handle();
        vuid = ycbcr ? "VUID-VkCopyImageInfo2-dstImage-01547" : "VUID-VkCopyImageInfo2-dstImage-00132";
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, vuid);
        m_errorMonitor->SetUnexpectedError("layout should be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL instead of GENERAL.");
        m_errorMonitor->SetUnexpectedError("doesn't match the previously used layout VK_IMAGE_LAYOUT_GENERAL..");
        vkCmdCopyImage2Function(m_commandBuffer->handle(), &copy_image_info2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, CreateImageViewBreaksParameterCompatibilityRequirements) {
    TEST_DESCRIPTION(
        "Attempts to create an Image View with a view type that does not match the image type it is being created from.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool maintenance1_support = CanEnableDeviceExtension(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
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
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
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
    ivci = LvlInitStruct<VkImageViewCreateInfo>();
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
    ivci = LvlInitStruct<VkImageViewCreateInfo>();
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
    if (maintenance1_support) {
        CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-06727");
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
    if (res || !m_device->phy().features().sparseBinding || !maintenance1_support) {
        printf("%s %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
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
    ivci = LvlInitStruct<VkImageViewCreateInfo>();
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
        VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
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
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

    // The 02277 VU is 'probably' redundant, but keeping incase a future spec change
    // This extra VU checked is because depth formats are only compatible with themselves
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-01018");
    // Test for error message
    CreateImageViewTest(*this, &ivci, optimal_error_codes[i]);
}

TEST_F(VkLayerTest, InvalidImageViewUsageCreateInfo) {
    TEST_DESCRIPTION("Usage modification via a chained VkImageViewUsageCreateInfo struct");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Test requires DeviceProfileLayer, unavailable - skipped.\n", kSkipPrefix);
        return;
    }

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE_2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
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

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
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
    VkImageViewUsageCreateInfo usage_ci = LvlInitStruct<VkImageViewUsageCreateInfo>();
    usage_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
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

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE_2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    // without VK_EXT_separate_stencil_usage explicitly enabled
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageSubresourceRange range = {aspect, 0, 1, 0, 1};

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_create_info.flags = 0;
    image_view_create_info.image = VK_NULL_HANDLE;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = depth_format;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    image_view_create_info.subresourceRange = range;

    VkImageViewUsageCreateInfo image_view_usage_create_info = LvlInitStruct<VkImageViewUsageCreateInfo>();
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

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!CanEnableDeviceExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE_2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }

    if (!CanEnableDeviceExtension(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME)) {
        printf("%s VK_EXT_separate_stencil_usage Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    const auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkImageAspectFlags aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
    const VkImageSubresourceRange range = {aspect, 0, 1, 0, 1};

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_create_info.flags = 0;
    image_view_create_info.image = VK_NULL_HANDLE;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = depth_format;
    image_view_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                         VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
    image_view_create_info.subresourceRange = range;

    VkImageViewUsageCreateInfo image_view_usage_create_info = LvlInitStruct<VkImageViewUsageCreateInfo>();
    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    image_view_create_info.pNext = &image_view_usage_create_info;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());
    image_view_create_info.image = image.handle();

    image_view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;  // Extra flag

    // VkImageViewUsageCreateInfo::usage must not include any bits that were not set in VkImageCreateInfo::usage
    CreateImageViewTest(*this, &image_view_create_info, "VUID-VkImageViewCreateInfo-pNext-02662");

    VkImageStencilUsageCreateInfoEXT image_stencil_create_info = LvlInitStruct<VkImageStencilUsageCreateInfoEXT>();
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

    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
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

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.layerCount = 1;
    // Cause an error by setting an invalid image aspect
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;

    CreateImageViewTest(*this, &image_view_create_info, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");
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
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04461");
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

    // 04462 If format has a depth component the aspectMask member of pResource must containt VK_IMAGE_ASPECT_DEPTH_BIT
    {
        VkFormat format = VK_FORMAT_D32_SFLOAT;
        VkFormatProperties image_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &image_format_properties);
        if ((image_format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) > 0) {
            VkImageObj img(m_device);
            img.InitNoLayout(32, 32, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

            VkImageSubresource subres = {};
            subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // ERROR: triggers VU 04462

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04462");
            vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // 04463 If format has a stencil component the aspectMask member of pResource must containt VK_IMAGE_ASPECT_STENCIL_BIT
    {
        VkFormat format = VK_FORMAT_S8_UINT;
        VkFormatProperties image_format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), format, &image_format_properties);
        if ((image_format_properties.linearTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) > 0) {
            VkImageObj img(m_device);
            img.InitNoLayout(32, 32, 1, format, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

            VkImageSubresource subres = {};
            subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  // ERROR: triggers VU 04463

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04463");
            vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
            m_errorMonitor->VerifyFound();
        }
    }

    // 04464 If format does not contain stencil or depth component the aspectMask member of pResource must not contain
    // VK_IMAGE_ASPECT_DEPTH_BIT or VK_IMAGE_ASPECT_STENCIL_BIT
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;  // ERROR: triggers VU 00997 and 04464

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04461");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04464");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }
    {
        VkImageObj img(m_device);
        img.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        ASSERT_TRUE(img.initialized());

        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;  // ERROR: triggers VU 00997 and 04464

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04461");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageSubresourceLayout-format-04464");
        vk::GetImageSubresourceLayout(m_device->device(), img.image(), &subres, &subres_layout);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, ImageLayerUnsupportedFormat) {
    TEST_DESCRIPTION("Creating images with unsupported formats ");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create image with unsupported format - Expect FORMAT_UNSUPPORTED
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageViewCreateInfo imgViewInfo = LvlInitStruct<VkImageViewCreateInfo>();
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

    VkImageViewCreateInfo imgViewInfo = LvlInitStruct<VkImageViewCreateInfo>();
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
        printf("%s Device does not support R8_UINT as color attachment; skipped\n", kSkipPrefix);
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

    VkImageViewCreateInfo imgViewInfo = LvlInitStruct<VkImageViewCreateInfo>();
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
        (DeviceExtensionEnabled(VK_KHR_MAINTENANCE_2_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1));
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

    // Use 1.1 to get VK_KHR_sampler_ycbcr_conversion easier
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s test requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        printf("samplerYcbcrConversion not supported, skipping test\n");
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    ycbcr_create_info.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;

    VkSamplerYcbcrConversion conversion;
    vk::CreateSamplerYcbcrConversion(m_device->device(), &ycbcr_create_info, nullptr, &conversion);

    VkSamplerYcbcrConversionInfo ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    ycbcr_info.conversion = conversion;

    VkImageCreateInfo ci = LvlInitStruct<VkImageCreateInfo>();
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

        VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>(&ycbcr_info);
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

        VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>(&ycbcr_info);
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
    vk::DestroySamplerYcbcrConversion(m_device->device(), conversion, nullptr);
}

TEST_F(VkLayerTest, CreateImageViewInvalidSubresourceRange) {
    TEST_DESCRIPTION("Passing bad image subrange to CreateImageView");
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool maintenance1 = CanEnableDeviceExtension(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.create_info().arrayLayers == 1);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo img_view_info_template = LvlInitStruct<VkImageViewCreateInfo>();
    img_view_info_template.image = image.handle();
    img_view_info_template.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    img_view_info_template.format = image.format();
    // subresourceRange to be filled later for the purposes of this test
    img_view_info_template.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_view_info_template.subresourceRange.baseMipLevel = 0;
    img_view_info_template.subresourceRange.levelCount = 0;
    img_view_info_template.subresourceRange.baseArrayLayer = 0;
    img_view_info_template.subresourceRange.layerCount = 0;

    auto const base_layer_vuid =
        maintenance1 ? "VUID-VkImageViewCreateInfo-image-01482" : "VUID-VkImageViewCreateInfo-subresourceRange-01480";
    auto const layer_count_vuid =
        maintenance1 ? "VUID-VkImageViewCreateInfo-subresourceRange-01483" : "VUID-VkImageViewCreateInfo-subresourceRange-01719";

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

    // Try baseArrayLayer >= image.arrayLayers with VK_REMAINING_ARRAY_LAYERS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, VK_REMAINING_ARRAY_LAYERS};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        CreateImageViewTest(*this, &img_view_info, base_layer_vuid);
    }

    // Try baseArrayLayer >= image.arrayLayers without VK_REMAINING_ARRAY_LAYERS
    {
        const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
        VkImageViewCreateInfo img_view_info = img_view_info_template;
        img_view_info.subresourceRange = range;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, layer_count_vuid);
        CreateImageViewTest(*this, &img_view_info, base_layer_vuid);
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
        CreateImageViewTest(*this, &img_view_info, layer_count_vuid);
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

        VkImageViewCreateInfo cube_img_view_info_template = LvlInitStruct<VkImageViewCreateInfo>();
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

    {
        VkImageObj volumeImage(m_device);
        auto image_ci = vk_testing::Image::create_info();
        image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        image_ci.extent = {8, 8, 8};
        image_ci.mipLevels = 4;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        volumeImage.init(&image_ci);

        VkImageViewCreateInfo volume_img_view_info_template = LvlInitStruct<VkImageViewCreateInfo>();
        volume_img_view_info_template.image = volumeImage.handle();
        volume_img_view_info_template.format = volumeImage.format();

        // 3D views
        {
            // first mip
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // all mips
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 4, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // too many mips
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 5, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
            }
            // invalid base mip
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 5, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-01718");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-01478");
            }
            // too many layers
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-imageViewType-04973");
                CreateImageViewTest(*this, &img_view_info, layer_count_vuid);
            }
            // invalid base layer
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, layer_count_vuid);
                CreateImageViewTest(*this, &img_view_info, base_layer_vuid);
            }
        }
        if (maintenance1) {
            // 2D views
            // first mip, first layer
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // all mips, first layer (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 4, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-image-04970");
            }
            // first mip, all layers (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-imageViewType-04973");
            }
            // mip 3, 8 layers (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-imageViewType-04973");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }
            // mip 3, layer 7 (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 7, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02724");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }
            // 2D array views
            // first mip, first layer
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // all mips, first layer (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 4, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-image-04970");
            }
            // first mip, all layers
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // mip 3, layer 0
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 0, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info);
            }
            // mip 3, 8 layers (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 0, 8};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }
            // mip 3, layer 7 (invalid)
            {
                const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 3, 1, 7, 1};
                VkImageViewCreateInfo img_view_info = volume_img_view_info_template;
                img_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                img_view_info.subresourceRange = range;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-02724");
                CreateImageViewTest(*this, &img_view_info, "VUID-VkImageViewCreateInfo-subresourceRange-02725");
            }

            // Checking sparse flags are not set
            VkImageViewCreateInfo sparse_image_view_ci = volume_img_view_info_template;
            sparse_image_view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            // using VK_IMAGE_CREATE_SPARSE_BINDING_BIT
            if (device_features.sparseBinding) {
                VkImageObj sparse_image(m_device);
                image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
                sparse_image.Init(image_ci, 0, false);
                sparse_image_view_ci.image = sparse_image.handle();

                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
            }
            // using VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT
            if (device_features.sparseResidencyImage3D) {
                VkImageObj sparse_image(m_device);
                image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-flags-00987");
                sparse_image.Init(image_ci, 0, false);
                sparse_image_view_ci.image = sparse_image.handle();

                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
            }
            // using VK_IMAGE_CREATE_SPARSE_ALIASED_BIT
            if (device_features.sparseResidencyAliased) {
                VkImageObj sparse_image(m_device);
                image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT |
                                 VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
                sparse_image.Init(image_ci, 0, false);
                sparse_image_view_ci.image = sparse_image.handle();

                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
                sparse_image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01020");
                CreateImageViewTest(*this, &sparse_image_view_ci, "VUID-VkImageViewCreateInfo-image-04971");
            }
        }
    }
}

TEST_F(VkLayerTest, InvalidImageViewLayerCount) {
    TEST_DESCRIPTION("Image and ImageView arrayLayers/layerCount parameters not being compatibile");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>(nullptr);
    image_ci.flags = 0;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.extent = {128, 1, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    VkImageObj image_1d(m_device);
    image_1d.init(&image_ci);
    ASSERT_TRUE(image_1d.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_2D;
    VkImageObj image_2d(m_device);
    image_2d.init(&image_ci);
    ASSERT_TRUE(image_2d.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    VkImageObj image_3d(m_device);
    image_3d.init(&image_ci);
    ASSERT_TRUE(image_3d.initialized());

    image_ci.arrayLayers = 2;

    image_ci.imageType = VK_IMAGE_TYPE_1D;
    VkImageObj image_1d_array(m_device);
    image_1d_array.init(&image_ci);
    ASSERT_TRUE(image_1d_array.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_2D;
    VkImageObj image_2d_array(m_device);
    image_2d_array.init(&image_ci);
    ASSERT_TRUE(image_2d_array.initialized());

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    VkImageFormatProperties img_limits;
    ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));
    layer_data::optional<VkImageObj> image_3d_array;
    if (img_limits.maxArrayLayers >= image_ci.arrayLayers) {
        image_3d_array.emplace(m_device);
        image_3d_array->init(&image_ci);
        ASSERT_TRUE(image_3d_array->initialized());
    }

    // base for each test that never changes
    VkImageViewCreateInfo image_view_ci = LvlInitStruct<VkImageViewCreateInfo>(nullptr);
    image_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Sanity checks
    {
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = 1;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        if (image_3d_array) {
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
            image_view_ci.image = image_3d_array->image();
            CreateImageViewTest(*this, &image_view_ci);
        }

        image_view_ci.subresourceRange.baseArrayLayer = 1;
        image_view_ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci);
        if (image_3d_array) {
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
            image_view_ci.image = image_3d_array->image();
            CreateImageViewTest(*this, &image_view_ci);
        }

        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d.image();
        CreateImageViewTest(*this, &image_view_ci);
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
        image_view_ci.image = image_3d.image();
        CreateImageViewTest(*this, &image_view_ci);
    }

    // layerCount is not 1 as imageView is not an array type
    {
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = 2;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04973");

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04973");

        if (image_3d_array) {
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
            image_view_ci.image = image_3d_array->image();
            CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04973");
        }
    }

    // layerCount is VK_REMAINING_ARRAY_LAYERS but not 1
    {
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_1D;
        image_view_ci.image = image_1d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04974");

        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.image = image_2d_array.image();
        CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04974");

        if (image_3d_array) {
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_3D;
            image_view_ci.image = image_3d_array->image();
            CreateImageViewTest(*this, &image_view_ci, "VUID-VkImageViewCreateInfo-imageViewType-04974");
        }
    }
}

TEST_F(VkLayerTest, CreateImageMiscErrors) {
    TEST_DESCRIPTION("Misc leftover valid usage errors in VkImageCreateInfo struct");

    VkPhysicalDeviceFeatures features{};
    ASSERT_NO_FATAL_FAILURE(Init(&features));

    VkImageCreateInfo tmp_img_ci = LvlInitStruct<VkImageCreateInfo>();
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

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
        image_ci.mipLevels = 1;
        image_ci.tiling = VK_IMAGE_TILING_LINEAR;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-samples-02257");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 2;
        image_ci.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-02259");

        image_ci = safe_image_ci;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.mipLevels = 1;
        image_ci.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
        image_ci.tiling = VK_IMAGE_TILING_LINEAR;
        CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-flags-02259");
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

    VkImageCreateInfo tmp_img_ci = LvlInitStruct<VkImageCreateInfo>();
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
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool push_fragment_density_support = CanEnableDeviceExtension(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    const bool push_fragment_density_offset_support = CanEnableDeviceExtension(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, 0));

    VkImageCreateInfo tmp_img_ci = LvlInitStruct<VkImageCreateInfo>();
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
            image_ci.format = VK_FORMAT_R8G8_UNORM;  // only mandatory format for fragment density map
            VkImageFormatProperties img_limits;
            ASSERT_VK_SUCCESS(GPDIFPHelper(gpu(), &image_ci, &img_limits));

            image_ci.extent = {dev_limits.maxFramebufferWidth + 1, 64, 1};
            if (dev_limits.maxFramebufferWidth + 1 > img_limits.maxExtent.width) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02252");
            }

            if (!push_fragment_density_offset_support) {
                CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06514");
            }

            image_ci.extent = {64, dev_limits.maxFramebufferHeight + 1, 1};
            if (dev_limits.maxFramebufferHeight + 1 > img_limits.maxExtent.height) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-extent-02253");
            }

            if (!push_fragment_density_offset_support) {
                CreateImageTest(*this, &image_ci, "VUID-VkImageCreateInfo-fragmentDensityMapOffset-06515");
            }
        }
    }
}

TEST_F(VkLayerTest, SamplerImageViewFormatUnsupportedFilter) {
    TEST_DESCRIPTION(
        "Create sampler with a filter and use with image view using a format that does not support the sampler filter.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool cubic_support = CanEnableDeviceExtension(VK_IMG_FILTER_CUBIC_EXTENSION_NAME);

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
    tests[0].err_msg = "VUID-vkCmdDraw-magFilter-04553";

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

    const char bindStateFragiSamplerShaderText[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform isampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";

    const char bindStateFraguSamplerShaderText[] = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform usampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";

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
        sci.compareEnable = VK_FALSE;

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
            fs = new VkShaderObj(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
        } else if (test_struct.format_type == SINT) {
            fs = new VkShaderObj(this, bindStateFragiSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
        } else if (test_struct.format_type == UINT) {
            fs = new VkShaderObj(this, bindStateFraguSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
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

    AddRequiredExtensions(VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
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
    VkShaderObj fs(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

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

    // Use 1.1 to get VK_KHR_sampler_ycbcr_conversion easier
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s test requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        printf("samplerYcbcrConversion not supported, skipping test\n");
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

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
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    ycbcr_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;
    VkSamplerYcbcrConversion conversions[2];
    vk::CreateSamplerYcbcrConversion(m_device->handle(), &ycbcr_create_info, nullptr, &conversions[0]);
    ycbcr_create_info.components.a = VK_COMPONENT_SWIZZLE_ZERO;  // Just anything different than above
    vk::CreateSamplerYcbcrConversion(m_device->handle(), &ycbcr_create_info, nullptr, &conversions[1]);

    VkSamplerYcbcrConversionInfo ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
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

    if (features2.features.samplerAnisotropy == VK_TRUE) {
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
    ycbcr_info.conversion = conversions[0];  // Need two samplers with different conversions
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>(&ycbcr_info);
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

    if (!descriptor_set.set_) {
        printf("%s Failed to allocate descriptor set, skipping test.\n", kSkipPrefix);
        return;
    }

    // Use the same image view twice, using the same sampler, with the *second* mismatched with the *second* immutable sampler
    VkDescriptorImageInfo image_infos[2];
    image_infos[0] = {};
    image_infos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_infos[0].imageView = view;
    image_infos[0].sampler = samplers[0];
    image_infos[1] = image_infos[0];

    // Update the descriptor set expecting to get an error
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
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

    vk::DestroySamplerYcbcrConversion(m_device->device(), conversions[0], nullptr);
    vk::DestroySamplerYcbcrConversion(m_device->device(), conversions[1], nullptr);
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

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidImageAspect");

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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
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

    if (!AddRequiredInstanceExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // Required extensions except VK_KHR_GET_MEMORY_REQUIREMENTS_2 -- to create the needed error
    std::vector<const char *> required_device_extensions = {VK_KHR_MAINTENANCE_1_EXTENSION_NAME, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
                                                            VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME};
    for (auto dev_ext : required_device_extensions) {
        if (DeviceExtensionSupported(dev_ext)) {
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
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    ycbcr_create_info.format = VK_FORMAT_UNDEFINED;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;
    VkSamplerYcbcrConversion conversion;
    vkCreateSamplerYcbcrConversionKHR(m_device->handle(), &ycbcr_create_info, nullptr, &conversion);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCreateBufferSize) {
    TEST_DESCRIPTION("Attempt to create VkBuffer with size of zero");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkBufferCreateInfo info = LvlInitStruct<VkBufferCreateInfo>();
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
    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_0 =
        LvlInitStruct<VkPhysicalDeviceProtectedMemoryProperties>();

    VkPhysicalDeviceIDProperties id_properties = LvlInitStruct<VkPhysicalDeviceIDProperties>(&protected_memory_properties_0);

    VkPhysicalDeviceProtectedMemoryProperties protected_memory_properties_1 =
        LvlInitStruct<VkPhysicalDeviceProtectedMemoryProperties>(&id_properties);

    VkPhysicalDeviceProperties2 physical_device_properties2 =
        LvlInitStruct<VkPhysicalDeviceProperties2>(&protected_memory_properties_1);

    vk::GetPhysicalDeviceProperties2(gpu(), &physical_device_properties2);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DedicatedAllocationBinding) {
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
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
    m_errorMonitor->ExpectSuccess();
    vk::BindImageMemory(m_device->handle(), image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, DedicatedAllocationImageAliasing) {
    auto aliasing_features = LvlInitStruct<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&aliasing_features);
    m_device_extension_names.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_NV_DEDICATED_ALLOCATION_IMAGE_ALIASING_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }
    aliasing_features.dedicatedAllocationImageAliasing = VK_TRUE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

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
    m_errorMonitor->ExpectSuccess();
    vk::BindImageMemory(m_device->handle(), image->handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->ExpectSuccess();
    image.reset();  // destroy the original image
    vk::BindImageMemory(m_device->handle(), post_delete_image.handle(), dedicated_image_memory.handle(), 0);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, CornerSampledImageNV) {
    TEST_DESCRIPTION("Test VK_NV_corner_sampled_image.");
    m_device_extension_names.push_back(VK_NV_CORNER_SAMPLED_IMAGE_EXTENSION_NAME);
    auto corner_sampled_image_features = LvlInitStruct<VkPhysicalDeviceCornerSampledImageFeaturesNV>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&corner_sampled_image_features);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    if (!AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    AddRequiredExtensions(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));
    device_features.shaderStorageImageMultisample = VK_FALSE;  // Force multisampled storage images off

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s VK_EXT_separate_stencil_usage Extension not supported, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

    PFN_vkGetPhysicalDeviceImageFormatProperties2KHR vkGetPhysicalDeviceImageFormatProperties2KHR =
        (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vk::GetInstanceProcAddr(instance(),
                                                                                  "vkGetPhysicalDeviceImageFormatProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceImageFormatProperties2KHR != nullptr);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageStencilUsageCreateInfoEXT image_stencil_create_info = LvlInitStruct<VkImageStencilUsageCreateInfoEXT>();
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT;

    image_create_info.pNext = &image_stencil_create_info;

    VkPhysicalDeviceImageFormatInfo2 image_format_info2 =
        LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&image_stencil_create_info);
    image_format_info2.format = image_create_info.format;
    image_format_info2.type = image_create_info.imageType;
    image_format_info2.tiling = image_create_info.tiling;
    image_format_info2.usage = image_create_info.usage;
    image_format_info2.flags = image_create_info.flags;

    VkImageFormatProperties2 image_format_properties2 = LvlInitStruct<VkImageFormatProperties2>();
    image_format_properties2.imageFormatProperties = {};

    // when including VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, must not include bits other than
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT or VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    image_stencil_create_info.stencilUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539");
    vkGetPhysicalDeviceImageFormatProperties2KHR(m_device->phy().handle(), &image_format_info2, &image_format_properties2);
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

    m_device_extension_names.push_back(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    auto ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ycbcr_features);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

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
    VkSamplerYcbcrConversionCreateInfo sycci = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
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
    VkSamplerYcbcrConversionInfo sampler_ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    sampler_ycbcr_info.conversion = ycbcr_conv;
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

    // Use 1.1 to get VK_KHR_sampler_ycbcr_conversion easier
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s test requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        printf("samplerYcbcrConversion not supported, skipping test\n");
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

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
    VkSamplerYcbcrConversionCreateInfo sycci = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
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
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.a
    sycci.components = identity;
    sycci.components.a = VK_COMPONENT_SWIZZLE_R;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02582");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // make sure zero and one are allowed for components.a
    m_errorMonitor->ExpectSuccess();
    sycci.components.a = VK_COMPONENT_SWIZZLE_ONE;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, nullptr);
    sycci.components.a = VK_COMPONENT_SWIZZLE_ZERO;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, nullptr);
    m_errorMonitor->VerifyNotFound();

    // test components.r
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_G;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02583");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.b
    sycci.components = identity;
    sycci.components.b = VK_COMPONENT_SWIZZLE_G;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02584");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // test components.r and components.b together
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_B;
    sycci.components.b = VK_COMPONENT_SWIZZLE_B;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02585");
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    m_errorMonitor->VerifyFound();

    // make sure components.r and components.b can be swapped
    m_errorMonitor->ExpectSuccess();
    sycci.components = identity;
    sycci.components.r = VK_COMPONENT_SWIZZLE_B;
    sycci.components.b = VK_COMPONENT_SWIZZLE_R;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, nullptr);
    m_errorMonitor->VerifyNotFound();

    // Non RGB Identity model
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY;
    {
        // Non RGB Identity can't have a explicit zero swizzle
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_ZERO;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
        m_errorMonitor->VerifyFound();

        // Non RGB Identity can't have a explicit one swizzle
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_ONE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
        m_errorMonitor->VerifyFound();

        // VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM has 3 component so RGBA conversion has implicit A as one
        sycci.components = identity;
        sycci.components.g = VK_COMPONENT_SWIZZLE_A;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerYcbcrConversionCreateInfo-components-02581");
        vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
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
        vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);
        vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, nullptr);
        m_errorMonitor->VerifyNotFound();
    }

    // Create a valid conversion with guaranteed support
    sycci.format = mp_format;
    sycci.components = identity;
    vk::CreateSamplerYcbcrConversion(device(), &sycci, NULL, &ycbcr_conv);

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkSamplerYcbcrConversionInfo conversion_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    conversion_info.conversion = ycbcr_conv;

    VkImageView image_view;
    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>(&conversion_info);
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

    vk::DestroySamplerYcbcrConversion(device(), ycbcr_conv, nullptr);
}

TEST_F(VkLayerTest, BufferDeviceAddressEXT) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    m_device_extension_names.push_back(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferAddressFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
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

    m_errorMonitor->ExpectSuccess();
    vk::BindBufferMemory(m_device->device(), buffer, buffer_mem, 0);

    vk::FreeMemory(m_device->device(), buffer_mem, NULL);
    vk::DestroyBuffer(m_device->device(), buffer, NULL);
    m_errorMonitor->VerifyNotFound();
}

TEST_F(VkLayerTest, BufferDeviceAddressEXTDisabled) {
    TEST_DESCRIPTION("Test VK_EXT_buffer_device_address.");
    m_device_extension_names.push_back(VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferAddressFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    buffer_device_address_features.bufferDeviceAddress = VK_FALSE;
    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
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
    m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

    buffer_device_address_features.bufferDeviceAddressCaptureReplay = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
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

    VkMemoryAllocateFlagsInfo alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
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

    m_device_extension_names.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&buffer_device_address_features);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support this feature, skipping tests\n", kSkipPrefix);
        return;
    }

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

TEST_F(VkLayerTest, CreateImageYcbcrFormats) {
    TEST_DESCRIPTION("Creating images with Ycbcr Formats.");

    if (!EnableDeviceProfileLayer()) {
        printf("%s Failed to enable device profile layer.\n", kSkipPrefix);
        return;
    }

    // Enable KHR multiplane req'd extensions
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    const bool ycbcr_array_extension = CanEnableDeviceExtension(VK_EXT_YCBCR_IMAGE_ARRAYS_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;

    // Load required functions
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        printf("%s Failed to device profile layer.\n", kSkipPrefix);
        return;
    }

    if (!ImageFormatIsSupported(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)) {
        printf("%s VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM is unsupported.\n", kSkipPrefix);
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
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
            (ycbcr_array_extension) ? "VUID-VkImageCreateInfo-format-06414" : "VUID-VkImageCreateInfo-format-06413";
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
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-06410");
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
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-06411");
    image_create_info = reset_create_info;

    // invalid width
    image_create_info.extent.width = 31;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-04712");
    image_create_info = reset_create_info;

    // invalid height (since 420 format)
    image_create_info.extent.height = 31;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-04713");
    image_create_info = reset_create_info;

    // invalid imageType
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    // Can't just set height to 1 as stateless valdiation will hit 04713 first
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-imageType-00956");
    m_errorMonitor->SetUnexpectedError("VUID-VkImageCreateInfo-extent-02253");
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-format-06412");
    image_create_info = reset_create_info;

    // Test using a format that doesn't support disjoint
    image_create_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageCreateFormatFeatures-02260");
    image_create_info = reset_create_info;
}

TEST_F(VkLayerTest, InvalidSamplerFilterMinmax) {
    TEST_DESCRIPTION("Invalid uses of VK_EXT_sampler_filter_minmax.");

    // Enable KHR multiplane req'd extensions
    const bool mp_extensions = AddRequiredExtensions(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME) &&
                               AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    if (!mp_extensions) {
        printf("%s Instance extensions not supported for %s and %s\n", kSkipPrefix, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME,
               VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!CanEnableDeviceExtension(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME)) {
        printf("%s test requires Sampler Filter MinMax extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    if (!CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    // Enable Ycbcr Conversion Features
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
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
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    ycbcr_create_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;

    VkSamplerYcbcrConversion conversion;
    vkCreateSamplerYcbcrConversionFunction(m_device->handle(), &ycbcr_create_info, nullptr, &conversion);

    VkSamplerYcbcrConversionInfo ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    ycbcr_info.conversion = conversion;

    VkSamplerReductionModeCreateInfo reduction_info = LvlInitStruct<VkSamplerReductionModeCreateInfo>();
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

    auto coherent_memory_features_amd = LvlInitStruct<VkPhysicalDeviceCoherentMemoryFeaturesAMD>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&coherent_memory_features_amd);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }
    if (!coherent_memory_features_amd.deviceCoherentMemory) {
        printf("%s device coherent memory amd not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if (IsPlatform(kMockICD) || DeviceSimulation()) {
        printf("%s MockICD does not support the necessary memory type, skipping test\n", kSkipPrefix);
        return;
    }

    if (!AddRequiredDeviceExtensions(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME)) {
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
        printf("%s test requires Vulkan 1.1 extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
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
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool drm_format_modifier = CanEnableDeviceExtension(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitState());

    if (!CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        printf("%s test requires KHR YCbCr extensions, not available.  Skipping.\n", kSkipPrefix);
    } else {
        // Need to make sure disjoint is supported for format
        // Also need to support an arbitrary image usage feature
        VkFormatProperties format_properties;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, &format_properties);
        if (!((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT) &&
              (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))) {
            printf("%s test requires disjoint/sampled feature bit on format.  Skipping.\n", kSkipPrefix);
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
        }
    }
}

TEST_F(VkLayerTest, FragmentDensityMapEnabled) {
    TEST_DESCRIPTION("Validation must check several conditions that apply only when Fragment Density Maps are used.");

    if (!AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME) &&
        !AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extensions for %s or %s; skipped.\n", kSkipPrefix,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME, VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool fdmSupported = CanEnableDeviceExtension(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    const bool fdm2Supported = CanEnableDeviceExtension(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);

    // Check extension support
    if (!(fdmSupported || fdm2Supported)) {
        printf("%s test requires %s or %s extensions. Skipping.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT density_map2_features =
        LvlInitStruct<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&density_map2_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentDensityMap2PropertiesEXT density_map2_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentDensityMap2PropertiesEXT>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&density_map2_properties);
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

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
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

    if (!AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME) &&
        !AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)) {
        printf("%s Did not find required instance extensions for %s or %s; skipped.\n", kSkipPrefix,
               VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME, VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool fdmSupported = CanEnableDeviceExtension(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    const bool fdm2Supported = CanEnableDeviceExtension(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);

    // Check extension support
    if (!(fdmSupported || fdm2Supported)) {
        printf("%s test requires %s or %s extensions. Skipping.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME,
               VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
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
    VkPhysicalDeviceASTCDecodeFeaturesEXT astc_decode_features = LvlInitStruct<VkPhysicalDeviceASTCDecodeFeaturesEXT>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&astc_decode_features);
    m_device_extension_names.push_back(VK_EXT_ASTC_DECODE_MODE_EXTENSION_NAME);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

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

    VkImageViewASTCDecodeModeEXT astc_decode_mode = LvlInitStruct<VkImageViewASTCDecodeModeEXT>();
    astc_decode_mode.decodeMode = VK_FORMAT_R16G16B16A16_SFLOAT;

    VkImageView image_view;
    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>(&astc_decode_mode);
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
    VkPhysicalDeviceCustomBorderColorFeaturesEXT border_color_features =
        LvlInitStruct<VkPhysicalDeviceCustomBorderColorFeaturesEXT>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&border_color_features);
    m_device_extension_names.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

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

    VkSamplerCustomBorderColorCreateInfoEXT custom_color_cinfo = LvlInitStruct<VkSamplerCustomBorderColorCreateInfoEXT>();
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
        LvlInitStruct<VkPhysicalDeviceCustomBorderColorPropertiesEXT>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&custom_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);
    if ((custom_properties.maxCustomBorderColorSamplers <= 0xFFFF) &&
        (prop2.properties.limits.maxSamplerAllocationCount >= custom_properties.maxCustomBorderColorSamplers)) {
        VkSampler samplers[0xFFFF];
        // Still have one custom border color sampler from above, so this should exceed max
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCreateInfo-None-04012");
        if (prop2.properties.limits.maxSamplerAllocationCount <= custom_properties.maxCustomBorderColorSamplers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCreateSampler-maxSamplerAllocationCount-04110");
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
    VkPhysicalDeviceCustomBorderColorFeaturesEXT border_color_features =
        LvlInitStruct<VkPhysicalDeviceCustomBorderColorFeaturesEXT>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&border_color_features);
    m_device_extension_names.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

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
    VkSamplerCustomBorderColorCreateInfoEXT custom_color_cinfo = LvlInitStruct<VkSamplerCustomBorderColorCreateInfoEXT>();
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
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[0].pImageInfo = &img_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
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
    if (!AddRequiredExtensions(ext_mem_extension_name)) {
        printf("%s External memory extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!CanEnableDeviceExtension(ext_mem_extension_name)) {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix, ext_mem_extension_name);
        return;
    }
    const bool bind_memory2 = CanEnableDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);

    // Create Export Image
    VkImage image_export = VK_NULL_HANDLE;
    VkExternalMemoryImageCreateInfo external_image_info = LvlInitStruct<VkExternalMemoryImageCreateInfo>();
    external_image_info.handleTypes = handle_type;
    VkImageCreateInfo image_info = LvlInitStruct<VkImageCreateInfo>(&external_image_info);
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.arrayLayers = 1;
    image_info.extent = {64, 64, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.mipLevels = 1;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkResult result = vk::CreateImage(device(), &image_info, NULL, &image_export);
    if (result != VK_SUCCESS) {
        printf("%s Unable to create a valid external image.\n", kSkipPrefix);
        return;
    }

    // Create export memory with different handleType
    VkExportMemoryAllocateInfo export_memory_info = LvlInitStruct<VkExportMemoryAllocateInfo>();
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT_KHR;

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&export_memory_info);

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

        VkBindImageMemoryInfo bind_image_info = LvlInitStruct<VkBindImageMemoryInfo>();
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
    if (!AddRequiredExtensions(ext_mem_extension_name)) {
        printf("%s External memory extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }
    AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!CanEnableDeviceExtension(ext_mem_extension_name)) {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix, ext_mem_extension_name);
        return;
    }

    const bool bind_memory2 = CanEnableDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties phys_mem_props;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &phys_mem_props);

    // Create Export Buffer
    VkBuffer buffer_export = VK_NULL_HANDLE;
    VkExternalMemoryBufferCreateInfo external_buffer_info = LvlInitStruct<VkExternalMemoryBufferCreateInfo>();
    external_buffer_info.handleTypes = handle_type;
    VkBufferCreateInfo buffer_info = LvlInitStruct<VkBufferCreateInfo>(&external_buffer_info);
    buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.size = 4096;
    VkResult result = vk::CreateBuffer(device(), &buffer_info, NULL, &buffer_export);
    if (result != VK_SUCCESS) {
        printf("%s Unable to create a valid external buffer.\n", kSkipPrefix);
        return;
    }

    // Create export memory with different handleType
    VkExportMemoryAllocateInfo export_memory_info = LvlInitStruct<VkExportMemoryAllocateInfo>();
    export_memory_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT_KHR;

    VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&export_memory_info);

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

        VkBindBufferMemoryInfo bind_buffer_info = LvlInitStruct<VkBindBufferMemoryInfo>();
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

TEST_F(VkLayerTest, UnnormalizedCoordinatesCombinedSampler) {
    TEST_DESCRIPTION(
        "If a samper is unnormalizedCoordinates, the imageview has to be some specific types. Uses COMBINED_IMAGE_SAMPLER");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_errorMonitor->ExpectSuccess();

    // This generates OpImage*Dref* instruction on R8G8B8A8_UNORM format.
    // Verify that it is allowed on this implementation if
    // VK_KHR_format_feature_flags2 is available.
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);

        if (!(fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR)) {
            printf("%s R8G8B8A8_UNORM does not support OpImage*Dref* operations, skipping.\n", kSkipPrefix);
            return;
        }
    }

    VkShaderObj vs(this, bindStateMinimalShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform sampler3D image_view_3d;
        layout (set = 0, binding = 1) uniform sampler2D tex[2];
        layout (set = 0, binding = 2) uniform sampler2DShadow tex_dep[2];
        void main() {
            // VUID 02702
            // 3D Image View is used with unnormalized coordinates
            // Also is VUID 02703 but the invalid image view is reported first
            vec4 x = texture(image_view_3d, vec3(0));

            // VUID 02703
            // OpImageSampleDrefImplicitLod is used with unnormalized coordinates
            float f = texture(tex_dep[0], vec3(0));

            // VUID 02704
            // OpImageSampleExplicitLod instructions that incudes a offset with unnormalized coordinates
            x = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());
    VkImageView view_pass = image.targetView(format);

    VkImageObj image_3d(m_device);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_3d.Init(image_ci);
    ASSERT_TRUE(image_3d.initialized());

    // If the sampler is unnormalizedCoordinates, the imageview type shouldn't be 3D, CUBE, 1D_ARRAY, 2D_ARRAY, CUBE_ARRAY.
    // This causes DesiredFailure.
    VkImageView view_fail = image_3d.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                                VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_3D);

    VkSampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    ASSERT_VK_SUCCESS(vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler));

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_fail, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, view_pass, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 2);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, view_pass, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 2);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02702");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02703");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02704");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, UnnormalizedCoordinatesSeparateSampler) {
    TEST_DESCRIPTION(
        "If a samper is unnormalizedCoordinates, the imageview has to be some specific types. Doesn't use COMBINED_IMAGE_SAMPLER");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    m_errorMonitor->ExpectSuccess();

    // This generates OpImage*Dref* instruction on R8G8B8A8_UNORM format.
    // Verify that it is allowed on this implementation if
    // VK_KHR_format_feature_flags2 is available.
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        auto fmt_props_3 = LvlInitStruct<VkFormatProperties3KHR>();
        auto fmt_props = LvlInitStruct<VkFormatProperties2>(&fmt_props_3);

        vk::GetPhysicalDeviceFormatProperties2(gpu(), VK_FORMAT_R8G8B8A8_UNORM, &fmt_props);

        if (!(fmt_props_3.optimalTilingFeatures & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT_KHR)) {
            printf("%s R8G8B8A8_UNORM does not support OpImage*Dref* operations, skipping.\n", kSkipPrefix);
            return;
        }
    }

    VkShaderObj vs(this, bindStateMinimalShaderText, VK_SHADER_STAGE_VERTEX_BIT);

    const char fsSource[] = R"glsl(
        #version 450
        // VK_DESCRIPTOR_TYPE_SAMPLER
        layout(set = 0, binding = 0) uniform sampler s1;
        layout(set = 0, binding = 1) uniform sampler s2;
        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        layout(set = 0, binding = 2) uniform texture2D si_good;
        layout(set = 0, binding = 3) uniform texture2D si_good_2;
        layout(set = 0, binding = 4) uniform texture3D si_bad[2]; // 3D image view

        void main() {
            // VUID 02702
            // 3D Image View is used with unnormalized coordinates
            // Also is VUID 02703 but the invalid image view is reported first
            vec4 x = texture(sampler3D(si_bad[1], s1), vec3(0));

            // VUID 02703
            // OpImageSampleDrefImplicitLod is used with unnormalized coordinates
            x = texture(sampler2D(si_good, s1), vec2(0));

            // VUID 02704
            // OpImageSampleExplicitLod instructions that incudes a offset with unnormalized coordinates
            x = textureLodOffset(sampler2D(si_good_2, s2), vec2(0), 0, ivec2(0));
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {3, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {4, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(128, 128, 1, 1, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    ASSERT_TRUE(image.initialized());
    VkImageView view_pass_a = image.targetView(format);
    VkImageView view_pass_b = image.targetView(format);

    VkImageObj image_3d(m_device);
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_3d.Init(image_ci);
    ASSERT_TRUE(image_3d.initialized());

    // If the sampler is unnormalizedCoordinates, the imageview type shouldn't be 3D, CUBE, 1D_ARRAY, 2D_ARRAY, CUBE_ARRAY.
    // This causes DesiredFailure.
    VkImageView view_fail = image_3d.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0,
                                                VK_REMAINING_ARRAY_LAYERS, VK_IMAGE_VIEW_TYPE_3D);

    // Need 2 samplers (and ImageView) because testing both VUID and it will tie both errors to the same sampler/imageView, but only
    // 02703 will be triggered since it's first in the validation code
    VkSampler sampler_a;
    VkSampler sampler_b;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    ASSERT_VK_SUCCESS(vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler_a));
    ASSERT_VK_SUCCESS(vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler_b));

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler_a, VK_DESCRIPTOR_TYPE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler_b, VK_DESCRIPTOR_TYPE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 1);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, view_pass_a, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, view_pass_b, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(4, view_fail, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 2);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02702");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02703");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02704");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CreateImageViewIncompatibleFormat) {
    TEST_DESCRIPTION("Tests for VUID-VkImageViewCreateInfo-image-01761");
    // original issue https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2203

    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    const auto ycbcr_support = CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    const auto maintenance2_support = CanEnableDeviceExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);

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

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

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

    VkImageViewCreateInfo imgViewInfo = LvlInitStruct<VkImageViewCreateInfo>();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgViewInfo.image = mutImage.handle();

    // The Image's format is non-planar and incompatible with the ImageView's format, which should trigger
    // VUID-VkImageViewCreateInfo-image-01761
    imgViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
    CreateImageViewTest(*this, &imgViewInfo, error_vuid);

    // With a identical format, there should be no error
    imgViewInfo.format = imageInfo.format;
    CreateImageViewTest(*this, &imgViewInfo, {});

    imageInfo.flags |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;
    VkImageObj mut_compat_image(m_device);
    mut_compat_image.init(&imageInfo);
    ASSERT_TRUE(mut_compat_image.initialized());

    imgViewInfo.image = mut_compat_image.handle();
    imgViewInfo.format = VK_FORMAT_R8_SINT;  // different, but size compatible
    CreateImageViewTest(*this, &imgViewInfo, {});
}

TEST_F(VkLayerTest, CreateImageViewIncompatibleDepthFormat) {
    TEST_DESCRIPTION("Tests for VUID-VkImageViewCreateInfo-image-01761 with depth format");

    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    if (!AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
        return;
    }

    VkPhysicalDeviceFeatures device_features = {};
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(GetPhysicalDeviceFeatures(&device_features));

    const auto maintenance2_support = CanEnableDeviceExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    const auto ycbcr_support = CanEnableDeviceExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);

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

    ASSERT_NO_FATAL_FAILURE(InitState(&device_features));

    const VkFormat depthOnlyFormat = FindSupportedDepthOnlyFormat(gpu());
    const VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    if ((depthOnlyFormat == VK_FORMAT_UNDEFINED) || (depthStencilFormat == VK_FORMAT_UNDEFINED)) {
        printf("%s requires a depth only and depth/stencil format.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   nullptr,
                                   VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                   VK_IMAGE_TYPE_2D,
                                   depthStencilFormat,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};

    VkImageObj mutImage(m_device);
    mutImage.init(&imageInfo);
    ASSERT_TRUE(mutImage.initialized());

    VkImageViewCreateInfo imgViewInfo = LvlInitStruct<VkImageViewCreateInfo>();
    imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imgViewInfo.subresourceRange.layerCount = 1;
    imgViewInfo.subresourceRange.baseMipLevel = 0;
    imgViewInfo.subresourceRange.levelCount = 1;
    imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imgViewInfo.image = mutImage.handle();
    // "Each depth/stencil format is only compatible with itself."
    imgViewInfo.format = depthOnlyFormat;
    CreateImageViewTest(*this, &imgViewInfo, error_vuid);
}

TEST_F(VkLayerTest, CreateImageViewMissingYcbcrConversion) {
    TEST_DESCRIPTION("Do not use VkSamplerYcbcrConversionInfo when required for an image view.");

    // Use 1.1 to get VK_KHR_sampler_ycbcr_conversion easier
    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s test requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        printf("samplerYcbcrConversion not supported, skipping test\n");
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    image.Init(128, 128, 1, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo view_info = LvlInitStruct<VkImageViewCreateInfo>();
    view_info.flags = 0;
    view_info.image = image.handle();
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    view_info.subresourceRange.layerCount = 1;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    CreateImageViewTest(*this, &view_info, "VUID-VkImageViewCreateInfo-format-06415");
}

TEST_F(VkLayerTest, InvalidShadingRateUsage) {
    TEST_DESCRIPTION("Specify invalid usage of the fragment shading rate image view usage.");
    VkPhysicalDeviceImagelessFramebufferFeatures if_features = LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeatures>();
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(&if_features);
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    bool retval = InitFrameworkAndRetrieveFeatures(features2);
    if (!retval) {
        printf("%s Error initializing extensions or retrieving features, skipping test\n", kSkipPrefix);
        return;
    }

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        printf("%s requires attachmentFragmentShadingRate feature.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto format =
        FindFormatWithoutFeatures(gpu(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    if (!format) {
        printf("%s No format found without shading rate attachment support. Skipped.\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    // Initialize image with transfer source usage
    image.Init(128, 128, 1, format, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView imageview;
    VkImageViewCreateInfo createinfo = LvlInitStruct<VkImageViewCreateInfo>();
    createinfo.image = image.handle();
    createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createinfo.format = format;
    createinfo.subresourceRange.layerCount = 1;
    createinfo.subresourceRange.baseMipLevel = 0;
    createinfo.subresourceRange.levelCount = 1;
    if (FormatIsColor(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    } else if (FormatHasDepth(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    } else if (FormatHasStencil(format)) {
        createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    // Create a view with the fragment shading rate attachment usage, but that doesn't support it
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-usage-04550");
    vk::CreateImageView(m_device->device(), &createinfo, NULL, &imageview);
    m_errorMonitor->VerifyFound();

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsrProperties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2 properties = LvlInitStruct<VkPhysicalDeviceProperties2>(&fsrProperties);

    vk::GetPhysicalDeviceProperties2(gpu(), &properties);

    if (!fsrProperties.layeredShadingRateAttachments) {
        if (IsPlatform(kMockICD) || DeviceSimulation()) {
            printf(
                "%s DevSim doesn't correctly advertise format support for fragment shading rate attachments, skipping some "
                "tests.\n",
                kSkipPrefix);
        } else {
            VkImageObj image2(m_device);
            image2.Init(VkImageObj::ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_R8_UINT,
                                                      VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                                                      VK_IMAGE_TILING_OPTIMAL));
            ASSERT_TRUE(image2.initialized());

            createinfo.image = image2.handle();
            createinfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            createinfo.format = VK_FORMAT_R8_UINT;
            createinfo.subresourceRange.layerCount = 2;
            createinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-usage-04551");
            vk::CreateImageView(m_device->device(), &createinfo, NULL, &imageview);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, InvalidImageFormatList) {
    TEST_DESCRIPTION("Tests for VK_KHR_image_format_list");

    if (!AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME)) {
        printf("%s: Missing required instance extensions\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s extension not supported, skipping test\n", kSkipPrefix, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    // Use sampled formats that will always be supported
    // Last format is not compatible with the rest
    const VkFormat formats[4] = {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8_UNORM};
    VkImageFormatListCreateInfo formatList = LvlInitStruct<VkImageFormatListCreateInfo>(nullptr);
    formatList.viewFormatCount = 4;
    formatList.pViewFormats = formats;

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   &formatList,
                                   VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_R8G8B8A8_UNORM,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};

    VkImage badImage = VK_NULL_HANDLE;
    VkImageObj mutableImage(m_device);
    VkImageObj mutableImageZero(m_device);
    VkImageObj normalImage(m_device);

    // Not all 4 formats are compatible
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-06722");
    vk::CreateImage(device(), &imageInfo, nullptr, &badImage);
    m_errorMonitor->VerifyFound();

    // Should work with only first 3 in array
    m_errorMonitor->ExpectSuccess();
    formatList.viewFormatCount = 3;
    mutableImage.init(&imageInfo);
    ASSERT_TRUE(mutableImage.initialized());
    m_errorMonitor->VerifyNotFound();

    // Make sure no error if 0 format
    m_errorMonitor->ExpectSuccess();
    formatList.viewFormatCount = 0;
    formatList.pViewFormats = &formats[3];  // non-compatible format
    mutableImageZero.init(&imageInfo);
    ASSERT_TRUE(mutableImageZero.initialized());
    m_errorMonitor->VerifyNotFound();
    // reset
    formatList.viewFormatCount = 3;
    formatList.pViewFormats = formats;

    // Can't use 2 or higher formats if no mutable flag
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-flags-04738");
    imageInfo.flags = 0;
    vk::CreateImage(device(), &imageInfo, nullptr, &badImage);
    m_errorMonitor->VerifyFound();

    // Make sure no error if 1 format
    m_errorMonitor->ExpectSuccess();
    formatList.viewFormatCount = 1;
    normalImage.init(&imageInfo);
    ASSERT_TRUE(normalImage.initialized());
    m_errorMonitor->VerifyNotFound();

    VkImageViewCreateInfo imageViewInfo = LvlInitStruct<VkImageViewCreateInfo>(nullptr);
    imageViewInfo.flags = 0;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.image = mutableImage.handle();

    // Not in format list
    imageViewInfo.format = VK_FORMAT_R8_SNORM;
    m_errorMonitor->SetUnexpectedError("VUID-VkImageViewCreateInfo-image-01018");
    CreateImageViewTest(*this, &imageViewInfo, "VUID-VkImageViewCreateInfo-pNext-01585");

    imageViewInfo.format = VK_FORMAT_R8G8B8A8_SNORM;
    CreateImageViewTest(*this, &imageViewInfo, {});

    // If viewFormatCount is zero should not hit VUID 01585
    imageViewInfo.image = mutableImageZero.handle();
    CreateImageViewTest(*this, &imageViewInfo, {});
}

TEST_F(VkLayerTest, InvalidImageFormatListSizeCompatible) {
    TEST_DESCRIPTION("Tests for VK_KHR_image_format_list with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT");

    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s required extensions not supported, skipping test\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if (!ImageFormatAndFeaturesSupported(gpu(), VK_FORMAT_ASTC_4x4_UNORM_BLOCK, VK_IMAGE_TILING_OPTIMAL,
                                         VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        printf("%s Required formats/features not supported, skipping test\n", kSkipPrefix);
        return;
    }

    const VkFormat formats[2] = {VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32_UINT};
    VkImageFormatListCreateInfo formatList = LvlInitStruct<VkImageFormatListCreateInfo>(nullptr);
    formatList.viewFormatCount = 1;
    formatList.pViewFormats = formats;

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   &formatList,
                                   VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT,
                                   VK_IMAGE_TYPE_2D,
                                   VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
                                   {128, 128, 1},
                                   1,
                                   1,
                                   VK_SAMPLE_COUNT_1_BIT,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_SHARING_MODE_EXCLUSIVE,
                                   0,
                                   nullptr,
                                   VK_IMAGE_LAYOUT_UNDEFINED};


    // The first image in the list should be size-compatible (128-bit)
    m_errorMonitor->ExpectSuccess();
    VkImageObj good_image(m_device);
    good_image.init(&imageInfo);
    m_errorMonitor->VerifyNotFound();

    // The second image in the list should NOT be size-compatible (64-bit)
    formatList.viewFormatCount = 2;
    VkImage badImage = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-pNext-06722");
    vk::CreateImage(device(), &imageInfo, nullptr, &badImage);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SparseMemoryBindOffset) {
    TEST_DESCRIPTION("Try to use VkSparseImageMemoryBind with offset not less than memory size");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.size = 1024;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = NULL;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (m_device->phy().features().sparseResidencyBuffer) {
        buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    } else {
        printf("%s Test requires unsupported sparseResidencyBuffer feature. Skipped.\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(nullptr);
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
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (m_device->phy().features().sparseResidencyImage2D) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    } else {
        printf("%s Test requires unsupported sparseResidencyImage2D feature. Skipped.\n", kSkipPrefix);
        return;
    }

    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_create_info);

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>(nullptr);
    mem_alloc.allocationSize = 1024;

    vk_testing::DeviceMemory mem;
    mem.init(*m_device, mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = mem_alloc.allocationSize;
    buffer_memory_bind.memory = mem.handle();
    buffer_memory_bind.memoryOffset = 2048;

    VkSparseImageMemoryBind image_memory_bind = {};
    image_memory_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_bind.memoryOffset = 4096;
    image_memory_bind.memory = mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseImageOpaqueMemoryBindInfo image_opaque_memory_bind_info = {};
    image_opaque_memory_bind_info.image = image.handle();
    image_opaque_memory_bind_info.bindCount = 1;
    image_opaque_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseImageMemoryBindInfo image_memory_bind_info = {};
    image_memory_bind_info.image = image.handle();
    image_memory_bind_info.bindCount = 1;
    image_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = LvlInitStruct<VkBindSparseInfo>();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.imageOpaqueBindCount = 1;
    bind_info.pImageOpaqueBinds = &image_opaque_memory_bind_info;
    bind_info.imageBindCount = 1;
    bind_info.pImageBinds = &image_memory_bind_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseMemoryBind-memoryOffset-01101");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseMemoryBind-memoryOffset-01101");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseMemoryBind-memoryOffset-01101");
    vk::QueueBindSparse(m_device->m_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    buffer_memory_bind.memoryOffset = 0;
    image_memory_bind.memoryOffset = 0;
    image_memory_bind.subresource.mipLevel = 1;
    image_memory_bind.subresource.arrayLayer = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseImageMemoryBindInfo-subresource-01722");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseImageMemoryBindInfo-subresource-01723");
    vk::QueueBindSparse(m_device->m_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidImageSplitInstanceBindRegionCount) {
    TEST_DESCRIPTION("Bind image memory with VkBindImageMemoryDeviceGroupInfo but invalid flags");

    if (!AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
        return;
    }
    if (!AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    const bool bind_memory_2_extension = CanEnableDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    const bool device_group_extension = CanEnableDeviceExtension(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);

    if (!bind_memory_2_extension) {
        printf("%s test requires VK_KHR_bind_memory2 extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    } else if (!device_group_extension) {
        printf("%s test requires VK_KHR_device_group extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;

    if (bind_memory_2_extension) {
        if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
            vkBindImageMemory2Function = vk::BindImageMemory2;
        } else {
            vkBindImageMemory2Function =
                (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
        }
    }

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(nullptr);
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
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    vk_testing::DeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image.handle(), &mem_reqs);
    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>(nullptr);
    mem_alloc.allocationSize = mem_reqs.size;

    for (int i = 0; i < 32; i++) {
        if (mem_reqs.memoryTypeBits & (1 << i)) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }

    image_mem.init(*m_device, mem_alloc);

    std::array<uint32_t, 2> deviceIndices = {{0, 0}};
    VkRect2D splitInstanceBindregion = {{0, 0}, {16, 16}};
    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = LvlInitStruct<VkBindImageMemoryDeviceGroupInfo>();
    bind_devicegroup_info.deviceIndexCount = 2;
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 1;
    bind_devicegroup_info.pSplitInstanceBindRegions = &splitInstanceBindregion;

    VkBindImageMemoryInfo bindInfo = LvlInitStruct<VkBindImageMemoryInfo>();
    bindInfo.pNext = &bind_devicegroup_info;
    bindInfo.image = image.handle();
    bindInfo.memory = image_mem.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryInfo-pNext-01627");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01633");
    vkBindImageMemory2Function(device(), 1, &bindInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidImageSplitInstanceBindRegionCountWithDeviceGroup) {
    TEST_DESCRIPTION("Bind image memory with VkBindImageMemoryDeviceGroupInfo but invalid splitInstanceBindRegionCount");

    if (!AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME) || !AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)) {
        printf("%s Missing required instance extensions\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    const bool bind_memory_2_extension = CanEnableDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
    const bool device_group_extension = CanEnableDeviceExtension(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);

    if (!bind_memory_2_extension) {
        printf("%s test requires VK_KHR_bind_memory2 extensions, not available.  Skipping.\n", kSkipPrefix);
        return;
    } else if (!device_group_extension) {
        printf("%s test requires VK_KHR_device_group extensions, not available.  Skipping.\n", kSkipPrefix);
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
        printf(
            "%s Test requires a physical device group with more than 1 device to use "
            "VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT.\n",
            kSkipPrefix);
        return;
    }

    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function = nullptr;

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        vkBindImageMemory2Function = vk::BindImageMemory2;
    } else {
        vkBindImageMemory2Function =
            (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");
    }

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(nullptr);
    image_create_info.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
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
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkDeviceMemory image_mem;
    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image.handle(), &mem_reqs);
    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>(nullptr);
    mem_alloc.allocationSize = mem_reqs.size;

    for (int i = 0; i < 32; i++) {
        if (mem_reqs.memoryTypeBits & (1 << i)) {
            mem_alloc.memoryTypeIndex = i;
            break;
        }
    }

    vk::AllocateMemory(device(), &mem_alloc, NULL, &image_mem);

    VkRect2D splitInstanceBindregion = {{0, 0}, {16, 16}};
    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = LvlInitStruct<VkBindImageMemoryDeviceGroupInfo>();
    bind_devicegroup_info.splitInstanceBindRegionCount = 2;
    bind_devicegroup_info.pSplitInstanceBindRegions = &splitInstanceBindregion;

    VkBindImageMemoryInfo bindInfo = LvlInitStruct<VkBindImageMemoryInfo>();
    bindInfo.pNext = &bind_devicegroup_info;
    bindInfo.image = image.handle();
    bindInfo.memory = image_mem;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-splitInstanceBindRegionCount-01636");
    vkBindImageMemory2Function(device(), 1, &bindInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDescriptorSetLayoutBindings) {
    TEST_DESCRIPTION("Create descriptor set layout with incompatible bindings.");

    if (!(CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names, m_device_extension_names, NULL,
                                                         m_errorMonitor))) {
        printf("Descriptor indexing or one of its dependencies not supported, skipping tests\n");
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (VK_FALSE == indexing_features.descriptorBindingUniformBufferUpdateAfterBind) {
        printf("%s Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind, skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorSetLayoutBinding update_binding = {};
    update_binding.binding = 0;
    update_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_binding.descriptorCount = 1;
    update_binding.stageFlags = VK_SHADER_STAGE_ALL;
    update_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding dynamic_binding = {};
    dynamic_binding.binding = 1;
    dynamic_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    dynamic_binding.descriptorCount = 1;
    dynamic_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dynamic_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[2] = {update_binding, dynamic_binding};

    VkDescriptorBindingFlags flags[2] = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, 0};

    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;

    VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    create_info.pNext = &flags_create_info;
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    create_info.bindingCount = 2;
    create_info.pBindings = bindings;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-03001");
    VkDescriptorSetLayout setLayout;
    vk::CreateDescriptorSetLayout(m_device->handle(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDescriptorSetLayoutBinding) {
    TEST_DESCRIPTION("Create invalid descriptor set layout.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
        return;
    }
    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mutable_descriptor_type_features);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (mutable_descriptor_type_features.mutableDescriptorType == VK_FALSE) {
        printf("%s mutableDescriptorType feature not supported. Skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkSampler sampler = VK_NULL_HANDLE;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vk::CreateSampler(m_device->device(), &sampler_info, NULL, &sampler);

    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = &sampler;

    VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER};

    VkMutableDescriptorTypeListVALVE mutable_descriptor_type_list = {};
    mutable_descriptor_type_list.descriptorTypeCount = 1;
    mutable_descriptor_type_list.pDescriptorTypes = descriptor_types;

    VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

    VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout setLayout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-descriptorType-04605");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04594");
    vk::CreateDescriptorSetLayout(m_device->handle(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DedicatedAllocationBufferWithInvalidFlags) {
    TEST_DESCRIPTION("Verify that flags are valid with VkDedicatedAllocationBufferCreateInfoNV");

    if (!AddRequiredExtensions(VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME)) {
        printf("%s Missing required instance extensions for %s\n", kSkipPrefix, VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME);
    }
    // Positive test to check parameter_validation and unique_objects support for NV_dedicated_allocation
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s VK_NV_DEDICATED_ALLOCATION_EXTENSION_NAME Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDedicatedAllocationBufferCreateInfoNV dedicated_buffer_create_info = LvlInitStruct<VkDedicatedAllocationBufferCreateInfoNV>();
    dedicated_buffer_create_info.dedicatedAllocation = VK_TRUE;

    uint32_t queue_family_index = 0;
    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.pNext = &dedicated_buffer_create_info;
    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &queue_family_index;

    VkBuffer buffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCreateInfo-pNext-01571");
    vk::CreateBuffer(m_device->device(), &buffer_create_info, NULL, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidMemoryAllocatepNextChain) {
    // Attempts to allocate from a memory type that doesn't exist

    if (!AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME)) {
        printf("%s Missing instance extensions required by %s\n", kSkipPrefix, VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    // Check for external memory device extensions
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s External memory extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);

    VkExportMemoryAllocateInfoNV export_memory_info_nv = LvlInitStruct<VkExportMemoryAllocateInfoNV>();
    export_memory_info_nv.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

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

TEST_F(VkLayerTest, BlockTexelViewInvalidLevelOrLayerCount) {
    TEST_DESCRIPTION(
        "Attempts to create an Image View with an image using VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT, but levelCount and "
        "layerCount are not 1.");

    if (!AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        printf("%s Missing instance extensions required for %s\n", kSkipPrefix, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE_2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 4;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!image.IsCompatible(image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        printf("%s Image usage and format not compatible on device\n", kSkipPrefix);
        return;
    }
    image.Init(image_create_info, 0);

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Test for error message
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.levelCount = 4;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01584");

    // Test for error message
    ivci.subresourceRange.layerCount = 2;
    ivci.subresourceRange.levelCount = 1;
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01584");
}

TEST_F(VkLayerTest, FillBufferCmdPoolUnsupported) {
    TEST_DESCRIPTION(
        "Use a command buffer with vkCmdFillBuffer that was allocated from a command pool that does not support graphics or "
        "compute opeartions");

    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t transfer = m_device->QueueFamilyWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    if (transfer == UINT32_MAX) {
        printf("%s Required queue families not present (non-graphics non-compute capable required).\n", kSkipPrefix);
        return;
    }
    VkQueueObj *queue = m_device->queue_family_queues(transfer)[0].get();

    VkCommandPoolObj pool(m_device, transfer, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj cb(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, queue);

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    VkBufferObj buffer;
    buffer.init_as_dst(*m_device, (VkDeviceSize)20, reqs);

    cb.begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdFillBuffer-commandBuffer-00030");
    cb.FillBuffer(buffer.handle(), 0, 12, 0x11111111);
    m_errorMonitor->VerifyFound();
    cb.end();
}

TEST_F(VkLayerTest, InvalidBindIMageMemoryDeviceGroupInfo) {
    TEST_DESCRIPTION("Checks for invalid BindIMageMemoryDeviceGroupInfo.");

    if (!AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME) || !AddRequiredExtensions(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)) {
        printf("%s Missing required instance extensions for %s and %s\n", kSkipPrefix, VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
               VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!CanEnableDeviceExtension(VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        printf("%s VK_KHR_DEVICE_GROUP_EXTENSION_NAME Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    if (!CanEnableDeviceExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME)) {
        printf("%s VK_KHR_BIND_MEMORY_2_EXTENSION_NAME Extension not supported, skipping test\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkBindImageMemory2KHR vkBindImageMemory2Function =
        (PFN_vkBindImageMemory2KHR)vk::GetDeviceProcAddr(m_device->handle(), "vkBindImageMemory2KHR");

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryRequirements mem_reqs;
    vk::GetImageMemoryRequirements(m_device->device(), image.handle(), &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = mem_reqs.memoryTypeBits;

    bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
    if (!pass) {
        printf("%s Failed to set memory type.\n", kSkipPrefix);
        return;
    }

    vk_testing::DeviceMemory memory;
    memory.init(*m_device, mem_alloc);

    uint32_t deviceIndex = 0;

    VkRect2D region = {};
    region.offset.x = 0;
    region.offset.y = 0;
    region.extent.width = image.width();
    region.extent.height = image.height();

    VkBindImageMemoryDeviceGroupInfo bimdgi = LvlInitStruct<VkBindImageMemoryDeviceGroupInfo>();
    bimdgi.deviceIndexCount = 1;
    bimdgi.pDeviceIndices = &deviceIndex;
    bimdgi.splitInstanceBindRegionCount = 1;
    bimdgi.pSplitInstanceBindRegions = &region;

    VkBindImageMemoryInfo bind_info = LvlInitStruct<VkBindImageMemoryInfo>(&bimdgi);
    bind_info.image = image.handle();
    bind_info.memory = memory.handle();
    bind_info.memoryOffset = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemoryDeviceGroupInfo-deviceIndexCount-01633");
    vkBindImageMemory2Function(m_device->device(), 1, &bind_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, BlockTexelViewInvalidType) {
    TEST_DESCRIPTION(
        "Create Image with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT and non-compressed format and ImageView with view type "
        "VK_IMAGE_VIEW_TYPE_3D.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!CanEnableDeviceExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE_2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!image.IsCompatible(image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        printf("%s Image usage and format not compatible on device\n", kSkipPrefix);
        return;
    }
    image.Init(image_create_info, 0);

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.levelCount = 1;

    // Test for error message
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-04739");
}

TEST_F(VkLayerTest, BlockTexelViewInvalidFormat) {
    TEST_DESCRIPTION("Create Image with VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT with non compatible formats.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!CanEnableDeviceExtension(VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        printf("%s Test requires API >= 1.1 or KHR_MAINTENANCE_2 extension, unavailable - skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;  // 64-bit block size
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!image.IsCompatible(image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        printf("%s Image usage and format not compatible on device\n", kSkipPrefix);
        return;
    }
    image.Init(image_create_info, 0);

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.levelCount = 1;

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;  // 32-bit block size
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01583");

    ivci.format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;  // 64-bit block size, but not same format class
    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewCreateInfo-image-01583");

    ivci.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;  // 64-bit block size, and same format class
    CreateImageViewTest(*this, &ivci);
}

TEST_F(VkLayerTest, InvalidImageSubresourceRangeAspectMask) {
    TEST_DESCRIPTION("Test creating Image with invalid VkImageSubresourceRange aspectMask.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s test requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }

    auto features11 = LvlInitStruct<VkPhysicalDeviceVulkan11Features>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&features11);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (features11.samplerYcbcrConversion != VK_TRUE) {
        printf("samplerYcbcrConversion not supported, skipping test\n");
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = mp_format;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageObj image(m_device);
    image.init(&image_create_info);
    ASSERT_TRUE(image.initialized());

    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
    ycbcr_create_info.format = mp_format;
    ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
    ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
    ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY};
    ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
    ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
    ycbcr_create_info.forceExplicitReconstruction = false;

    VkSamplerYcbcrConversion conversion;
    vk::CreateSamplerYcbcrConversion(m_device->device(), &ycbcr_create_info, nullptr, &conversion);

    VkSamplerYcbcrConversionInfo ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    ycbcr_info.conversion = conversion;

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>(&ycbcr_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = mp_format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT;

    CreateImageViewTest(*this, &ivci, "VUID-VkImageSubresourceRange-aspectMask-01670");
    vk::DestroySamplerYcbcrConversion(m_device->device(), conversion, nullptr);
}

TEST_F(VkLayerTest, InvalidCreateImageQueueFamilies) {
    TEST_DESCRIPTION("Checks for invalid queue families in ImageCreateInfo.");

    bool get_physical_device_properties2 = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (get_physical_device_properties2) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t queue_families[2] = {0, 0};

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
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
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.queueFamilyIndexCount = 2;
    image_create_info.pQueueFamilyIndices = queue_families;
    image_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;

    const char *vuid =
        (get_physical_device_properties2) ? "VUID-VkImageCreateInfo-sharingMode-01420" : "VUID-VkImageCreateInfo-sharingMode-01392";
    CreateImageTest(*this, &image_create_info, vuid);

    uint32_t queue_node_count;
    vk::GetPhysicalDeviceQueueFamilyProperties(gpu(), &queue_node_count, NULL);

    queue_families[1] = queue_node_count;
    CreateImageTest(*this, &image_create_info, vuid);
}

TEST_F(VkLayerTest, InvalidConditionalRenderingBufferUsage) {
    TEST_DESCRIPTION("Use a buffer without conditional rendering usage when needed.");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Did not find required device extension %s; test skipped.\n", kSkipPrefix,
               VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdBeginConditionalRenderingEXT vkCmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBeginConditionalRenderingEXT");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 1024;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = LvlInitStruct<VkConditionalRenderingBeginInfoEXT>();
    conditional_rendering_begin.buffer = buffer.handle();

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkConditionalRenderingBeginInfoEXT-buffer-01982");
    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, ImageFormatInfoDrmFormatModifier) {
    TEST_DESCRIPTION("Validate VkPhysicalDeviceImageFormatInfo2.");

    if (!AddRequiredExtensions(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME)) {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s not supported, skipping tests\n", kSkipPrefix, VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vk::GetInstanceProcAddr(
        instance(), "vkGetPhysicalDeviceImageFormatProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceImageFormatProperties2KHR != nullptr);

    VkPhysicalDeviceImageDrmFormatModifierInfoEXT image_drm_format_modifier =
        LvlInitStruct<VkPhysicalDeviceImageDrmFormatModifierInfoEXT>();

    VkPhysicalDeviceImageFormatInfo2 image_format_info =
        LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>(&image_drm_format_modifier);
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_format_info.flags = 0;

    VkImageFormatProperties2 image_format_properties = LvlInitStruct<VkImageFormatProperties2>();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vkGetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    image_format_info.pNext = nullptr;
    image_format_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02249");
    vkGetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();

    image_format_info.pNext = &image_drm_format_modifier;
    image_drm_format_modifier.sharingMode = VK_SHARING_MODE_CONCURRENT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageDrmFormatModifierInfoEXT-sharingMode-02315");
    vkGetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();
    image_drm_format_modifier.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImageFormatListCreateInfo format_list = LvlInitStruct<VkImageFormatListCreateInfo>(&image_drm_format_modifier);
    format_list.viewFormatCount = 0; // Invalid
    image_format_info.pNext = &format_list;
    image_format_info.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPhysicalDeviceImageFormatInfo2-tiling-02313");
    vkGetPhysicalDeviceImageFormatProperties2KHR(gpu(), &image_format_info, &image_format_properties);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidQueueBindSparseMemoryType) {
    TEST_DESCRIPTION("Test QueueBindSparse with lazily allocated memory");

    ASSERT_NO_FATAL_FAILURE(Init());

    if (!m_device->phy().features().sparseResidencyBuffer) {
        printf("%s Test requires unsupported sparseResidencyBuffer feature. Skipped.\n", kSkipPrefix);
        return;
    } else if (!m_device->phy().features().sparseResidencyImage2D) {
        printf("%s Test requires unsupported sparseResidencyImage2D feature. Skipped.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceMemoryProperties memory_info;
    vk::GetPhysicalDeviceMemoryProperties(gpu(), &memory_info);
    uint32_t lazily_allocated_index = memory_info.memoryTypeCount;  // Set to an invalid value just in case
    for (uint32_t i = 0; i < memory_info.memoryTypeCount; ++i) {
        if ((memory_info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0) {
            lazily_allocated_index = i;
            break;
        }
    }
    if (lazily_allocated_index == memory_info.memoryTypeCount) {
        printf("%s Did not find memory with VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT. Skipped.\n", kSkipPrefix);
        return;
    }

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.size = 1024;

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>(nullptr);
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
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

    VkBufferObj buffer;
    buffer.init_no_mem(*m_device, buffer_create_info);

    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_create_info);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc = vk_testing::DeviceMemory::alloc_info(buffer_mem_reqs.size, 0);
    buffer_mem_alloc.memoryTypeIndex = lazily_allocated_index;

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    VkMemoryAllocateInfo image_mem_alloc = vk_testing::DeviceMemory::alloc_info(image_mem_reqs.size, 0);
    image_mem_alloc.memoryTypeIndex = lazily_allocated_index;

    vk_testing::DeviceMemory buffer_mem;
    buffer_mem.init(*m_device, buffer_mem_alloc);

    vk_testing::DeviceMemory image_mem;
    image_mem.init(*m_device, image_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseMemoryBind image_memory_bind = {};
    image_memory_bind.size = image_mem_reqs.size;
    image_memory_bind.memory = image_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseImageOpaqueMemoryBindInfo image_opaque_memory_bind_info = {};
    image_opaque_memory_bind_info.image = image.handle();
    image_opaque_memory_bind_info.bindCount = 1;
    image_opaque_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = LvlInitStruct<VkBindSparseInfo>();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.imageOpaqueBindCount = 1;
    bind_info.pImageOpaqueBinds = &image_opaque_memory_bind_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseMemoryBind-memory-01097");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSparseMemoryBind-memory-01097");
    vk::QueueBindSparse(m_device->m_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidConditionalRenderingOffset) {
    TEST_DESCRIPTION("Begin conditional rendering with invalid offset.");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!DeviceExtensionSupported(gpu(), nullptr, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME)) {
        printf("%s Did not find required device extension %s; test skipped.\n", kSkipPrefix, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdBeginConditionalRenderingEXT vkCmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBeginConditionalRenderingEXT");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 128;
    buffer_create_info.usage = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = LvlInitStruct<VkConditionalRenderingBeginInfoEXT>();
    conditional_rendering_begin.buffer = buffer.handle();
    conditional_rendering_begin.offset = 3;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkConditionalRenderingBeginInfoEXT-offset-01984");
    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_errorMonitor->VerifyFound();

    conditional_rendering_begin.offset = buffer_create_info.size;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkConditionalRenderingBeginInfoEXT-offset-01983");
    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidBeginConditionalRendering) {
    TEST_DESCRIPTION("Begin conditional rendering when it is already active.");

    AddRequiredExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Did not find required device extension %s; test skipped.\n", kSkipPrefix,
               VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkCmdBeginConditionalRenderingEXT vkCmdBeginConditionalRenderingEXT =
        (PFN_vkCmdBeginConditionalRenderingEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdBeginConditionalRenderingEXT");
    PFN_vkCmdEndConditionalRenderingEXT vkCmdEndConditionalRenderingEXT =
        (PFN_vkCmdEndConditionalRenderingEXT)vk::GetDeviceProcAddr(m_device->handle(), "vkCmdEndConditionalRenderingEXT");

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_create_info);

    VkConditionalRenderingBeginInfoEXT conditional_rendering_begin = LvlInitStruct<VkConditionalRenderingBeginInfoEXT>();
    conditional_rendering_begin.buffer = buffer.handle();

    m_commandBuffer->begin();
    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginConditionalRenderingEXT-None-01980");
    vkCmdBeginConditionalRenderingEXT(m_commandBuffer->handle(), &conditional_rendering_begin);
    m_errorMonitor->VerifyFound();
    vkCmdEndConditionalRenderingEXT(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidMultiSampleImageView) {
    TEST_DESCRIPTION("Begin conditional rendering when it is already active.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;
    if ((dev_limits.sampledImageColorSampleCounts & VK_SAMPLE_COUNT_2_BIT) == 0) {
        printf("%s Required VkSampleCountFlagBits are not supported; skipping\n", kSkipPrefix);
        return;
    }

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties image_format_properties;
    vk::GetPhysicalDeviceImageFormatProperties(m_device->phy().handle(), image_create_info.format, image_create_info.imageType,
                                               image_create_info.tiling, image_create_info.usage, image_create_info.flags,
                                               &image_format_properties);

    if (image_format_properties.sampleCounts < 2) {
        printf("%s Required VkSampleCountFlagBits for image format are not supported; skipping\n", kSkipPrefix);
        return;
    }

    VkImageObj image(m_device);
    image.init(&image_create_info);

    VkImageViewCreateInfo dsvci = LvlInitStruct<VkImageViewCreateInfo>();
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_1D;
    dsvci.format = VK_FORMAT_R8G8B8A8_UNORM;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView imageView;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-04972");
    vk::CreateImageView(m_device->device(), &dsvci, nullptr, &imageView);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestBindingDescriptorSetFromHostOnlyPool) {
    TEST_DESCRIPTION("Try to bind a descriptor set that was allocated from a pool with VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool pool;
    pool.init(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkDescriptorSetAllocateInfo allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &ds_layout_handle;

    VkDescriptorSet descriptor_set;
    vk::AllocateDescriptorSets(device(), &allocate_info, &descriptor_set);

    VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout});

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-04616");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &descriptor_set, 0, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, CopyMutableDescriptors) {
    TEST_DESCRIPTION("Copy mutable descriptors.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
        return;
    }
    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mutable_descriptor_type_features);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (mutable_descriptor_type_features.mutableDescriptorType == VK_FALSE) {
        printf("%s mutableDescriptorType feature not supported. Skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    {
        VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER};

        VkMutableDescriptorTypeListVALVE mutable_descriptor_type_list = {};
        mutable_descriptor_type_list.descriptorTypeCount = 1;
        mutable_descriptor_type_list.pDescriptorTypes = descriptor_types;

        VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
        mdtci.mutableDescriptorTypeListCount = 1;
        mdtci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

        VkDescriptorPoolSize pool_sizes[2] = {};
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[0].descriptorCount = 2;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        pool_sizes[1].descriptorCount = 2;

        VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>(&mdtci);
        ds_pool_ci.maxSets = 2;
        ds_pool_ci.poolSizeCount = 2;
        ds_pool_ci.pPoolSizes = pool_sizes;

        vk_testing::DescriptorPool pool;
        pool.init(*m_device, ds_pool_ci);

        VkDescriptorSetLayoutBinding bindings[2] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[0].pImmutableSamplers = nullptr;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        vk_testing::DescriptorSetLayout set_layout;
        set_layout.init(*m_device, create_info);
        VkDescriptorSetLayout set_layout_handle = set_layout.handle();

        VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

        VkDescriptorSetAllocateInfo allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
        allocate_info.descriptorPool = pool.handle();
        allocate_info.descriptorSetCount = 2;
        allocate_info.pSetLayouts = layouts;

        VkDescriptorSet descriptor_sets[2];
        vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

        VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
        buffer_ci.size = 32;
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VkBufferObj buffer;
        buffer.init(*m_device, buffer_ci);

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = buffer_ci.size;

        VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_sets[0];
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

        VkCopyDescriptorSet copy_set = LvlInitStruct<VkCopyDescriptorSet>();
        copy_set.srcSet = descriptor_sets[1];
        copy_set.srcBinding = 1;
        copy_set.dstSet = descriptor_sets[0];
        copy_set.dstBinding = 0;
        copy_set.descriptorCount = 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyDescriptorSet-dstSet-04612");
        vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_set);
        m_errorMonitor->VerifyFound();
    }
    {
        VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

        VkMutableDescriptorTypeListVALVE mutable_descriptor_type_lists[2] = {};
        mutable_descriptor_type_lists[0].descriptorTypeCount = 1;
        mutable_descriptor_type_lists[0].pDescriptorTypes = descriptor_types;
        mutable_descriptor_type_lists[1].descriptorTypeCount = 2;
        mutable_descriptor_type_lists[1].pDescriptorTypes = descriptor_types;

        VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
        mdtci.mutableDescriptorTypeListCount = 2;
        mdtci.pMutableDescriptorTypeLists = mutable_descriptor_type_lists;

        VkDescriptorPoolSize pool_size = {};
        pool_size.type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        pool_size.descriptorCount = 4;

        VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>(&mdtci);
        ds_pool_ci.maxSets = 2;
        ds_pool_ci.poolSizeCount = 1;
        ds_pool_ci.pPoolSizes = &pool_size;

        vk_testing::DescriptorPool pool;
        pool.init(*m_device, ds_pool_ci);

        VkDescriptorSetLayoutBinding bindings[2] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[0].pImmutableSamplers = nullptr;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        vk_testing::DescriptorSetLayout set_layout;
        set_layout.init(*m_device, create_info);
        VkDescriptorSetLayout set_layout_handle = set_layout.handle();

        VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

        VkDescriptorSetAllocateInfo allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
        allocate_info.descriptorPool = pool.handle();
        allocate_info.descriptorSetCount = 2;
        allocate_info.pSetLayouts = layouts;

        VkDescriptorSet descriptor_sets[2];
        vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

        VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
        buffer_ci.size = 32;
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VkBufferObj buffer;
        buffer.init(*m_device, buffer_ci);

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = buffer_ci.size;

        VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_sets[0];
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

        VkCopyDescriptorSet copy_set = LvlInitStruct<VkCopyDescriptorSet>();
        copy_set.srcSet = descriptor_sets[1];
        copy_set.srcBinding = 1;
        copy_set.dstSet = descriptor_sets[0];
        copy_set.dstBinding = 0;
        copy_set.descriptorCount = 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyDescriptorSet-dstSet-04614");
        vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_set);
        m_errorMonitor->VerifyFound();
    }
    {
        VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

        VkMutableDescriptorTypeListVALVE mutable_descriptor_type_lists[2] = {};
        mutable_descriptor_type_lists[0].descriptorTypeCount = 2;
        mutable_descriptor_type_lists[0].pDescriptorTypes = descriptor_types;
        mutable_descriptor_type_lists[1].descriptorTypeCount = 0;
        mutable_descriptor_type_lists[1].pDescriptorTypes = nullptr;

        VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
        mdtci.mutableDescriptorTypeListCount = 2;
        mdtci.pMutableDescriptorTypeLists = mutable_descriptor_type_lists;

        VkDescriptorPoolSize pool_sizes[2] = {};
        pool_sizes[0].type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        pool_sizes[0].descriptorCount = 2;
        pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount = 2;

        VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>(&mdtci);
        ds_pool_ci.maxSets = 2;
        ds_pool_ci.poolSizeCount = 2;
        ds_pool_ci.pPoolSizes = pool_sizes;

        vk_testing::DescriptorPool pool;
        pool.init(*m_device, ds_pool_ci);

        VkDescriptorSetLayoutBinding bindings[2] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[0].pImmutableSamplers = nullptr;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        vk_testing::DescriptorSetLayout set_layout;
        set_layout.init(*m_device, create_info);
        VkDescriptorSetLayout set_layout_handle = set_layout.handle();

        VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

        VkDescriptorSetAllocateInfo allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
        allocate_info.descriptorPool = pool.handle();
        allocate_info.descriptorSetCount = 2;
        allocate_info.pSetLayouts = layouts;

        VkDescriptorSet descriptor_sets[2];
        vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

        VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
        buffer_ci.size = 32;
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        VkBufferObj buffer;
        buffer.init(*m_device, buffer_ci);

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = buffer.handle();
        buffer_info.offset = 0;
        buffer_info.range = buffer_ci.size;

        VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_sets[0];
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = &buffer_info;

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

        VkCopyDescriptorSet copy_set = LvlInitStruct<VkCopyDescriptorSet>();
        copy_set.srcSet = descriptor_sets[0];
        copy_set.srcBinding = 0;
        copy_set.dstSet = descriptor_sets[1];
        copy_set.dstBinding = 1;
        copy_set.descriptorCount = 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyDescriptorSet-srcSet-04613");
        vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_set);
        m_errorMonitor->VerifyFound();
    }
}


TEST_F(VkLayerTest, ValidateUpdatingMutableDescriptors) {
    TEST_DESCRIPTION("Validate updating mutable descriptors.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
        return;
    }
    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mutable_descriptor_type_features);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (mutable_descriptor_type_features.mutableDescriptorType == VK_FALSE) {
        printf("%s mutableDescriptorType feature not supported. Skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

    VkMutableDescriptorTypeListVALVE mutable_descriptor_type_list = {};
    mutable_descriptor_type_list.descriptorTypeCount = 1;
    mutable_descriptor_type_list.pDescriptorTypes = descriptor_types;

    VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

    VkDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[0].descriptorCount = 2;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    pool_sizes[1].descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>(&mdtci);
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 2;
    ds_pool_ci.pPoolSizes = pool_sizes;

    vk_testing::DescriptorPool pool;
    pool.init(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[0].pImmutableSamplers = nullptr;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_ALL;
    bindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
    create_info.bindingCount = 2;
    create_info.pBindings = bindings;

    vk_testing::DescriptorSetLayout set_layout;
    set_layout.init(*m_device, create_info);
    VkDescriptorSetLayout set_layout_handle = set_layout.handle();

    VkDescriptorSetLayout layouts[2] = {set_layout_handle, set_layout_handle};

    VkDescriptorSetAllocateInfo allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 2;
    allocate_info.pSetLayouts = layouts;

    VkDescriptorSet descriptor_sets[2];
    vk::AllocateDescriptorSets(device(), &allocate_info, descriptor_sets);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vk::CreateSampler(device(), &sampler_ci, nullptr, &sampler);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.sampler = sampler;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_sets[1];
    descriptor_write.dstBinding = 1;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    VkCopyDescriptorSet copy_set = LvlInitStruct<VkCopyDescriptorSet>();
    copy_set.srcSet = descriptor_sets[1];
    copy_set.srcBinding = 1;
    copy_set.dstSet = descriptor_sets[0];
    copy_set.dstBinding = 0;
    copy_set.descriptorCount = 1;

    vk::DestroySampler(device(), sampler, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00325");
    vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Image2DViewOf3D) {
    TEST_DESCRIPTION("Checks for invalid use of 2D views of 3D images");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Vulkan12Struct requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
        return;
    }

    auto image_2D_view_of_3D_features = LvlInitStruct<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&image_2D_view_of_3D_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!image_2D_view_of_3D_features.image2DViewOf3D){
        printf("%s image2DViewOf3D is not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    OneOffDescriptorSet descriptor_set(m_device,
                                       { {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr} }
                                       );

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent = {64, 64, 4};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

    VkImageObj image_3d(m_device);
    image_3d.init(&image_ci);
    ASSERT_TRUE(image_3d.initialized());
    VkImageViewCreateInfo view_ci = LvlInitStruct<VkImageViewCreateInfo>();
    view_ci.image = image_3d.handle();
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    view_ci.subresourceRange.layerCount = 1;
    view_ci.subresourceRange.baseMipLevel = 0;
    view_ci.subresourceRange.levelCount = 1;
    view_ci.subresourceRange.baseArrayLayer = 0;
    view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_testing::ImageView view_2d_array;
    view_2d_array.init(*m_device, view_ci);

    descriptor_set.WriteDescriptorImageInfo(0, view_2d_array.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-imageView-06712");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    descriptor_set.descriptor_writes.clear();

    vk_testing::ImageView view_2d;
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_2d.init(*m_device, view_ci);
    descriptor_set.WriteDescriptorImageInfo(0, view_2d.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-06710");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    descriptor_set.descriptor_writes.clear();
    
    image_ci.flags = 0;
    VkImageObj image_3d_no_flag(m_device);
    image_3d_no_flag.init(&image_ci);
    VkImageView view;
    view_ci.image = image_3d_no_flag.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-06728");
    vk::CreateImageView(m_device->device(), &view_ci, nullptr, &view);
    m_errorMonitor->VerifyFound();

    const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
    view_ci.subresourceRange = range;
    view_ci.image = image_3d_no_flag.handle();
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-06723");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-image-06724");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCreateInfo-subresourceRange-06725");
    vk::CreateImageView(m_device->device(), &view_ci, nullptr, &view);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, Image2DViewOf3DFeature) {
    TEST_DESCRIPTION("Checks for image image_2d_view_of_3d features");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("%s Vulkan12Struct requires Vulkan 1.1+, skipping test\n", kSkipPrefix);
        return;
    }
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
        return;
    }

    auto image_2D_view_of_3D_features = LvlInitStruct<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&image_2D_view_of_3D_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    image_2D_view_of_3D_features.image2DViewOf3D = VK_FALSE;
    image_2D_view_of_3D_features.sampler2DViewOf3D = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}});

    VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent = {64, 64, 4};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.flags = VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;

    VkImageObj image_3d(m_device);
    image_3d.init(&image_ci);
    ASSERT_TRUE(image_3d.initialized());
    VkImageViewCreateInfo view_ci = LvlInitStruct<VkImageViewCreateInfo>();
    view_ci.image = image_3d.handle();
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    view_ci.subresourceRange.layerCount = 1;
    view_ci.subresourceRange.baseMipLevel = 0;
    view_ci.subresourceRange.levelCount = 1;
    view_ci.subresourceRange.baseArrayLayer = 0;
    view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_testing::ImageView view_2d_array;
    view_2d_array.init(*m_device, view_ci);

    descriptor_set.WriteDescriptorImageInfo(0, view_2d_array.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-descriptorType-06714");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    descriptor_set.descriptor_writes.clear();

    descriptor_set.WriteDescriptorImageInfo(1, view_2d_array.handle(), VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                            VK_IMAGE_LAYOUT_GENERAL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-descriptorType-06713");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImageViewMinLod) {
    TEST_DESCRIPTION("Checks for image view minimum level of detail.");

    AddRequiredExtensions(VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME);
        return;
    }
    auto image_view_min_lod_features = LvlInitStruct<VkPhysicalDeviceImageViewMinLodFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&image_view_min_lod_features);
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (image_view_min_lod_features.minLod == VK_FALSE) {
        printf("%s image view min lod feature not supported. Skipped.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 4;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    VkImageObj image2D(m_device);
    image2D.init(&image_create_info);
    ASSERT_TRUE(image2D.initialized());

    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image2D.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 4;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    auto ivml = LvlInitStruct<VkImageViewMinLodCreateInfoEXT>();
    ivml.minLod = 4.0;
    ivci.pNext = &ivml;

    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewMinLodCreateInfoEXT-minLod-06456");
    VkImageView image_view = {};
    ivml.minLod = 1.0;
    VkResult res = vk::CreateImageView(m_device->device(), &ivci, nullptr, &image_view);
    ASSERT_TRUE(res == VK_SUCCESS);

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-06450");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vk::DestroyImageView(m_device->device(), image_view, NULL);
}

TEST_F(VkLayerTest, ImageViewMinLodFeature) {
    TEST_DESCRIPTION("Checks for image view minimum level of detail feature enabled.");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkImageObj image(m_device);
    // Initialize image with transfer source usage
    image.Init(128, 128, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 2;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    auto ivml = LvlInitStruct<VkImageViewMinLodCreateInfoEXT>();
    ivml.minLod = 1.0;
    ivci.pNext = &ivml;

    CreateImageViewTest(*this, &ivci, "VUID-VkImageViewMinLodCreateInfoEXT-minLod-06455");
}

TEST_F(VkLayerTest, ValidateDeviceImageMemoryRequirements) {
    TEST_DESCRIPTION("Validate usage of VkDeviceImageMemoryRequirementsKHR.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequestedExtensionsEnabled()) {
        printf("%s Extension %s is not supported, skipping test.\n", kSkipPrefix, VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
        return;
    }

    PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirementsKHR =
        reinterpret_cast<PFN_vkGetDeviceImageMemoryRequirementsKHR>(vk::GetInstanceProcAddr(instance(), "vkGetDeviceImageMemoryRequirementsKHR"));
    assert(vkGetDeviceImageMemoryRequirementsKHR != nullptr);

    auto image_swapchain_create_info = LvlInitStruct<VkImageSwapchainCreateInfoKHR>();
    image_swapchain_create_info.swapchain = m_swapchain;

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>(&image_swapchain_create_info);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
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

TEST_F(VkLayerTest, TestCompletelyOverlappingBufferCopy) {
    TEST_DESCRIPTION("Test copying between buffers with completely overlapping source and destination regions.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkBufferCopy copy_info;
    copy_info.srcOffset = 0;
    copy_info.dstOffset = 0;
    copy_info.size = 256;

    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src_and_dst(*m_device, copy_info.size, reqs);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-pRegions-00117");

    m_commandBuffer->begin();
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer.handle(), buffer.handle(), 1, &copy_info);
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestCopyingInterleavedRegions) {
    TEST_DESCRIPTION("Test copying between interleaved source and destination regions.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkBufferCopy copy_infos[4];
    copy_infos[0].srcOffset = 0;
    copy_infos[0].dstOffset = 4;
    copy_infos[0].size = 4;
    copy_infos[1].srcOffset = 8;
    copy_infos[1].dstOffset = 12;
    copy_infos[1].size = 4;
    copy_infos[2].srcOffset = 16;
    copy_infos[2].dstOffset = 20;
    copy_infos[2].size = 4;
    copy_infos[3].srcOffset = 24;
    copy_infos[3].dstOffset = 28;
    copy_infos[3].size = 4;

    VkBufferObj buffer;
    VkMemoryPropertyFlags reqs = 0;
    buffer.init_as_src_and_dst(*m_device, 32, reqs);


    m_commandBuffer->begin();

    m_errorMonitor->ExpectSuccess();
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer.handle(), buffer.handle(), 4, copy_infos);
    m_errorMonitor->VerifyNotFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdCopyBuffer-pRegions-00117");
    copy_infos[2].dstOffset = 21;
    vk::CmdCopyBuffer(m_commandBuffer->handle(), buffer.handle(), buffer.handle(), 4, copy_infos);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();

}
