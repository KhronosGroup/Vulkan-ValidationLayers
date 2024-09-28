/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
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
#include "../framework/render_pass_helper.h"
#include "../framework/barrier_queue_family.h"

#include "utils/vk_layer_utils.h"

// A reasonable well supported default VkImageCreateInfo for image creation
VkImageCreateInfo ImageTest::DefaultImageInfo() {
    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.flags = 0;                          // assumably any is supported
    ci.imageType = VK_IMAGE_TYPE_2D;       // any is supported
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;  // has mandatory support for all usages
    ci.extent = {64, 64, 1};               // limit is 256 for 3D, or 4096
    ci.mipLevels = 1;                      // any is supported
    ci.arrayLayers = 1;                    // limit is 256
    ci.samples = VK_SAMPLE_COUNT_1_BIT;    // needs to be 1 if TILING_LINEAR
    // if VK_IMAGE_TILING_LINEAR imageType must be 2D, usage must be TRANSFER, and levels layers samplers all 1
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;  // depends on format
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    return ci;
}

class PositiveImage : public ImageTest {};

TEST_F(PositiveImage, OwnershipTranfersImage) {
    TEST_DESCRIPTION("Valid image ownership transfers that shouldn't create errors");
    RETURN_IF_SKIP(Init());

    vkt::Queue *no_gfx_queue = m_device->QueueWithoutCapabilities(VK_QUEUE_GRAPHICS_BIT);
    if (!no_gfx_queue) {
        GTEST_SKIP() << "Required queue not present (non-graphics non-compute capable required)";
    }

    vkt::CommandPool no_gfx_pool(*m_device, no_gfx_queue->family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer no_gfx_cb(*m_device, no_gfx_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Create an "exclusive" image owned by the graphics queue.
    VkFlags image_use = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkt::Image image(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, image_use);
    image.SetLayout(VK_IMAGE_LAYOUT_GENERAL);
    auto image_subres = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
    auto image_barrier = image.image_memory_barrier(0, 0, image.Layout(), image.Layout(), image_subres);
    image_barrier.srcQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.dstQueueFamilyIndex = no_gfx_queue->family_index;

    ValidOwnershipTransfer(m_errorMonitor, m_default_queue, m_command_buffer, no_gfx_queue, no_gfx_cb,
                           VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, nullptr, &image_barrier);

    // Change layouts while changing ownership
    image_barrier.srcQueueFamilyIndex = no_gfx_queue->family_index;
    image_barrier.dstQueueFamilyIndex = m_device->graphics_queue_node_index_;
    image_barrier.oldLayout = image.Layout();
    // Make sure the new layout is different from the old
    if (image_barrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } else {
        image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    ValidOwnershipTransfer(m_errorMonitor, no_gfx_queue, no_gfx_cb, m_default_queue, m_command_buffer,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, nullptr, &image_barrier);
}

TEST_F(PositiveImage, AliasedMemoryTracking) {
    TEST_DESCRIPTION(
        "Create a buffer, allocate memory, bind memory, destroy the buffer, create an image, and bind the same memory to it");

    RETURN_IF_SKIP(Init());

    VkDeviceSize buff_size = 256;
    auto buffer = std::make_unique<vkt::Buffer>(*m_device, vkt::Buffer::CreateInfo(buff_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT),
                                                vkt::no_mem);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;  // mandatory format
    image_create_info.extent.width = 64;                  // at least 4096x4096 is supported
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    const auto buffer_memory_requirements = buffer->memory_requirements();
    const auto image_memory_requirements = image.memory_requirements();

    vkt::DeviceMemory mem;
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = (std::max)(buffer_memory_requirements.size, image_memory_requirements.size);
    bool has_memtype =
        m_device->phy().SetMemoryType(buffer_memory_requirements.memoryTypeBits & image_memory_requirements.memoryTypeBits,
                                      &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (!has_memtype) {
        GTEST_SKIP() << "Failed to find a host visible memory type for both a buffer and an image";
    }
    mem.init(*m_device, alloc_info);

    auto pData = mem.map();
    std::memset(pData, 0xCADECADE, static_cast<size_t>(buff_size));
    mem.unmap();

    buffer->bind_memory(mem, 0);

    // NOW, destroy the buffer. Obviously, the resource no longer occupies this
    // memory. In fact, it was never used by the GPU.
    // Just be sure, wait for idle.
    buffer.reset(nullptr);
    m_device->Wait();

    // VALIDATION FAILURE:
    image.bind_memory(mem, 0);
}

TEST_F(PositiveImage, CreateImageViewFollowsParameterCompatibilityRequirements) {
    TEST_DESCRIPTION("Verify that creating an ImageView with valid usage does not generate validation errors.");

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo imgInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 nullptr,
                                 VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
                                 VK_IMAGE_TYPE_2D,
                                 VK_FORMAT_R8G8B8A8_UNORM,
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
    vkt::Image image(*m_device, imgInfo, vkt::set_layout);
    image.CreateView();
}

TEST_F(PositiveImage, BasicUsage) {
    TEST_DESCRIPTION("Verify that we can create a view with usage INPUT_ATTACHMENT");
    RETURN_IF_SKIP(Init());
    vkt::Image image(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
    vkt::ImageView view = image.CreateView();
}

TEST_F(PositiveImage, BarrierLayoutToImageUsage) {
    TEST_DESCRIPTION("Ensure barriers' new and old VkImageLayout are compatible with their images' VkImageUsageFlags");

    RETURN_IF_SKIP(Init());
    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    InitRenderTarget();

    VkImageMemoryBarrier img_barrier = vku::InitStructHelper();
    img_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;

    {
        vkt::Image img_color(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        vkt::Image img_ds1(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        vkt::Image img_ds2(*m_device, 128, 128, 1, depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        vkt::Image img_xfer_src(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        vkt::Image img_xfer_dst(*m_device, 128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        vkt::Image img_sampled(*m_device, 32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
        vkt::Image img_input(*m_device, 128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        const struct {
            vkt::Image &image_obj;
            VkImageLayout old_layout;
            VkImageLayout new_layout;
        } buffer_layouts[] = {
            // clang-format off
            {img_color,    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_ds1,      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL},
            {img_ds2,      VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  VK_IMAGE_LAYOUT_GENERAL},
            {img_sampled,  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_input,    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             VK_IMAGE_LAYOUT_GENERAL},
            {img_xfer_dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             VK_IMAGE_LAYOUT_GENERAL},
            // clang-format on
        };
        const uint32_t layout_count = sizeof(buffer_layouts) / sizeof(buffer_layouts[0]);

        m_command_buffer.begin();
        for (uint32_t i = 0; i < layout_count; ++i) {
            img_barrier.image = buffer_layouts[i].image_obj.handle();
            const VkImageUsageFlags usage = buffer_layouts[i].image_obj.usage();
            img_barrier.subresourceRange.aspectMask = (usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                                                          ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)
                                                          : VK_IMAGE_ASPECT_COLOR_BIT;

            img_barrier.oldLayout = buffer_layouts[i].old_layout;
            img_barrier.newLayout = buffer_layouts[i].new_layout;
            vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                                   nullptr, 0, nullptr, 1, &img_barrier);

            img_barrier.oldLayout = buffer_layouts[i].new_layout;
            img_barrier.newLayout = buffer_layouts[i].old_layout;
            vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 0,
                                   nullptr, 0, nullptr, 1, &img_barrier);
        }
        m_command_buffer.end();

        img_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        img_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
}

TEST_F(PositiveImage, FormatCompatibility) {
    TEST_DESCRIPTION("Test format compatibility");

    AddRequiredExtensions(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    RETURN_IF_SKIP(InitState());

    VkFormat format = VK_FORMAT_R12X4G12X4_UNORM_2PACK16;

    VkImageFormatListCreateInfo format_list = vku::InitStructHelper();
    format_list.viewFormatCount = 1;
    format_list.pViewFormats = &format;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&format_list);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    VkImage image;
    vk::CreateImage(device(), &image_create_info, nullptr, &image);

    vk::DestroyImage(device(), image, nullptr);
}

TEST_F(PositiveImage, MultpilePNext) {
    TEST_DESCRIPTION(
        "Use VkImageFormatListCreateInfo and VkImageCompressionControlEXT to make sure internal "
        "DispatchGetPhysicalDeviceImageFormatProperties2 pass them along");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    const VkFormat view_format = VK_FORMAT_R8G8B8A8_UINT;

    VkImageFormatListCreateInfo format_list = vku::InitStructHelper();
    format_list.viewFormatCount = 1;
    format_list.pViewFormats = &view_format;

    VkImageCompressionControlEXT image_compression = vku::InitStructHelper(&format_list);
    image_compression.compressionControlPlaneCount = 0;
    image_compression.flags = VK_IMAGE_COMPRESSION_DEFAULT_EXT;

    VkImageCreateInfo image_create_info = vku::InitStructHelper(&image_compression);
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.flags = 0;

    vkt::Image(*m_device, image_create_info);
}

TEST_F(PositiveImage, FramebufferFrom3DImage) {
    TEST_DESCRIPTION("Validate creating a framebuffer from a 3D image.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 4;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkImageViewCreateInfo dsvci = vku::InitStructHelper();
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    dsvci.format = VK_FORMAT_B8G8R8A8_UNORM;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.layerCount = 4;
    dsvci.subresourceRange.baseArrayLayer = 0;
    dsvci.subresourceRange.levelCount = 1;
    vkt::ImageView view(*m_device, dsvci);

    VkFramebufferCreateInfo fci = vku::InitStructHelper();
    fci.renderPass = m_renderPass;
    fci.attachmentCount = 1;
    fci.pAttachments = &view.handle();
    fci.width = 32;
    fci.height = 32;
    fci.layers = 4;
    vkt::Framebuffer fb(*m_device, fci);
}

TEST_F(PositiveImage, SubresourceLayout) {
    RETURN_IF_SKIP(Init());

    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = 64;
    image_ci.extent.height = 64;
    image_ci.mipLevels = 7;
    image_ci.arrayLayers = 6;
    image_ci.format = VK_FORMAT_R8_UINT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_ci);

    m_command_buffer.begin();
    const auto subresource_range = image.subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);
    auto barrier = image.image_memory_barrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresource_range);
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.subresourceRange.baseMipLevel = 1;
    barrier.subresourceRange.levelCount = 1;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.subresourceRange.baseMipLevel = 2;
    vk::CmdPipelineBarrier(m_command_buffer.handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);
    m_command_buffer.end();
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(PositiveImage, ImagelessLayoutTracking) {
    TEST_DESCRIPTION("Test layout tracking on imageless framebuffers");
    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceImagelessFramebufferFeaturesKHR physicalDeviceImagelessFramebufferFeatures =
        vku::InitStructHelper();
    physicalDeviceImagelessFramebufferFeatures.imagelessFramebuffer = VK_TRUE;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 =
        vku::InitStructHelper(&physicalDeviceImagelessFramebufferFeatures);

    uint32_t physical_device_group_count = 0;
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, nullptr);

    if (physical_device_group_count == 0) {
        GTEST_SKIP() << "physical_device_group_count is 0";
    }
    std::vector<VkPhysicalDeviceGroupProperties> physical_device_group(physical_device_group_count,
                                                                       {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});
    vk::EnumeratePhysicalDeviceGroups(instance(), &physical_device_group_count, physical_device_group.data());
    VkDeviceGroupDeviceCreateInfo create_device_pnext = vku::InitStructHelper();
    create_device_pnext.physicalDeviceCount = physical_device_group[0].physicalDeviceCount;
    create_device_pnext.pPhysicalDevices = physical_device_group[0].physicalDevices;
    create_device_pnext.pNext = &physicalDeviceFeatures2;

    RETURN_IF_SKIP(InitState(nullptr, &create_device_pnext));
    RETURN_IF_SKIP(InitSwapchain(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));

    uint32_t attachmentWidth = m_surface_capabilities.minImageExtent.width;
    uint32_t attachmentHeight = m_surface_capabilities.minImageExtent.height;
    VkFormat attachmentFormat = m_surface_formats[0].format;

    RenderPassSingleSubpass rp(*this);
    rp.AddAttachmentDescription(attachmentFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    rp.AddAttachmentReference({0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
    rp.AddColorAttachment(0);
    rp.CreateRenderPass();

    // Create an image to use in an imageless framebuffer.  Bind swapchain memory to it.
    VkImageSwapchainCreateInfoKHR image_swapchain_create_info = vku::InitStructHelper();
    image_swapchain_create_info.swapchain = m_swapchain;
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                         &image_swapchain_create_info,
                                         0,
                                         VK_IMAGE_TYPE_2D,
                                         attachmentFormat,
                                         {attachmentWidth, attachmentHeight, 1},
                                         1,
                                         1,
                                         VK_SAMPLE_COUNT_1_BIT,
                                         VK_IMAGE_TILING_OPTIMAL,
                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                         VK_SHARING_MODE_EXCLUSIVE,
                                         0,
                                         nullptr,
                                         VK_IMAGE_LAYOUT_UNDEFINED};
    vkt::Image image(*m_device, imageCreateInfo, vkt::no_mem);

    VkBindImageMemoryDeviceGroupInfo bind_devicegroup_info = vku::InitStructHelper();
    bind_devicegroup_info.deviceIndexCount = physical_device_group[0].physicalDeviceCount;
    std::array<uint32_t, 8> deviceIndices = {{0}};
    bind_devicegroup_info.pDeviceIndices = deviceIndices.data();
    bind_devicegroup_info.splitInstanceBindRegionCount = 0;
    bind_devicegroup_info.pSplitInstanceBindRegions = nullptr;

    VkBindImageMemorySwapchainInfoKHR bind_swapchain_info = vku::InitStructHelper(&bind_devicegroup_info);
    bind_swapchain_info.swapchain = m_swapchain;
    bind_swapchain_info.imageIndex = 0;

    VkBindImageMemoryInfo bind_info = vku::InitStructHelper(&bind_swapchain_info);
    bind_info.image = image.handle();
    bind_info.memory = VK_NULL_HANDLE;
    bind_info.memoryOffset = 0;

    vk::BindImageMemory2(device(), 1, &bind_info);

    uint32_t swapchain_images_count = 0;
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, nullptr);
    std::vector<VkImage> swapchain_images;
    swapchain_images.resize(swapchain_images_count);
    vk::GetSwapchainImagesKHR(device(), m_swapchain, &swapchain_images_count, swapchain_images.data());
    uint32_t current_buffer;
    vkt::Semaphore image_acquired(*m_device);
    vk::AcquireNextImageKHR(device(), m_swapchain, kWaitTimeout, image_acquired, VK_NULL_HANDLE, &current_buffer);

    vkt::ImageView imageView = image.CreateView();
    VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO_KHR,
                                                                          nullptr,
                                                                          0,
                                                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                                                          attachmentWidth,
                                                                          attachmentHeight,
                                                                          1,
                                                                          1,
                                                                          &attachmentFormat};
    VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo = vku::InitStructHelper();
    framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
    framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
    VkFramebufferCreateInfo framebufferCreateInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                     &framebufferAttachmentsCreateInfo,
                                                     VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR,
                                                     rp.Handle(),
                                                     1,
                                                     reinterpret_cast<const VkImageView *>(1),
                                                     attachmentWidth,
                                                     attachmentHeight,
                                                     1};
    vkt::Framebuffer framebuffer(*m_device, framebufferCreateInfo);

    VkRenderPassAttachmentBeginInfoKHR renderPassAttachmentBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO_KHR,
                                                                        nullptr, 1, &imageView.handle()};
    VkRenderPassBeginInfo renderPassBeginInfo =
        vku::InitStruct<VkRenderPassBeginInfo>(&renderPassAttachmentBeginInfo, rp.Handle(), framebuffer.handle(),
                                               VkRect2D{{0, 0}, {attachmentWidth, attachmentHeight}}, 0u, nullptr);

    // RenderPass should change the image layout of both the swapchain image and the aliased image to PRESENT_SRC_KHR
    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(renderPassBeginInfo);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();

    VkPresentInfoKHR present = vku::InitStructHelper();
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &image_acquired.handle();
    present.pSwapchains = &m_swapchain;
    present.pImageIndices = &current_buffer;
    present.swapchainCount = 1;
    vk::QueuePresentKHR(m_default_queue->handle(), &present);
    m_default_queue->Wait();
}

TEST_F(PositiveImage, ExtendedUsageWithDifferentFormatViews) {
    TEST_DESCRIPTION("Create views with different formats of an image with extended usage");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags =
        VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT | VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_BC3_UNORM_BLOCK;
    image_ci.extent = {
        64,  // width
        64,  // height
        1,   // depth
    };
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkImageFormatProperties image_properties;
    VkResult err = vk::GetPhysicalDeviceImageFormatProperties(gpu(), image_ci.format, image_ci.imageType, image_ci.tiling,
                                                              image_ci.usage, image_ci.flags, &image_properties);
    // Test not supported by driver
    if (err != VK_SUCCESS) {
        GTEST_SKIP() << "Image format not valid for format, type, tiling, usage and flags combination.";
    }

    vkt::Image image(*m_device, image_ci);
    ASSERT_TRUE(image.handle() != VK_NULL_HANDLE);

    // Since the format is compatible with all image's usage, there's no need to restrict usage
    VkImageViewCreateInfo iv_ci = vku::InitStructHelper();
    iv_ci.image = image.handle();
    iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv_ci.format = VK_FORMAT_R32G32B32A32_UINT;
    iv_ci.subresourceRange.layerCount = 1;
    iv_ci.subresourceRange.baseMipLevel = 0;
    iv_ci.subresourceRange.levelCount = 1;
    iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkt::ImageView view(*m_device, iv_ci);
    ASSERT_TRUE(view.handle() != VK_NULL_HANDLE);

    // Since usage is inherited from the image, we need to restrict the usage to a subset
    // Compressed images do not support storage, but we want to sample from the compressed
    VkImageViewUsageCreateInfo ivu_ci = vku::InitStructHelper();
    ivu_ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    iv_ci.pNext = &ivu_ci;
    vkt::ImageView view2(*m_device, iv_ci);
    ASSERT_TRUE(view2.handle() != VK_NULL_HANDLE);
}

TEST_F(PositiveImage, ImageCompressionControl) {
    TEST_DESCRIPTION("Checks for creating fixed rate compression image.");

    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_IMAGE_COMPRESSION_CONTROL_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageCompressionControl);
    RETURN_IF_SKIP(Init());

    // Query possible image format with vkGetPhysicalDeviceImageFormatProperties2KHR
    VkPhysicalDeviceImageFormatInfo2 image_format_info = vku::InitStructHelper();
    image_format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_format_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkImageCompressionPropertiesEXT compression_properties = vku::InitStructHelper();
    VkImageFormatProperties2 image_format_properties = vku::InitStructHelper(&compression_properties);

    vk::GetPhysicalDeviceImageFormatProperties2(gpu(), &image_format_info, &image_format_properties);

    auto image_ci = vkt::Image::CreateInfo();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.tiling = VK_IMAGE_TILING_LINEAR;
    image_ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    // Create with disabled image compression
    {
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_DISABLED_EXT;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }

    // Create with default image compression, without fixed rate
    {
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_DEFAULT_EXT;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }

    // Create with fixed rate compression image
    {
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }

    // Create with fixed rate compression image
    if (compression_properties.imageCompressionFixedRateFlags != VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT) {
        VkImageCompressionFixedRateFlagsEXT supported_compression_fixed_rate = VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT;

        for (uint32_t index = 0; index < 32; index++) {
            if ((compression_properties.imageCompressionFixedRateFlags & (1 << index)) != 0) {
                supported_compression_fixed_rate = (1 << index);
                break;
            }
        }

        ASSERT_TRUE(supported_compression_fixed_rate != VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT);

        VkImageCompressionFixedRateFlagsEXT fixedRageFlags = supported_compression_fixed_rate;
        VkImageCompressionControlEXT compressionControl = vku::InitStructHelper();
        compressionControl.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT;
        compressionControl.pFixedRateFlags = &fixedRageFlags;
        compressionControl.compressionControlPlaneCount = 1;
        image_ci.pNext = &compressionControl;

        vkt::Image image(*m_device, image_ci);
    }
}

TEST_F(PositiveImage, Create3DImageView) {
    TEST_DESCRIPTION("Create a 3D image view");

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView(VK_IMAGE_VIEW_TYPE_3D);
}

TEST_F(PositiveImage, SlicedCreateInfo) {
    TEST_DESCRIPTION("Test VkImageViewSlicedCreateInfoEXT");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageSlicedViewOf3D);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_3D;
    ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ci.extent = {32, 32, 8};
    ci.mipLevels = 6;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, ci, vkt::set_layout);

    VkImageViewSlicedCreateInfoEXT sliced_info = vku::InitStructHelper();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&sliced_info);
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
    ivci.format = ci.format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.layerCount = 1;

    auto get_effective_depth = [&]() -> uint32_t {
        return GetEffectiveExtent(ci, ivci.subresourceRange.aspectMask, ivci.subresourceRange.baseMipLevel).depth;
    };

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 8;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 4;
        sliced_info.sliceOffset = 4;
        ivci.subresourceRange.baseMipLevel = 0;
        ASSERT_TRUE(get_effective_depth() == 8);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 4;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 1;
        ASSERT_TRUE(get_effective_depth() == 4);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = 2;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.baseMipLevel = 2;
        ASSERT_TRUE(get_effective_depth() == 2);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        ivci.subresourceRange.baseMipLevel = 5;
        ASSERT_TRUE(get_effective_depth() == 1);

        vkt::ImageView image_view(*m_device, ivci);
    }

    {
        sliced_info.sliceCount = VK_REMAINING_3D_SLICES_EXT;
        sliced_info.sliceOffset = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.baseMipLevel = 5;
        ASSERT_TRUE(get_effective_depth() == 1);

        vkt::ImageView image_view(*m_device, ivci);
    }
}

TEST_F(PositiveImage, DescriptorSubresourceLayout) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create image, view, and sampler
    const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    auto usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 5, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    VkImageSubresourceRange view_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 3, 1};
    VkImageSubresourceRange first_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageSubresourceRange full_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 5};
    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange = view_range;

    vkt::ImageView view(*m_device, image_view_create_info);
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer cmd_buf(*m_device, m_command_pool);

    enum TestType {
        kInternal,  // Image layout mismatch is *within* a given command buffer
        kExternal   // Image layout mismatch is with the current state of the image, found at QueueSubmit
    };
    std::array<TestType, 2> test_list = {{kInternal, kExternal}};

    for (TestType test_type : test_list) {
        auto init_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        VkImageMemoryBarrier image_barrier = vku::InitStructHelper();

        cmd_buf.begin();
        image_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.image = image.handle();
        image_barrier.subresourceRange = full_range;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = init_layout;

        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        image_barrier.subresourceRange = first_range;
        image_barrier.oldLayout = init_layout;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        image_barrier.subresourceRange = view_range;
        image_barrier.oldLayout = init_layout;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        if (test_type == kExternal) {
            // The image layout is external to the command buffer we are recording to test.  Submit to push to instance scope.
            cmd_buf.end();
            m_default_queue->Submit(cmd_buf);
            m_default_queue->Wait();
            cmd_buf.begin();
        }

        cmd_buf.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, NULL);

        vk::CmdDraw(cmd_buf.handle(), 1, 0, 0, 0);

        cmd_buf.EndRenderPass();
        cmd_buf.end();

        // Submit cmd buffer
        m_default_queue->Submit(cmd_buf);
        m_default_queue->Wait();
    }
}

TEST_F(PositiveImage, Descriptor3D2DSubresourceLayout) {
    TEST_DESCRIPTION("Verify renderpass layout transitions for a 2d ImageView created from a 3d Image.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(Init());
    InitRenderTarget();

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    // Create image, view, and sampler
    const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    auto usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    static const uint32_t kWidth = 128;
    static const uint32_t kHeight = 128;

    VkImageCreateInfo image_ci_3d = vku::InitStructHelper();
    image_ci_3d.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci_3d.imageType = VK_IMAGE_TYPE_3D;
    image_ci_3d.format = format;
    image_ci_3d.extent.width = kWidth;
    image_ci_3d.extent.height = kHeight;
    image_ci_3d.extent.depth = 8;
    image_ci_3d.mipLevels = 1;
    image_ci_3d.arrayLayers = 1;
    image_ci_3d.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci_3d.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci_3d.usage = usage;
    vkt::Image image_3d(*m_device, image_ci_3d, vkt::set_layout);

    vkt::Image other_image(*m_device, kWidth, kHeight, 1, format, usage);

    // The image view is a 2D slice of the 3D image at depth = 4, which we request by
    // asking for arrayLayer = 4
    VkImageSubresourceRange view_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 4, 1};
    // But, the spec says:
    //    Automatic layout transitions apply to the entire image subresource attached
    //    to the framebuffer. If the attachment view is a 2D or 2D array view of a
    //    3D image, even if the attachment view only refers to a subset of the slices
    //    of the selected mip level of the 3D image, automatic layout transitions apply
    //    to the entire subresource referenced which is the entire mip level in this case.
    VkImageSubresourceRange full_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vkt::ImageView view_2d, other_view;
    VkImageViewCreateInfo image_view_create_info = vku::InitStructHelper();
    image_view_create_info.image = image_3d.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange = view_range;

    view_2d.init(*m_device, image_view_create_info);
    ASSERT_TRUE(view_2d.initialized());

    image_view_create_info.image = other_image.handle();
    image_view_create_info.subresourceRange = full_range;
    other_view.init(*m_device, image_view_create_info);
    ASSERT_TRUE(other_view.initialized());

    std::vector<VkAttachmentDescription> attachments = {
        {0, format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
    };

    std::vector<VkAttachmentReference> color = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    VkSubpassDescription subpass = {
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, (uint32_t)color.size(), color.data(), nullptr, nullptr, 0, nullptr};

    std::vector<VkSubpassDependency> deps = {
        {VK_SUBPASS_EXTERNAL, 0,
         (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT),
         (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
         (VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
          VK_ACCESS_TRANSFER_WRITE_BIT),
         (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT), 0},
        {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         (VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT), VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT), 0},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   (uint32_t)attachments.size(),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   (uint32_t)deps.size(),
                                   deps.data()};
    // Create Sampler
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    vkt::Sampler sampler(*m_device, sampler_ci);

    descriptor_set.WriteDescriptorImageInfo(0, other_view, sampler);
    descriptor_set.UpdateDescriptorSets();

    vkt::RenderPass rp(*m_device, rpci);

    // Create PSO to be used for draw-time errors below
    VkShaderObj fs(this, kFragmentSamplerGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[1] = fs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.CreateGraphicsPipeline();

    vkt::CommandBuffer cmd_buf(*m_device, m_command_pool);

    enum TestType {
        kInternal,  // Image layout mismatch is *within* a given command buffer
        kExternal   // Image layout mismatch is with the current state of the image, found at QueueSubmit
    };
    std::array<TestType, 2> test_list = {{kInternal, kExternal}};

    for (TestType test_type : test_list) {
        VkImageMemoryBarrier image_barrier = vku::InitStructHelper();

        vkt::Framebuffer fb(*m_device, rp.handle(), 1, &view_2d.handle(), kWidth, kHeight);

        cmd_buf.begin();
        image_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        image_barrier.image = image_3d.handle();
        image_barrier.subresourceRange = full_range;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);
        image_barrier.image = other_image.handle();
        vk::CmdPipelineBarrier(cmd_buf.handle(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &image_barrier);

        if (test_type == kExternal) {
            // The image layout is external to the command buffer we are recording to test.  Submit to push to instance scope.
            cmd_buf.end();
            m_default_queue->Submit(cmd_buf);
            m_default_queue->Wait();
            cmd_buf.begin();
        }

        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();
        m_renderPassBeginInfo.renderArea = {{0, 0}, {kWidth, kHeight}};

        cmd_buf.BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);
        vk::CmdDraw(cmd_buf.handle(), 1, 0, 0, 0);

        cmd_buf.EndRenderPass();
        cmd_buf.end();

        // Submit cmd buffer
        m_default_queue->Submit(cmd_buf);
        m_default_queue->Wait();
    }
}

TEST_F(PositiveImage, BlitRemainingArrayLayers) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance5);
    RETURN_IF_SKIP(Init());
    ;

    VkFormat f_color = VK_FORMAT_R32_SFLOAT;  // Need features ..BLIT_SRC_BIT & ..BLIT_DST_BIT
    if (!FormatFeaturesAreSupported(gpu(), f_color, VK_IMAGE_TILING_OPTIMAL,
                                    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        GTEST_SKIP() << "No blit feature format support";
    }

    VkImageCreateInfo ci = vku::InitStructHelper();
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = f_color;
    ci.extent = {64, 64, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 4;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkt::Image image(*m_device, ci, vkt::set_layout);

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 2, VK_REMAINING_ARRAY_LAYERS};
    blitRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, VK_REMAINING_ARRAY_LAYERS};
    blitRegion.srcOffsets[0] = {0, 0, 0};
    blitRegion.srcOffsets[1] = {16, 16, 1};
    blitRegion.dstOffsets[0] = {32, 32, 0};
    blitRegion.dstOffsets[1] = {64, 64, 1};

    m_command_buffer.begin();

    vk::CmdBlitImage(m_command_buffer.handle(), image.handle(), image.Layout(), image.handle(), image.Layout(), 1, &blitRegion,
                     VK_FILTER_NEAREST);

    blitRegion.dstSubresource.layerCount = 2;  // same as VK_REMAINING_ARRAY_LAYERS
    vk::CmdBlitImage(m_command_buffer.handle(), image.handle(), image.Layout(), image.handle(), image.Layout(), 1, &blitRegion,
                     VK_FILTER_NEAREST);
}

TEST_F(PositiveImage, BlockTexelViewCompatibleMultipleLayers) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance6);
    RETURN_IF_SKIP(Init());

    VkPhysicalDeviceMaintenance6PropertiesKHR maintenance6_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(maintenance6_props);
    if (!maintenance6_props.blockTexelViewCompatibleMultipleLayers) {
        GTEST_SKIP() << "blockTexelViewCompatibleMultipleLayers is not enabled";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
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
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!vkt::Image::IsCompatible(*m_device, image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        GTEST_SKIP() << "Image usage and format not compatible on device";
    }
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.layerCount = 2;
    vkt::ImageView view(*m_device, ivci);
}

TEST_F(PositiveImage, ImageAlignmentControl) {
    AddRequiredExtensions(VK_MESA_IMAGE_ALIGNMENT_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageAlignmentControl);
    RETURN_IF_SKIP(Init());

    const uint32_t alignment = 0x1;
    VkPhysicalDeviceImageAlignmentControlPropertiesMESA props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(props);
    if (!(props.supportedImageAlignmentMask & alignment)) {
        GTEST_SKIP() << "supportedImageAlignmentMask doesn't support testing alignment";
    }
    VkImageAlignmentControlCreateInfoMESA alignment_control = vku::InitStructHelper();
    alignment_control.maximumRequestedAlignment = alignment;

    VkImageCreateInfo image_create_info = DefaultImageInfo();
    image_create_info.pNext = &alignment_control;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
}

TEST_F(PositiveImage, ImageAlignmentControlZero) {
    AddRequiredExtensions(VK_MESA_IMAGE_ALIGNMENT_CONTROL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::imageAlignmentControl);
    RETURN_IF_SKIP(Init());

    VkImageAlignmentControlCreateInfoMESA alignment_control = vku::InitStructHelper();
    alignment_control.maximumRequestedAlignment = 0;  // Should ignore

    VkImageCreateInfo image_create_info = DefaultImageInfo();
    image_create_info.pNext = &alignment_control;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
}

TEST_F(PositiveImage, RemainingMipLevels2DViewOf3D) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_IMAGE_2D_VIEW_OF_3D_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT | VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 2;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkt::Image image(*m_device, image_ci, vkt::set_layout);

    vkt::ImageView view = image.CreateView(VK_IMAGE_VIEW_TYPE_2D, 0, VK_REMAINING_MIP_LEVELS, 0, 1);
}

TEST_F(PositiveImage, RemainingMipLevelsBlockTexelView) {
    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    image_create_info.extent.width = 32;
    image_create_info.extent.height = 32;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 2;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VkFormatProperties image_fmt;
    vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), image_create_info.format, &image_fmt);
    if (!vkt::Image::IsCompatible(*m_device, image_create_info.usage, image_fmt.optimalTilingFeatures)) {
        GTEST_SKIP() << "Image usage and format not compatible on device";
    }
    vkt::Image image(*m_device, image_create_info, vkt::set_layout);

    VkImageViewCreateInfo ivci = vku::InitStructHelper();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R16G16B16A16_UNORM;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 1};
    CreateImageViewTest(*this, &ivci);
}
