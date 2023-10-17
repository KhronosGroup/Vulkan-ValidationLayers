/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "generated/vk_extension_helper.h"

void YcbcrTest::InitBasicYcbcr(void *pNextFeatures) {
    // VK_KHR_sampler_ycbcr_conversion was added in 1.2
    const bool use_12 = m_attempted_api_version >= VK_API_VERSION_1_2;
    if (!use_12) {
        AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    }
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper(pNextFeatures);
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features = vku::InitStructHelper(pNextFeatures);
    VkPhysicalDeviceFeatures2 features2;
    if (use_12) {
        features2 = GetPhysicalDeviceFeatures2(features11);
        if (features11.samplerYcbcrConversion != VK_TRUE) {
            GTEST_SKIP() << "samplerYcbcrConversion not supported, skipping test";
        }
    } else {
        features2 = GetPhysicalDeviceFeatures2(ycbcr_features);
        if (ycbcr_features.samplerYcbcrConversion != VK_TRUE) {
            GTEST_SKIP() << "samplerYcbcrConversion feature not supported";
        }
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
}

TEST_F(PositiveYcbcr, PlaneAspectNone) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(Init())

    VkImageCreateInfo image_createinfo = vku::InitStructHelper();
    image_createinfo.imageType = VK_IMAGE_TYPE_2D;
    image_createinfo.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    image_createinfo.extent = {128, 128, 1};
    image_createinfo.mipLevels = 1;
    image_createinfo.arrayLayers = 1;
    image_createinfo.samples = VK_SAMPLE_COUNT_1_BIT;
    image_createinfo.tiling = VK_IMAGE_TILING_LINEAR;
    image_createinfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_createinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_createinfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkDeviceImageMemoryRequirements image_mem_reqs = vku::InitStructHelper();
    image_mem_reqs.pCreateInfo = &image_createinfo;
    image_mem_reqs.planeAspect = VK_IMAGE_ASPECT_NONE;
    VkMemoryRequirements2 mem_reqs_2 = vku::InitStructHelper();
    vk::GetDeviceImageMemoryRequirements(device(), &image_mem_reqs, &mem_reqs_2);
}

TEST_F(PositiveYcbcr, MultiplaneGetImageSubresourceLayout) {
    TEST_DESCRIPTION("Positive test, query layout of a single plane of a multiplane image. (repro Github #2530)");
    RETURN_IF_SKIP(InitBasicYcbcr())

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_LINEAR;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify format
    if (!ImageFormatIsSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    VkImageObj image{m_device};
    image.init_no_mem(*m_device, ci);

    // Query layout of 3rd plane
    VkImageSubresource subres = {};
    subres.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    subres.mipLevel = 0;
    subres.arrayLayer = 0;
    VkSubresourceLayout layout = {};

    vk::GetImageSubresourceLayout(device(), image, &subres, &layout);
}

TEST_F(PositiveYcbcr, MultiplaneImageCopyBufferToImage) {
    TEST_DESCRIPTION("Positive test of multiplane copy buffer to image");
    RETURN_IF_SKIP(InitBasicYcbcr())
    InitRenderTarget();

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR;  // All planes of equal extent
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {16, 16, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), gpu(), ci, features)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    VkImageObj image(m_device);
    image.init(&ci);

    m_commandBuffer->reset();
    m_commandBuffer->begin();
    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    std::array<VkImageAspectFlagBits, 3> aspects = {
        {VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, VK_IMAGE_ASPECT_PLANE_2_BIT}};
    std::array<vkt::Buffer, 3> buffers;

    VkBufferImageCopy copy = {};
    copy.imageSubresource.layerCount = 1;
    copy.imageExtent.depth = 1;
    copy.imageExtent.height = 16;
    copy.imageExtent.width = 16;

    for (size_t i = 0; i < aspects.size(); ++i) {
        buffers[i].init(*m_device, 16 * 16 * 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        copy.imageSubresource.aspectMask = aspects[i];
        vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffers[i].handle(), image.handle(),
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    }
    m_commandBuffer->end();
}

TEST_F(PositiveYcbcr, MultiplaneImageCopy) {
    TEST_DESCRIPTION("Copy Plane 0 to Plane 2");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicYcbcr())
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR;  // All planes of equal extent
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify format
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), gpu(), ci, features)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    VkImageObj image{m_device};
    image.init_no_mem(*m_device, ci);

    vkt::DeviceMemory mem_obj;
    mem_obj.init(*m_device, vkt::DeviceMemory::get_resource_alloc_info(*m_device, image.memory_requirements(), 0));

    vk::BindImageMemory(device(), image, mem_obj, 0);

    // Copy plane 0 to plane 2
    VkImageCopy copyRegion = {};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT_KHR;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = {0, 0, 0};
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = {0, 0, 0};
    copyRegion.extent.width = 128;
    copyRegion.extent.height = 128;
    copyRegion.extent.depth = 1;

    m_commandBuffer->begin();
    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_GENERAL);
    vk::CmdCopyImage(m_commandBuffer->handle(), image.handle(), VK_IMAGE_LAYOUT_GENERAL, image.handle(), VK_IMAGE_LAYOUT_GENERAL, 1,
                     &copyRegion);
    m_commandBuffer->end();

    auto submit_info = vku::InitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveYcbcr, MultiplaneImageBindDisjoint) {
    TEST_DESCRIPTION("Bind image with disjoint memory");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicYcbcr())
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR;  // All planes of equal extent
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {128, 128, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify format
    VkFormatFeatureFlags features =
        VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_DISJOINT_BIT;
    if (!ImageFormatIsSupported(instance(), gpu(), ci, features)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    VkImageObj image{m_device};
    image.init_no_mem(*m_device, ci);

    // Allocate & bind memory
    auto image_plane_req = vku::InitStruct<VkImagePlaneMemoryRequirementsInfo>();
    auto mem_req_info2 = vku::InitStruct<VkImageMemoryRequirementsInfo2>(&image_plane_req);
    mem_req_info2.image = image;
    auto mem_reqs2 = vku::InitStruct<VkMemoryRequirements2>();

    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    auto alloc_info = vku::InitStruct<VkMemoryAllocateInfo>();

    // Plane 0
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_reqs2);
    uint32_t mem_type = 0;
    auto phys_mem_props2 = vku::InitStruct<VkPhysicalDeviceMemoryProperties2>();
    vk::GetPhysicalDeviceMemoryProperties2(gpu(), &phys_mem_props2);
    for (mem_type = 0; mem_type < phys_mem_props2.memoryProperties.memoryTypeCount; mem_type++) {
        if ((mem_reqs2.memoryRequirements.memoryTypeBits & (1 << mem_type)) &&
            ((phys_mem_props2.memoryProperties.memoryTypes[mem_type].propertyFlags & mem_props) == mem_props)) {
            alloc_info.memoryTypeIndex = mem_type;
            break;
        }
    }
    alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
    vkt::DeviceMemory p0_mem(*m_device, alloc_info);

    // Plane 1 & 2 use same memory type
    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_reqs2);
    alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
    vkt::DeviceMemory p1_mem(*m_device, alloc_info);

    image_plane_req.planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;
    vk::GetImageMemoryRequirements2(device(), &mem_req_info2, &mem_reqs2);
    alloc_info.allocationSize = mem_reqs2.memoryRequirements.size;
    vkt::DeviceMemory p2_mem(*m_device, alloc_info);

    // Set up 3-plane binding
    VkBindImageMemoryInfo bind_info[3];
    VkBindImagePlaneMemoryInfo plane_info[3];
    for (int plane = 0; plane < 3; plane++) {
        plane_info[plane] = vku::InitStruct<VkBindImagePlaneMemoryInfo>();
        bind_info[plane] = vku::InitStruct<VkBindImageMemoryInfo>(&plane_info[plane]);
        bind_info[plane].image = image;
        bind_info[plane].memoryOffset = 0;
    }
    bind_info[0].memory = p0_mem.handle();
    bind_info[1].memory = p1_mem.handle();
    bind_info[2].memory = p2_mem.handle();
    plane_info[0].planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
    plane_info[1].planeAspect = VK_IMAGE_ASPECT_PLANE_1_BIT;
    plane_info[2].planeAspect = VK_IMAGE_ASPECT_PLANE_2_BIT;

    vk::BindImageMemory2(device(), 3, bind_info);
}

TEST_F(PositiveYcbcr, ImageLayout) {
    TEST_DESCRIPTION("Test that changing the layout of ASPECT_COLOR also changes the layout of the individual planes");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicYcbcr())
    InitRenderTarget();

    if (!FormatFeaturesAreSupported(gpu(), VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT)) {
        GTEST_SKIP() << "Required formats/features not supported";
    }

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR;  // All planes of equal extent
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {256, 256, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Verify format
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
    if (!ImageFormatIsSupported(instance(), gpu(), ci, features)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    VkImageObj image(m_device);
    image.Init(ci);

    vkt::Buffer buffer(*m_device, 128 * 128 * 3, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    VkBufferImageCopy copy_region = {};
    copy_region.bufferRowLength = 128;
    copy_region.bufferImageHeight = 128;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent.height = 64;
    copy_region.imageExtent.width = 64;
    copy_region.imageExtent.depth = 1;

    vk::ResetCommandBuffer(m_commandBuffer->handle(), 0);
    m_commandBuffer->begin();
    image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    vk::CmdCopyBufferToImage(m_commandBuffer->handle(), buffer.handle(), image.handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &copy_region);
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer(false);

    // Test to verify that views of multiplanar images have layouts tracked correctly
    // by changing the image's layout then using a view of that image
    vkt::SamplerYcbcrConversion conversion(*m_device, VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR);
    auto conversion_info = conversion.ConversionInfo();
    auto ivci = vku::InitStruct<VkImageViewCreateInfo>(&conversion_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView view(*m_device, ivci);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.pNext = &conversion_info;
    vkt::Sampler sampler(*m_device, sampler_ci);

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, &sampler.handle()},
                  });

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorImageInfo(0, view.handle(), sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img_barrier.image = image.handle();
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                           VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &img_barrier);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    auto submit_info = vku::InitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);

    vk::QueueWaitIdle(m_default_queue);
}
