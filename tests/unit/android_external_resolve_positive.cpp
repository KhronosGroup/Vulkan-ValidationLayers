/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/android_hardware_buffer.h"
#include "../framework/pipeline_helper.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)

void AndroidExternalResolveTest::InitBasicAndroidExternalResolve(void* pNextFeatures) {
    SetTargetApiVersion(VK_API_VERSION_1_2); // for RenderPass2
    AddRequiredExtensions(VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper(pNextFeatures);
    VkPhysicalDeviceExternalFormatResolveFeaturesANDROID external_format_resolve_features = vku::InitStructHelper(&features11);
    GetPhysicalDeviceFeatures2(external_format_resolve_features);
    if (external_format_resolve_features.externalFormatResolve == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) externalFormatResolve";
    }
    if (features11.samplerYcbcrConversion == VK_FALSE) {
        GTEST_SKIP() << "Test requires (unsupported) samplerYcbcrConversion";
    }

    RETURN_IF_SKIP(InitState(nullptr, &external_format_resolve_features));

    VkPhysicalDeviceExternalFormatResolvePropertiesANDROID external_format_resolve_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(external_format_resolve_props);
    nullColorAttachmentWithExternalFormatResolve = external_format_resolve_props.nullColorAttachmentWithExternalFormatResolve;
}

TEST_F(PositiveAndroidExternalResolve, RenderPassAndFramebuffer) {
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve())

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    // index 0 = color | index 1 = resolve
    VkAttachmentDescription2 attachment_desc[2];
    attachment_desc[0] = vku::InitStructHelper(&external_format);
    attachment_desc[0].format = format_resolve_prop.colorAttachmentFormat;
    attachment_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    attachment_desc[1] = attachment_desc[0];
    attachment_desc[1].format = VK_FORMAT_UNDEFINED;

    VkAttachmentReference2 color_attachment_ref = vku::InitStructHelper();
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment_ref.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    color_attachment_ref.attachment = 0;

    VkAttachmentReference2 resolve_attachment_ref = vku::InitStructHelper();
    resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve_attachment_ref.attachment = 1;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pResolveAttachments = &resolve_attachment_ref;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.attachmentCount = 2;
    render_pass_ci.pAttachments = attachment_desc;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj color_image(m_device);
    color_image.Init(image_ci);

    image_ci.pNext = &external_format;
    image_ci.format = VK_FORMAT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_ci);

    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&external_format);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkt::SamplerYcbcrConversion ycbcr_conv(*m_device, sycci);

    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageView attachments[2];
    attachments[0] = color_image.targetView(format_resolve_prop.colorAttachmentFormat);
    attachments[1] = resolve_image.targetView(ivci);

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper();
    fb_ci.width = 32;
    fb_ci.height = 32;
    fb_ci.layers = 1;
    fb_ci.renderPass = render_pass.handle();
    fb_ci.attachmentCount = 2;
    fb_ci.pAttachments = attachments;
    vkt::Framebuffer framebuffer(*m_device, fb_ci);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.gp_ci_.pNext = &external_format;
    pipe.gp_ci_.renderPass = render_pass.handle();
    pipe.CreateGraphicsPipeline();
}

TEST_F(PositiveAndroidExternalResolve, ImagelessFramebuffer) {
    VkPhysicalDeviceImagelessFramebufferFeatures imageless_framebuffer = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve(&imageless_framebuffer))

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    // index 0 = color | index 1 = resolve
    VkAttachmentDescription2 attachment_desc[2];
    attachment_desc[0] = vku::InitStructHelper(&external_format);
    attachment_desc[0].format = format_resolve_prop.colorAttachmentFormat;
    attachment_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    attachment_desc[1] = attachment_desc[0];
    attachment_desc[1].format = VK_FORMAT_UNDEFINED;

    VkAttachmentReference2 color_attachment_ref = vku::InitStructHelper();
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attachment_ref.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT;
    color_attachment_ref.attachment = 0;

    VkAttachmentReference2 resolve_attachment_ref = vku::InitStructHelper();
    resolve_attachment_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    resolve_attachment_ref.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    resolve_attachment_ref.attachment = 1;

    VkSubpassDescription2 subpass = vku::InitStructHelper();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pResolveAttachments = &resolve_attachment_ref;

    VkRenderPassCreateInfo2 render_pass_ci = vku::InitStructHelper();
    render_pass_ci.attachmentCount = 2;
    render_pass_ci.pAttachments = attachment_desc;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    vkt::RenderPass render_pass(*m_device, render_pass_ci);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj color_image(m_device);
    color_image.Init(image_ci);

    image_ci.pNext = &external_format;
    image_ci.format = VK_FORMAT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_ci);

    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&external_format);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkt::SamplerYcbcrConversion ycbcr_conv(*m_device, sycci);

    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageView attachments[2];
    attachments[0] = color_image.targetView(format_resolve_prop.colorAttachmentFormat);
    attachments[1] = resolve_image.targetView(ivci);

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info[2];
    framebuffer_attachment_image_info[0] = vku::InitStructHelper();
    framebuffer_attachment_image_info[0].usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info[0].width = 32;
    framebuffer_attachment_image_info[0].height = 32;
    framebuffer_attachment_image_info[0].layerCount = 1;
    framebuffer_attachment_image_info[0].viewFormatCount = 1;
    framebuffer_attachment_image_info[0].pViewFormats = &format_resolve_prop.colorAttachmentFormat;

    framebuffer_attachment_image_info[1] = framebuffer_attachment_image_info[0];
    framebuffer_attachment_image_info[1].usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    framebuffer_attachment_image_info[1].pViewFormats = &ivci.format;

    VkFramebufferAttachmentsCreateInfo framebuffer_attachments = vku::InitStructHelper();
    framebuffer_attachments.attachmentImageInfoCount = 2;
    framebuffer_attachments.pAttachmentImageInfos = framebuffer_attachment_image_info;

    VkFramebufferCreateInfo fb_ci = vku::InitStructHelper(&framebuffer_attachments);
    fb_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    fb_ci.width = 32;
    fb_ci.height = 32;
    fb_ci.layers = 1;
    fb_ci.attachmentCount = 2;
    fb_ci.renderPass = render_pass.handle();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkFramebufferCreateInfo-flags-03201");
    vkt::Framebuffer framebuffer(*m_device, fb_ci);

    VkClearValue clear_value = {};
    clear_value.color = {{0u, 0u, 0u, 0u}};

    VkRenderPassAttachmentBeginInfo render_pass_attachment_bi = vku::InitStructHelper();
    render_pass_attachment_bi.attachmentCount = 2;
    render_pass_attachment_bi.pAttachments = attachments;

    VkRenderPassBeginInfo render_pass_bi = vku::InitStructHelper(&render_pass_attachment_bi);
    render_pass_bi.renderPass = render_pass.handle();
    render_pass_bi.framebuffer = framebuffer.handle();
    render_pass_bi.renderArea.extent = {1, 1};
    render_pass_bi.clearValueCount = 1;
    render_pass_bi.pClearValues = &clear_value;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_bi);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(PositiveAndroidExternalResolve, DynamicRendering) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_features = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicAndroidExternalResolve(&dynamic_rendering_features))

    if (nullColorAttachmentWithExternalFormatResolve) {
        GTEST_SKIP() << "nullColorAttachmentWithExternalFormatResolve enabled";
    }

    vkt::AHB ahb(AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 64, 64);
    if (!ahb.handle()) {
        GTEST_SKIP() << "could not allocate AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420";
    }

    VkAndroidHardwareBufferFormatResolvePropertiesANDROID format_resolve_prop = vku::InitStructHelper();

    VkExternalFormatANDROID external_format = vku::InitStructHelper();
    external_format.externalFormat = ahb.GetExternalFormat(*m_device, &format_resolve_prop);

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format_resolve_prop.colorAttachmentFormat;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_4_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj color_image(m_device);
    color_image.Init(image_ci);

    image_ci.pNext = &external_format;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.format = VK_FORMAT_UNDEFINED;
    image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageObj resolve_image(m_device);
    resolve_image.Init(image_ci);

    VkSamplerYcbcrConversionCreateInfo sycci = vku::InitStructHelper(&external_format);
    sycci.format = VK_FORMAT_UNDEFINED;
    sycci.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    sycci.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
    sycci.components = {VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO};
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650");
    vkt::SamplerYcbcrConversion ycbcr_conv(*m_device, sycci);

    VkSamplerYcbcrConversionInfo syci = vku::InitStructHelper();
    syci.conversion = ycbcr_conv.handle();

    VkImageViewCreateInfo ivci = vku::InitStructHelper(&syci);
    ivci.image = resolve_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = VK_FORMAT_UNDEFINED;
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkRenderingAttachmentInfoKHR color_attachment = vku::InitStructHelper();
    color_attachment.imageView = color_image.targetView(format_resolve_prop.colorAttachmentFormat);
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_EXTERNAL_FORMAT_DOWNSAMPLE_ANDROID;
    color_attachment.resolveImageView = resolve_image.targetView(ivci);
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingInfoKHR begin_rendering_info = vku::InitStructHelper();
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.renderArea = {{0, 0}, {32, 32}};
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06865");
    m_errorMonitor->SetAllowedFailureMsg("VUID-VkRenderingAttachmentInfo-imageView-06129");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();
}

#endif  // VK_USE_PLATFORM_ANDROID_KHR
