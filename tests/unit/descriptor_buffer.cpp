/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_objects.h"

TEST_F(NegativeDescriptorBuffer, SetLayout) {
    TEST_DESCRIPTION("Descriptor buffer set layout tests.");
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08000");
        vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08000");
        vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08001");
        vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags =
            VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08002");
        vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        VkSampler samplers[2] = {sampler.handle(), sampler.handle()};
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, samplers};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-flags-08005");
        vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        VkSampler samplers[2] = {sampler.handle(), sampler.handle()};
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, samplers};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-flags-08006");
        vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }
    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        VkDescriptorSetLayout dsl;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-flags-08007");
        vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
        m_errorMonitor->VerifyFound();
    }

    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        const auto dslci1 = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        vk_testing::DescriptorSetLayout dsl1(*m_device, dslci1);

        const auto dslci2 = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, 0U, 1U, &binding);
        vk_testing::DescriptorSetLayout dsl2(*m_device, dslci2);

        VkPipelineLayout pipeline_layout;
        const std::array<VkDescriptorSetLayout, 2> set_layouts{dsl1.handle(), dsl2.handle()};
        auto plci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        plci.setLayoutCount = size32(set_layouts);
        plci.pSetLayouts = set_layouts.data();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-08008");
        vk::CreatePipelineLayout(device(), &plci, NULL, &pipeline_layout);
        m_errorMonitor->VerifyFound();
    }

    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
        const auto dslci1 = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
        vk_testing::DescriptorSetLayout dsl1(*m_device, dslci1);

        VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
        const auto dspci =
            LvlInitStruct<VkDescriptorPoolCreateInfo>(nullptr, static_cast<VkDescriptorPoolCreateFlags>(0), 1U, 1U, &pool_size);
        vk_testing::DescriptorPool pool(*m_device, dspci);

        VkDescriptorSet ds = VK_NULL_HANDLE;
        const auto alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>(nullptr, pool.handle(), 1U, &dsl1.handle());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-08009");
        vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &ds);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorBuffer, SetLayoutInlineUniformBlockEXT) {
    TEST_DESCRIPTION("Descriptor buffer set layout tests.");
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    auto inline_uniform_features = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>();
    InitBasicDescriptorBuffer(&inline_uniform_features);
    if (::testing::Test::IsSkipped()) return;

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    auto inlineUniformProps = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockPropertiesEXT>();
    GetPhysicalDeviceProperties2(inlineUniformProps);

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT,
                                               inlineUniformProps.maxInlineUniformBlockSize + 4, VK_SHADER_STAGE_FRAGMENT_BIT,
                                               nullptr};
    const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, 0U, 1U, &binding);
    VkDescriptorSetLayout dsl;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-descriptorType-08004");
    vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, SetLayoutMutableDescriptorEXT) {
    TEST_DESCRIPTION("Descriptor buffer set layout tests.");
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    auto mutable_descriptor_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>();
    InitBasicDescriptorBuffer(&mutable_descriptor_features);
    if (::testing::Test::IsSkipped()) return;

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutCreateFlags flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE;
    const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
    VkDescriptorSetLayout dsl;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-08003");
    vk::CreateDescriptorSetLayout(m_device->device(), &dslci, nullptr, &dsl);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, NotEnabled) {
    TEST_DESCRIPTION("Tests for when descriptor buffer is not enabled");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    auto acceleration_structure_features =
        LvlInitStruct<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(&buffer_device_address_features);
    GetPhysicalDeviceFeatures2(acceleration_structure_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &acceleration_structure_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()};
    const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                   VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
    const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1U, &binding);
    vk_testing::DescriptorSetLayout dsl(*m_device, dslci);

    auto plci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &dsl.handle();
    vk_testing::PipelineLayout pipeline_layout(*m_device, plci);

    {
        VkDeviceSize size;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDescriptorSetLayoutSizeEXT-None-08011");
        vk::GetDescriptorSetLayoutSizeEXT(m_device->device(), dsl.handle(), &size);
        m_errorMonitor->VerifyFound();
    }

    {
        VkDeviceSize offset;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-None-08013");
        vk::GetDescriptorSetLayoutBindingOffsetEXT(m_device->device(), dsl.handle(), 0, &offset);
        m_errorMonitor->VerifyFound();
    }

    {
        uint8_t buffer[128];
        auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();
        dgi.type = VK_DESCRIPTOR_TYPE_SAMPLER;
        dgi.data.pSampler = &sampler.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDescriptorEXT-None-08015");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.samplerDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();
    }

    {
        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-None-08068");
        vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layout.handle(), 0);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }

    {
        uint32_t data[128];
        const auto ocddci = LvlInitStruct<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(nullptr, &data);

        {
            uint32_t qfi = 0;
            auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
            buffCI.size = 4096;
            buffCI.flags = VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            buffCI.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
            buffCI.queueFamilyIndexCount = 1;
            buffCI.pQueueFamilyIndices = &qfi;

            buffCI.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
            CreateBufferTest(*this, &buffCI, "VUID-VkBufferCreateInfo-flags-08099");

            buffCI.flags = 0;

            if (descriptor_buffer_properties.bufferlessPushDescriptors) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCreateInfo-usage-08102");
            }
            buffCI.usage =
                VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
            CreateBufferTest(*this, &buffCI, "VUID-VkBufferCreateInfo-usage-08101");

            buffCI.pNext = &ocddci;
            buffCI.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
            CreateBufferTest(*this, &buffCI, "VUID-VkBufferCreateInfo-pNext-08100");
        }

        {
            auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
            image_create_info.flags |= VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.extent.width = 128;
            image_create_info.extent.height = 128;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.format = VK_FORMAT_D32_SFLOAT;
            image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            image_create_info.pNext = &ocddci;
            CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-08104");

            image_create_info.pNext = &ocddci;
            image_create_info.flags &= ~VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-pNext-08105");
        }

        {
            auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.extent.width = 128;
            image_create_info.extent.height = 128;
            image_create_info.extent.depth = 1;
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_create_info.format = VK_FORMAT_D32_SFLOAT;
            image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            vk_testing::Image temp_image(*m_device, image_create_info);

            auto dsvci = LvlInitStruct<VkImageViewCreateInfo>();
            dsvci.flags |= VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            dsvci.image = temp_image.handle();
            dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            dsvci.format = VK_FORMAT_D32_SFLOAT;
            dsvci.subresourceRange.layerCount = 1;
            dsvci.subresourceRange.baseMipLevel = 0;
            dsvci.subresourceRange.levelCount = 1;
            dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            CreateImageViewTest(*this, &dsvci, "VUID-VkImageViewCreateInfo-flags-08106");

            dsvci.pNext = &ocddci;
            dsvci.flags &= ~VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            CreateImageViewTest(*this, &dsvci, "VUID-VkImageViewCreateInfo-pNext-08107");
        }

        if (IsExtensionsEnabled(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)) {
            uint32_t qfi = 0;
            auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
            buffCI.size = 4096;
            buffCI.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
            buffCI.queueFamilyIndexCount = 1;
            buffCI.pQueueFamilyIndices = &qfi;

            vk_testing::Buffer as_buffer(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            VkAccelerationStructureKHR as;
            auto asci = LvlInitStruct<VkAccelerationStructureCreateInfoKHR>();
            asci.createFlags = VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            asci.buffer = as_buffer.handle();

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoKHR-createFlags-08108");
            vk::CreateAccelerationStructureKHR(m_device->device(), &asci, NULL, &as);
            m_errorMonitor->VerifyFound();

            asci.pNext = &ocddci;
            asci.createFlags &= ~VK_ACCELERATION_STRUCTURE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAccelerationStructureCreateInfoKHR-pNext-08109");
            vk::CreateAccelerationStructureKHR(m_device->device(), &asci, NULL, &as);
            m_errorMonitor->VerifyFound();
        }

        {
            auto sampler_ci = SafeSaneSamplerCreateInfo();
            sampler_ci.flags |= VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;

            CreateSamplerTest(*this, &sampler_ci, "VUID-VkSamplerCreateInfo-flags-08110");

            sampler_ci.pNext = &ocddci;
            sampler_ci.flags &= ~VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
            CreateSamplerTest(*this, &sampler_ci, "VUID-VkSamplerCreateInfo-pNext-08111");
        }
    }

    {
        uint8_t data[256];

        uint32_t qfi = 0;
        auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
        buffCI.size = 4096;
        buffCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffCI.queueFamilyIndexCount = 1;
        buffCI.pQueueFamilyIndices = &qfi;

        vk_testing::Buffer temp_buffer(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        auto bcddi = LvlInitStruct<VkBufferCaptureDescriptorDataInfoEXT>();
        bcddi.buffer = temp_buffer.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetBufferOpaqueCaptureDescriptorDataEXT-None-08072");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCaptureDescriptorDataInfoEXT-buffer-08075");
        vk::GetBufferOpaqueCaptureDescriptorDataEXT(m_device->device(), &bcddi, &data);
        m_errorMonitor->VerifyFound();
    }

    {
        uint8_t data[256];

        auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = 128;
        image_create_info.extent.height = 128;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.format = VK_FORMAT_D32_SFLOAT;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        vk_testing::Image temp_image;
        temp_image.init(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        auto icddi = LvlInitStruct<VkImageCaptureDescriptorDataInfoEXT>();
        icddi.image = temp_image.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageOpaqueCaptureDescriptorDataEXT-None-08076");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCaptureDescriptorDataInfoEXT-image-08079");
        vk::GetImageOpaqueCaptureDescriptorDataEXT(m_device->device(), &icddi, &data);
        m_errorMonitor->VerifyFound();
    }

    {
        uint8_t data[256];

        auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = 128;
        image_create_info.extent.height = 128;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.format = VK_FORMAT_D32_SFLOAT;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        vk_testing::Image temp_image;
        temp_image.init(*m_device, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        auto dsvci = LvlInitStruct<VkImageViewCreateInfo>();
        // dsvci.flags |= VK_IMAGE_VIEW_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        dsvci.image = temp_image.handle();
        dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        dsvci.format = VK_FORMAT_D32_SFLOAT;
        dsvci.subresourceRange.layerCount = 1;
        dsvci.subresourceRange.baseMipLevel = 0;
        dsvci.subresourceRange.levelCount = 1;
        dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        vk_testing::ImageView dsv(*m_device, dsvci);

        auto icddi = LvlInitStruct<VkImageViewCaptureDescriptorDataInfoEXT>();
        icddi.imageView = dsv.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetImageViewOpaqueCaptureDescriptorDataEXT-None-08080");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageViewCaptureDescriptorDataInfoEXT-imageView-08083");
        vk::GetImageViewOpaqueCaptureDescriptorDataEXT(m_device->device(), &icddi, &data);
        m_errorMonitor->VerifyFound();
    }

    {
        uint8_t data[256];

        auto scddi = LvlInitStruct<VkSamplerCaptureDescriptorDataInfoEXT>();
        scddi.sampler = sampler.handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetSamplerOpaqueCaptureDescriptorDataEXT-None-08084");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSamplerCaptureDescriptorDataInfoEXT-sampler-08087");
        vk::GetSamplerOpaqueCaptureDescriptorDataEXT(m_device->device(), &scddi, &data);
        m_errorMonitor->VerifyFound();
    }

    if (IsExtensionsEnabled(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)) {
        auto blas = rt::as::blueprint::AccelStructSimpleOnDeviceBottomLevel(DeviceValidationVersion(), 4096);
        blas->Build(*m_device);

        uint8_t data[256];

        auto ascddi = LvlInitStruct<VkAccelerationStructureCaptureDescriptorDataInfoEXT>();
        ascddi.accelerationStructure = blas->handle();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT-None-08088");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-VkAccelerationStructureCaptureDescriptorDataInfoEXT-accelerationStructure-08091");
        vk::GetAccelerationStructureOpaqueCaptureDescriptorDataEXT(m_device->device(), &ascddi, &data);
        m_errorMonitor->VerifyFound();
    }

    {
        uint32_t qfi = 0;
        auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
        buffCI.size = 4096;
        buffCI.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                       VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        buffCI.queueFamilyIndexCount = 1;
        buffCI.pQueueFamilyIndices = &qfi;

        auto allocate_flag_info = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        vk_testing::Buffer d_buffer(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

        auto dbbi = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
        dbbi.address = d_buffer.address();
        dbbi.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

        d_buffer.memory().destroy();

        m_commandBuffer->begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-None-08047");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08052");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
        vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->end();
    }
}

TEST_F(NegativeDescriptorBuffer, BindingAndOffsets) {
    TEST_DESCRIPTION("Descriptor buffer binding and offsets.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    auto descriptor_buffer_features = LvlInitStruct<VkPhysicalDeviceDescriptorBufferFeaturesEXT>();
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>(&descriptor_buffer_features);
    GetPhysicalDeviceFeatures2(buffer_device_address_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &buffer_device_address_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    const bool testPushDescriptorsInBuffers =
        descriptor_buffer_features.descriptorBufferPushDescriptors && !descriptor_buffer_properties.bufferlessPushDescriptors;

    m_commandBuffer->begin();

    uint32_t qfi = 0;
    auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 4096;
    buffCI.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    if (testPushDescriptorsInBuffers) {
        buffCI.usage |= VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
    }

    auto allocate_flag_info = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vk_testing::Buffer d_buffer(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

    {
        auto dbbi = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
        dbbi.address = d_buffer.address();
        dbbi.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        if (testPushDescriptorsInBuffers) {
            dbbi.usage |= VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
        }

        std::vector<VkDescriptorBufferBindingInfoEXT> binding_infos;

        for (uint32_t i = 0; i < descriptor_buffer_properties.maxDescriptorBufferBindings + 1; i++) {
            binding_infos.push_back(dbbi);
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdBindDescriptorBuffersEXT-maxSamplerDescriptorBufferBindings-08048");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdBindDescriptorBuffersEXT-maxResourceDescriptorBufferBindings-08049");
        if (testPushDescriptorsInBuffers) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-None-08050");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-VkDescriptorBufferBindingInfoEXT-bufferlessPushDescriptors-08056");
        }
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-bufferCount-08051");
        vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), binding_infos.size(), binding_infos.data());
        m_errorMonitor->VerifyFound();

        if (!testPushDescriptorsInBuffers) {
            auto dbbpdbh = LvlInitStruct<VkDescriptorBufferBindingPushDescriptorBufferHandleEXT>();

            dbbpdbh.buffer = d_buffer.handle();

            dbbi.pNext = &dbbpdbh;

            m_errorMonitor->SetDesiredFailureMsg(
                kErrorBit, "VUID-VkDescriptorBufferBindingPushDescriptorBufferHandleEXT-bufferlessPushDescriptors-08059");
            vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi);
            m_errorMonitor->VerifyFound();

            dbbi.pNext = nullptr;
        }
    }

    vk_testing::Buffer d_buffer2(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

    if (descriptor_buffer_properties.descriptorBufferOffsetAlignment != 1) {
        auto dbbi2 = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
        dbbi2.address = d_buffer2.address() + 1;  // make alignment bad
        dbbi2.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT |
                      VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferBindingInfoEXT-address-08057");
        vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi2);
        m_errorMonitor->VerifyFound();
    }

    {
        buffCI.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        vk_testing::Buffer bufferA(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

        auto dbbi2 = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
        dbbi2.address = bufferA.address();
        dbbi2.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferBindingInfoEXT-usage-08122");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
        vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi2);
        m_errorMonitor->VerifyFound();

        dbbi2.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferBindingInfoEXT-usage-08123");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
        vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi2);
        m_errorMonitor->VerifyFound();

        dbbi2.usage = VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferBindingInfoEXT-usage-08124");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBuffersEXT-pBindingInfos-08055");
        vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi2);
        m_errorMonitor->VerifyFound();
    }

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    vk_testing::DescriptorSetLayout dsl1;
    vk_testing::DescriptorSetLayout dsl2;
    vk_testing::PipelineLayout pipeline_layout;
    {
        const VkDescriptorSetLayoutBinding bindings[] = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        };
        const auto dslci1 = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(
            nullptr, static_cast<VkDescriptorSetLayoutCreateFlags>(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT),
            size32(bindings), bindings);
        dsl1.init(*m_device, dslci1);

        const VkDescriptorSetLayoutBinding bindings2[] = {
            {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()},
        };
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci2 = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, size32(bindings2), bindings2);
        dsl2.init(*m_device, dslci2);

        const VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};
        auto plci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        plci.setLayoutCount = 2;
        plci.pSetLayouts = set_layouts;

        pipeline_layout.init(*m_device, plci);
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08070");
        vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layout.handle(), 0);
        m_errorMonitor->VerifyFound();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-set-08071");
        vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layout.handle(), 2);
        m_errorMonitor->VerifyFound();
    }

    // VUID-vkCmdSetDescriptorBufferOffsetsEXT VUs
    {
        buffCI = LvlInitStruct<VkBufferCreateInfo>();
        buffCI.size = 4096;
        buffCI.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
        buffCI.queueFamilyIndexCount = 1;
        buffCI.pQueueFamilyIndices = &qfi;
        vk_testing::Buffer bufferA(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

        auto dbbi2 = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
        dbbi2.address = bufferA.address();
        dbbi2.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

        vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi2);

        uint32_t index = 0;
        VkDeviceSize offset = 0;

        if (descriptor_buffer_properties.descriptorBufferOffsetAlignment != 1) {
            index = 0;
            offset = 1;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08061");
            vk::CmdSetDescriptorBufferOffsetsEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 pipeline_layout.handle(), 0, 1, &index, &offset);
            m_errorMonitor->VerifyFound();
        }

        index = descriptor_buffer_properties.maxDescriptorBufferBindings;
        offset = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08064");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pBufferIndices-08065");
        vk::CmdSetDescriptorBufferOffsetsEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(),
                                             0, 1, &index, &offset);
        m_errorMonitor->VerifyFound();

        uint32_t indices[3] = {0};
        VkDeviceSize offsets[3] = {0};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-08066");
        vk::CmdSetDescriptorBufferOffsetsEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(),
                                             0, 3, indices, offsets);
        m_errorMonitor->VerifyFound();

        {
            const std::optional<uint32_t> no_gfx_qfi = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
            if (no_gfx_qfi) {
                VkCommandPoolObj command_pool(m_device, no_gfx_qfi.value());
                ASSERT_TRUE(command_pool.initialized());
                VkCommandBufferObj command_buffer(m_device, &command_pool);

                index = 0;
                offset = 0;

                command_buffer.begin();

                vk::CmdBindDescriptorBuffersEXT(command_buffer.handle(), 1, &dbbi2);

                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pipelineBindPoint-08067");
                vk::CmdSetDescriptorBufferOffsetsEXT(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     pipeline_layout.handle(), 0, 1, &index, &offset);
                m_errorMonitor->VerifyFound();
                command_buffer.end();
            }
        }

        {
            const VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT,
                                                          nullptr};
            const VkDescriptorSetLayoutObj set_layout_no_flag(m_device, {binding});
            const VkPipelineLayoutObj pipeline_layout_2(m_device, {&set_layout_no_flag, &set_layout_no_flag});

            const uint32_t indices_2[2] = {0, 0};
            const VkDeviceSize offsets_2[2] = {0, 0};
            vk::CmdBindDescriptorBuffersEXT(*m_commandBuffer, 1, &dbbi2);
            // complain about set layout for set 0
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-09006");
            // complain about set layout for set 1
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-firstSet-09006");
            vk::CmdSetDescriptorBufferOffsetsEXT(*m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_2, 0, 2,
                                                 indices_2, offsets_2);
            m_errorMonitor->VerifyFound();
        }
    }

    // Test mapping from address to buffers when validating buffer offsets
    {
        const VkDeviceSize large_buffer_size =
            std::max<VkDeviceSize>(256 * descriptor_buffer_properties.descriptorBufferOffsetAlignment, 8192);
        const VkDeviceSize small_buffer_size =
            std::max<VkDeviceSize>(4 * descriptor_buffer_properties.descriptorBufferOffsetAlignment, 4096);

        // Create a large and a small buffer
        buffCI = LvlInitStruct<VkBufferCreateInfo>();
        buffCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffCI.size = large_buffer_size;
        buffCI.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
        auto large_buffer = std::make_unique<vk_testing::Buffer>();
        large_buffer->init_no_mem(*m_device, buffCI);

        buffCI.size = small_buffer_size;
        auto small_buffer = std::make_unique<vk_testing::Buffer>();
        small_buffer->init_no_mem(*m_device, buffCI);

        VkMemoryRequirements buffer_mem_reqs = {};
        vk::GetBufferMemoryRequirements(m_device->device(), large_buffer->handle(), &buffer_mem_reqs);

        // Allocate common buffer memory
        auto alloc_flags = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
        alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        VkMemoryAllocateInfo alloc_info = LvlInitStruct<VkMemoryAllocateInfo>(&alloc_flags);
        alloc_info.allocationSize = buffer_mem_reqs.size;
        m_device->phy().set_memory_type(buffer_mem_reqs.memoryTypeBits, &alloc_info, 0);
        vk_testing::DeviceMemory buffer_memory(*m_device, alloc_info);

        // Bind those buffers to the same buffer memory
        vk::BindBufferMemory(device(), large_buffer->handle(), buffer_memory.handle(), 0);
        vk::BindBufferMemory(device(), small_buffer->handle(), buffer_memory.handle(), 0);

        // Check that internal mapping from address to buffers is correctly updated
        if (large_buffer->address() == small_buffer->address()) {
            // calling large_buffer->address() twice should not result in this buffer being mapped twice.
            // If it is mapped twice, the error below will not be thrown.
            const VkDeviceAddress common_address = large_buffer->address();

            auto dbbi = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
            dbbi.address = common_address;
            dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

            vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi);

            constexpr uint32_t index = 0;

            // First call should succeed because offset is small enough to fit in large_buffer
            const VkDeviceSize offset = small_buffer_size;
            vk::CmdSetDescriptorBufferOffsetsEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 pipeline_layout.handle(), 0, 1, &index, &offset);

            large_buffer = nullptr;
            // Large buffer has been deleted, its entry in the address to buffers map must have been as well.
            // Since offset is too large to fit in small buffer, vkCmdSetDescriptorBufferOffsetsEXT should fail
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetDescriptorBufferOffsetsEXT-pOffsets-08063");
            vk::CmdSetDescriptorBufferOffsetsEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                 pipeline_layout.handle(), 0, 1, &index, &offset);
            m_errorMonitor->VerifyFound();
        }
    }

    m_commandBuffer->end();
}

TEST_F(NegativeDescriptorBuffer, InconsistentBuffer) {
    TEST_DESCRIPTION("Dispatch pipeline with descriptor set bound while descriptor buffer expected");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    InitBasicDescriptorBuffer(&buffer_device_address_features);
    if (::testing::Test::IsSkipped()) return;

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};

    auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    dslci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    dslci.bindingCount = 1;
    dslci.pBindings = &binding;

    vk_testing::DescriptorSetLayout dsl(*m_device, dslci);

    auto plci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &dsl.handle();

    vk_testing::PipelineLayout pipeline_layout(*m_device, plci);
    ASSERT_TRUE(pipeline_layout.initialized());

    uint32_t qfi = 0;
    auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 4096;
    buffCI.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    auto allocate_flag_info = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vk_testing::Buffer buffer(*m_device, buffCI, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &allocate_flag_info);

    auto dbbi = LvlInitStruct<VkDescriptorBufferBindingInfoEXT>();
    dbbi.address = buffer.address();
    dbbi.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateComputePipeline());

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    vk::CmdBindDescriptorBuffersEXT(m_commandBuffer->handle(), 1, &dbbi);

    uint32_t index = 0;
    VkDeviceSize offset = 0;
    vk::CmdSetDescriptorBufferOffsetsEXT(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                                         &index, &offset);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08115");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDescriptorBuffer, InconsistentSet) {
    TEST_DESCRIPTION("Dispatch pipeline with descriptor buffer bound while of descriptor set expected");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    InitBasicDescriptorBuffer(&buffer_device_address_features);
    if (::testing::Test::IsSkipped()) return;

    const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
    auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    dslci.flags = 0;
    dslci.bindingCount = 1;
    dslci.pBindings = &binding;

    vk_testing::DescriptorSetLayout dsl(*m_device, dslci);

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    auto ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool pool(*m_device, ds_pool_ci);
    ASSERT_TRUE(pool.initialized());

    std::unique_ptr<vk_testing::DescriptorSet> ds(pool.alloc_sets(*m_device, dsl));
    ASSERT_TRUE(ds);

    auto plci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &dsl.handle();

    vk_testing::PipelineLayout pipeline_layout(*m_device, plci);
    ASSERT_TRUE(pipeline_layout.initialized());

    CreateComputePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.cp_ci_.flags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    pipe.InitState();
    ASSERT_VK_SUCCESS(pipe.CreateComputePipeline());

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);

    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &ds->handle(), 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-None-08117");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(NegativeDescriptorBuffer, BindPoint) {
    TEST_DESCRIPTION("Descriptor buffer invalid bind point.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    InitBasicDescriptorBuffer(&buffer_device_address_features);
    if (::testing::Test::IsSkipped()) return;

    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vk_testing::PipelineLayout pipeline_layout;
    {
        const VkDescriptorSetLayoutBinding bindings[] = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        };
        const VkDescriptorSetLayoutCreateInfo dslci1 = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr,
                                                        VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, size32(bindings),
                                                        bindings};
        vk_testing::DescriptorSetLayout dsl1(*m_device, dslci1);

        vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

        const VkDescriptorSetLayoutBinding binding2 = {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                       &sampler.handle()};
        const VkDescriptorSetLayoutCreateFlags flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT |
                                                       VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT;
        const auto dslci2 = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, flags, 1u, &binding2);
        vk_testing::DescriptorSetLayout dsl2(*m_device, dslci2);

        const VkDescriptorSetLayout set_layouts[2] = {dsl1.handle(), dsl2.handle()};
        auto plci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
        plci.setLayoutCount = 2;
        plci.pSetLayouts = set_layouts;

        pipeline_layout.init(*m_device, plci);
    }

    {
        const std::optional<uint32_t> no_gfx_qfi = m_device->QueueFamilyMatching(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
        if (!no_gfx_qfi) {
            GTEST_SKIP() << "No compute and transfer only queue family, skipping bindpoint and queue tests.";
            return;
        }

        VkCommandPoolObj command_pool(m_device, no_gfx_qfi.value());
        ASSERT_TRUE(command_pool.initialized());
        VkCommandBufferObj command_buffer(m_device, &command_pool);

        command_buffer.begin();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-vkCmdBindDescriptorBufferEmbeddedSamplersEXT-pipelineBindPoint-08069");
        vk::CmdBindDescriptorBufferEmbeddedSamplersEXT(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                       pipeline_layout.handle(), 1);
        m_errorMonitor->VerifyFound();
        command_buffer.end();
    }
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoBasic) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo().");
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    uint8_t buffer[128];
    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();
    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08018");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08018");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoSampler) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo() with a sampler backed VkDescriptorImageInfo.");
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    uint8_t buffer[128];
    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    const VkDescriptorImageInfo dii = {sampler.handle(), VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL};
    auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();

    dgi.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dgi.data.pCombinedImageSampler = &dii;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08034");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.combinedImageSamplerDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    dgi.data.pSampledImage = &dii;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08035");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.sampledImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.data.pSampledImage = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08035");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.sampledImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dgi.data.pStorageImage = &dii;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08036");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.data.pStorageImage = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08036");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageImageDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08037");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dgi.data.pStorageTexelBuffer = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08038");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dgi.data.pUniformBuffer = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08039");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dgi.data.pStorageBuffer = nullptr;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08040");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoAS) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo() for Acceleration Structure.");
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    uint8_t buffer[128];
    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();
    dgi.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    dgi.data.accelerationStructure = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08041");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.accelerationStructureDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoRtxNV) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo() for NV_ray_tracing.");
    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    uint8_t buffer[128];
    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();
    dgi.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    dgi.data.accelerationStructure = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08042");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.accelerationStructureDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, DescriptorGetInfoAddressRange) {
    TEST_DESCRIPTION("Descriptor buffer vkDescriptorGetInfo() with VkDescriptorAddressInfoEXT.");
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    auto buffer_device_address_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeatures>();
    InitBasicDescriptorBuffer(&buffer_device_address_features);
    if (::testing::Test::IsSkipped()) return;

    uint8_t buffer[128];
    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    uint32_t qfi = 0;
    auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 4096;
    buffCI.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    vk_testing::Buffer d_buffer;
    d_buffer.init_no_mem(*m_device, buffCI);

    auto dai = LvlInitStruct<VkDescriptorAddressInfoEXT>();
    auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();
    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dgi.data.pUniformBuffer = &dai;

    dai.address = 0;
    dai.range = 4;
    dai.format = VK_FORMAT_R8_UINT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-address-08043");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), d_buffer.handle(), &mem_reqs);

    auto memflagsinfo = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    memflagsinfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    auto mem_alloc_info =
        vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, mem_reqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    mem_alloc_info.pNext = &memflagsinfo;

    vk_testing::DeviceMemory mem(*m_device, mem_alloc_info);

    d_buffer.bind_memory(mem, 0);

    dai.address = d_buffer.address();
    dai.range = 4096 * buffCI.size;
    dai.format = VK_FORMAT_R8_UINT;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-range-08045");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dai.range = 0;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-range-08940");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dai.range = VK_WHOLE_SIZE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-range-08045");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-nullDescriptor-08939");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    {
        dai.range = 4;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDescriptorEXT-dataSize-08125");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize - 1, &buffer);
        m_errorMonitor->VerifyFound();
    }

    mem.destroy();

    dai.range = 4;

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dgi.data.pUniformBuffer = &dai;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08030");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    dgi.data.pStorageBuffer = &dai;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08031");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    dgi.data.pUniformTexelBuffer = &dai;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08032");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();

    dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dgi.data.pStorageTexelBuffer = &dai;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorDataEXT-type-08033");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageTexelBufferDescriptorSize, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, Various) {
    TEST_DESCRIPTION("Descriptor buffer various tests.");
    AddOptionalExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    AddOptionalExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;
    const bool nv_ray_tracing = IsExtensionsEnabled(VK_NV_RAY_TRACING_EXTENSION_NAME);

    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    vk_testing::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());
    {
        VkSampler invalid_sampler = CastToHandle<VkSampler, uintptr_t>(0xbaadbeef);
        VkImageView invalid_imageview = CastToHandle<VkImageView, uintptr_t>(0xbaadbeef);
        VkDeviceAddress invalid_buffer = CastToHandle<VkDeviceAddress, uintptr_t>(0xbaadbeef);

        uint8_t buffer[128];
        auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();

        const VkDescriptorImageInfo dii = {invalid_sampler, invalid_imageview, VK_IMAGE_LAYOUT_GENERAL};

        dgi.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        dgi.data.pCombinedImageSampler = &dii;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08019");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08020");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.combinedImageSamplerDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        dgi.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        dgi.data.pInputAttachmentImage = &dii;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08021");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.inputAttachmentDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        dgi.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        dgi.data.pSampledImage = &dii;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08022");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.sampledImageDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        dgi.data.pStorageImage = &dii;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08023");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageImageDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        auto dai = LvlInitStruct<VkDescriptorAddressInfoEXT>();
        dai.address = invalid_buffer;
        dai.range = 64;
        dai.format = VK_FORMAT_R8_UINT;

        dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        dgi.data.pUniformTexelBuffer = &dai;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-None-08044");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08024");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformTexelBufferDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        dgi.data.pStorageTexelBuffer = &dai;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-None-08044");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08025");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageTexelBufferDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        dgi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dgi.data.pUniformTexelBuffer = &dai;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-None-08044");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08026");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.uniformBufferDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        dgi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        dgi.data.pStorageTexelBuffer = &dai;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorAddressInfoEXT-None-08044");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08027");
        vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
        m_errorMonitor->VerifyFound();

        if (nv_ray_tracing) {
            dgi.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
            dgi.data.pStorageTexelBuffer = &dai;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08029");
            vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.storageBufferDescriptorSize, &buffer);
            m_errorMonitor->VerifyFound();
        }
    }

    {
        const VkDescriptorSetLayoutBinding binding{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                   &sampler.handle()};
        const auto dslci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(nullptr, 0U, 1U, &binding);
        vk_testing::DescriptorSetLayout dsl(*m_device, dslci);

        {
            VkDeviceSize size;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDescriptorSetLayoutSizeEXT-layout-08012");
            vk::GetDescriptorSetLayoutSizeEXT(m_device->device(), dsl.handle(), &size);
            m_errorMonitor->VerifyFound();

            VkDeviceSize offset;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkGetDescriptorSetLayoutBindingOffsetEXT-layout-08014");
            vk::GetDescriptorSetLayoutBindingOffsetEXT(m_device->device(), dsl.handle(), 0, &offset);
            m_errorMonitor->VerifyFound();
        }
    }

    {
        uint32_t qfi = 0;
        auto buffCI = LvlInitStruct<VkBufferCreateInfo>();
        buffCI.flags = VK_BUFFER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        buffCI.size = 4096;
        buffCI.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        buffCI.queueFamilyIndexCount = 1;
        buffCI.pQueueFamilyIndices = &qfi;

        vk_testing::Buffer d_buffer(*m_device, buffCI, vk_testing::no_mem);

        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(m_device->device(), d_buffer.handle(), &mem_reqs);

        auto mem_alloc_info =
            vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, mem_reqs, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        // no alloc flags
        vk_testing::DeviceMemory mem(*m_device, mem_alloc_info);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindBufferMemory-descriptorBufferCaptureReplay-08112");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemory-bufferDeviceAddressCaptureReplay-09200");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindBufferMemory-buffer-09201");
        vk::BindBufferMemory(m_device->device(), d_buffer.handle(), mem.handle(), 0);
        m_errorMonitor->VerifyFound();
    }
    {
        auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
        image_create_info.flags = VK_IMAGE_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = 128;
        image_create_info.extent.height = 128;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.format = VK_FORMAT_D32_SFLOAT;
        image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        vk_testing::Image temp_image(*m_device, image_create_info, vk_testing::no_mem);

        VkMemoryRequirements mem_reqs;
        vk::GetImageMemoryRequirements(m_device->device(), temp_image.handle(), &mem_reqs);

        auto mem_alloc_info =
            vk_testing::DeviceMemory::get_resource_alloc_info(*m_device, mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // no allocate flags
        vk_testing::DeviceMemory mem(*m_device, mem_alloc_info);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkBindImageMemory-descriptorBufferCaptureReplay-08113");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBindImageMemory-image-09202");
        vk::BindImageMemory(m_device->device(), temp_image.handle(), mem.handle(), 0);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeDescriptorBuffer, ExtensionCombination) {
    TEST_DESCRIPTION("Descriptor invalid extension combination.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME);
    AddRequiredExtensions(VK_AMD_SHADER_FRAGMENT_MASK_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    const auto q_props = vk_testing::PhysicalDevice(gpu()).queue_properties();
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    const float q_priority[] = {1.0f};
    auto queue_ci = LvlInitStruct<VkDeviceQueueCreateInfo>();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    auto device_ci = LvlInitStruct<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    device_ci.enabledExtensionCount = m_device_extension_names.size();
    device_ci.ppEnabledExtensionNames = m_device_extension_names.data();

    auto dbf = LvlInitStruct<VkPhysicalDeviceDescriptorBufferFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&dbf);
    device_ci.pNext = &features2;

    dbf.descriptorBuffer = true;

    VkDevice testDevice;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceCreateInfo-None-08095");
    vk::CreateDevice(gpu(), &device_ci, nullptr, &testDevice);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, SetBufferAddressSpaceLimits) {
    TEST_DESCRIPTION("Create VkBuffer with extension.");
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);
    // After a few GB, can have memory issues running these tests
    // descriptorBufferAddressSpaceSize is always the largest of the 3 buffer address size limits
    constexpr VkDeviceSize max_limit = static_cast<VkDeviceSize>(1) << 31;
    if (descriptor_buffer_properties.descriptorBufferAddressSpaceSize > max_limit) {
        GTEST_SKIP() << "descriptorBufferAddressSpaceSize are too large";
    }

    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = descriptor_buffer_properties.descriptorBufferAddressSpaceSize + 1;

    buffer_ci.usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-08097");

    buffer_ci.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-08098");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCreateInfo-usage-08097");
    buffer_ci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                      VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
    CreateBufferTest(*this, &buffer_ci, "VUID-VkBufferCreateInfo-usage-08098");
}

// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5826
TEST_F(NegativeDescriptorBuffer, NullHandle) {
    TEST_DESCRIPTION("Descriptor buffer various tests.");
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    const auto invalid_sampler = CastToHandle<VkSampler, uintptr_t>(0x0);
    const auto invalid_imageview = CastToHandle<VkImageView, uintptr_t>(0x0);

    std::array<std::byte, 128> buffer = {};
    auto dgi = LvlInitStruct<VkDescriptorGetInfoEXT>();

    const VkDescriptorImageInfo dii = {invalid_sampler, invalid_imageview, VK_IMAGE_LAYOUT_GENERAL};

    dgi.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dgi.data.pInputAttachmentImage = &dii;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorGetInfoEXT-type-08021");
    vk::GetDescriptorEXT(m_device->device(), &dgi, descriptor_buffer_properties.inputAttachmentDescriptorSize, buffer.data());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeDescriptorBuffer, BufferUsage) {
    TEST_DESCRIPTION("Wrong Usage for buffer createion.");
    InitBasicDescriptorBuffer();
    if (::testing::Test::IsSkipped()) return;

    auto descriptor_buffer_properties = LvlInitStruct<VkPhysicalDeviceDescriptorBufferPropertiesEXT>();
    GetPhysicalDeviceProperties2(descriptor_buffer_properties);

    if (descriptor_buffer_properties.bufferlessPushDescriptors) {
        GTEST_SKIP() << "bufferlessPushDescriptors is supported";
    }

    auto buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.size = 64;
    buffer_create_info.usage = VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;

    VkBuffer buffer;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkBufferCreateInfo-usage-08103");
    vk::CreateBuffer(device(), &buffer_create_info, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}