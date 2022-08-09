/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2021 ARM, Inc. All rights reserved.
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
 * Author: Quentin Huot-Marchand <quentin.huot-marchand@arm.com
 */

#include "cast_utils.h"
#include "layer_validation_tests.h"

TEST_F(VkLayerTest, ValidationArrayOOBRayTracingShaders) {
    TEST_DESCRIPTION(
        "Core validation: Verify detection of out-of-bounds descriptor array indexing and use of uninitialized descriptors for "
        "ray tracing shaders.");
    OOBRayTracingShadersTestBody(false);
}

TEST_F(VkLayerTest, InvalidDescriptorPoolConsistency) {
    VkResult err;

    TEST_DESCRIPTION("Allocate descriptor sets from one DS pool and attempt to delete them from another.");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeDescriptorSets-pDescriptorSets-parent");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool bad_pool;
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &bad_pool);
    ASSERT_VK_SUCCESS(err);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    err = vk::FreeDescriptorSets(m_device->device(), bad_pool, 1, &descriptor_set.set_);

    m_errorMonitor->VerifyFound();

    vk::DestroyDescriptorPool(m_device->device(), bad_pool, NULL);
}

TEST_F(VkLayerTest, BadSubpassIndices) {
    TEST_DESCRIPTION("Create render pass with valid stages");

    bool rp2_supported = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (rp2_supported) m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (rp2_supported) rp2_supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription sci[2] = {};
    sci[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sci[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    const VkPipelineStageFlags kGraphicsStages =
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    VkSubpassDependency dependency = {};
    // Use only 2 subpasses, so these values should trigger validation errors
    dependency.srcSubpass = 4;
    dependency.dstSubpass = 4;
    dependency.srcStageMask = kGraphicsStages;
    dependency.dstStageMask = kGraphicsStages;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 2;
    rpci.pSubpasses = sci;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;

    VkRenderPass render_pass = VK_NULL_HANDLE;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo-pDependencies-06866");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo-pDependencies-06867");
    vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    if (rp2_supported) {
        PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
            (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");
        safe_VkRenderPassCreateInfo2 create_info2;
        ConvertVkRenderPassCreateInfoToV2KHR(rpci, &create_info2);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-srcSubpass-02526");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-dstSubpass-02527");
        vkCreateRenderPass2KHR(m_device->device(), create_info2.ptr(), nullptr, &render_pass);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DrawWithPipelineIncompatibleWithSubpass) {
    TEST_DESCRIPTION("Use a pipeline for the wrong subpass in a render pass instance");

    ASSERT_NO_FATAL_FAILURE(Init());

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkSubpassDependency dep = {0,
                               1,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 2, subpasses, 1, &dep};
    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &imageView, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    const VkPipelineLayoutObj pl(m_device);
    pipe.CreateVKPipeline(pl.handle(), rp);

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp,
                                  fb,
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};

    // subtest 1: bind in the wrong subpass
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "built for subpass 0 but used in subpass 1");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndRenderPass(m_commandBuffer->handle());

    // subtest 2: bind in correct subpass, then transition to next subpass
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "built for subpass 0 but used in subpass 1");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndRenderPass(m_commandBuffer->handle());

    m_commandBuffer->end();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, ImageBarrierSubpassConflict) {
    TEST_DESCRIPTION("Check case where subpass index references different image from image barrier");
    ASSERT_NO_FATAL_FAILURE(Init());

    // Create RP/FB combo where subpass has incorrect index attachment, this is 2nd half of "VUID-vkCmdPipelineBarrier-image-02635"
    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    // ref attachment points to wrong attachment index compared to img_barrier below
    VkAttachmentReference ref = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
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

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, attach, 1, subpasses, 1, &dep};
    VkRenderPass rp;

    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    VkImageObj image2(m_device);
    image2.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView2 = image2.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    // re-use imageView from start of test
    VkImageView iv_array[2] = {imageView, imageView2};

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 2, iv_array, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp,
                                  fb,
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};

    VkImageMemoryBarrier img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
    img_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    img_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    img_barrier.image = image.handle(); /* barrier references image from attachment index 0 */
    img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    img_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_barrier.subresourceRange.baseArrayLayer = 0;
    img_barrier.subresourceRange.baseMipLevel = 0;
    img_barrier.subresourceRange.layerCount = 1;
    img_barrier.subresourceRange.levelCount = 1;
    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdPipelineBarrier-image-04073");
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1,
                           &img_barrier);
    m_errorMonitor->VerifyFound();

    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentIndexOutOfRange) {
    SetTargetApiVersion(VK_API_VERSION_1_2);
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    // There are no attachments, but refer to attachment 0.
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, subpasses, 0, nullptr};

    // "... must be less than the total number of attachments ..."
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-attachment-00834",
                         "VUID-VkRenderPassCreateInfo2-attachment-03051");
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentReadOnlyButCleared) {
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool maintenance2Supported = false;

    // Check for VK_KHR_maintenance2
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        maintenance2Supported = true;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        maintenance2Supported = true;
    }

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    VkAttachmentDescription description = {0,
                                           ds_format,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // loadOp
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // stencilLoadOp
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &description, 1, &subpass, 0, nullptr};

    // Test both cases when rp2 is not supported

    // Set loadOp to clear
    description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pAttachments-00836",
                         "VUID-VkRenderPassCreateInfo2-pAttachments-02522");

    if (maintenance2Supported == true) {
        depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                             "VUID-VkRenderPassCreateInfo-pAttachments-01566", "VUID-VkRenderPassCreateInfo2-pAttachments-02522");
    }

    description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // reset

    // Set stencilLoadOp to clear
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pAttachments-02511",
                         "VUID-VkRenderPassCreateInfo2-pAttachments-02523");

    if (maintenance2Supported == true) {
        depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                             "VUID-VkRenderPassCreateInfo-pAttachments-01567", "VUID-VkRenderPassCreateInfo2-pAttachments-02523");
    }

    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;  // reset
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentMismatchingLayoutsColor) {
    TEST_DESCRIPTION("Attachment is used simultaneously as two color attachments with different layouts.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference refs[] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 2, refs, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpasses, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "subpass 0 already uses attachment 0 with a different image layout",
                         "subpass 0 already uses attachment 0 with a different image layout");
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentDescriptionInvalidFinalLayout) {
    TEST_DESCRIPTION("VkAttachmentDescription's finalLayout must not be UNDEFINED or PREINITIALIZED");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    }
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

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-03061");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-finalLayout-00843",
                         "VUID-VkAttachmentDescription2-finalLayout-03061");

    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto depth_format = FindSupportedDepthOnlyFormat(gpu());
    auto stencil_format = FindSupportedStencilOnlyFormat(gpu());
    auto depth_stencil_format = FindSupportedDepthStencilFormat(gpu());

    if (separate_depth_stencil_layouts_features.separateDepthStencilLayouts) {
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                             "VUID-VkAttachmentDescription2-format-03300");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                             "VUID-VkAttachmentDescription2-format-03300");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                             "VUID-VkAttachmentDescription2-format-03300");
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03286",
                             "VUID-VkAttachmentDescription2-format-03300");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                             "VUID-VkAttachmentDescription2-format-03301");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                             "VUID-VkAttachmentDescription2-format-03301");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                             "VUID-VkAttachmentDescription2-format-03301");
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03287",
                             "VUID-VkAttachmentDescription2-format-03301");

        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

        if (depth_stencil_format) {
            attach_desc.format = depth_stencil_format;

            if (rp2Supported) {
                safe_VkRenderPassCreateInfo2 rpci2;

                attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                ConvertVkRenderPassCreateInfoToV2KHR(rpci, &rpci2);
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentDescription2-format-03302");
                attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                ConvertVkRenderPassCreateInfoToV2KHR(rpci, &rpci2);
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentDescription2-format-03302");
            } else {
                attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03288", "VUID-VkAttachmentDescription2-format-03302");
                attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03288", "VUID-VkAttachmentDescription2-format-03302");
                attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03288", "VUID-VkAttachmentDescription2-format-03302");
                attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03288", "VUID-VkAttachmentDescription2-format-03302");
            }

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

            if (rp2Supported) {
                safe_VkRenderPassCreateInfo2 rpci2;

                attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                ConvertVkRenderPassCreateInfoToV2KHR(rpci, &rpci2);
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentDescription2-format-03303");
                attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                ConvertVkRenderPassCreateInfoToV2KHR(rpci, &rpci2);
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentDescription2-format-03303");
            } else {
                attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03289", "VUID-VkAttachmentDescription2-format-03303");
                attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03289", "VUID-VkAttachmentDescription2-format-03303");
                attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03289", "VUID-VkAttachmentDescription2-format-03303");
                attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
                TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                     "VUID-VkAttachmentDescription-format-03289", "VUID-VkAttachmentDescription2-format-03303");
            }

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        if (depth_format) {
            attach_desc.format = depth_format;

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03290", "VUID-VkAttachmentDescription2-format-03304");
            attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03290", "VUID-VkAttachmentDescription2-format-03304");

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03291", "VUID-VkAttachmentDescription2-format-03305");
            attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03291", "VUID-VkAttachmentDescription2-format-03305");

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        if (stencil_format) {
            attach_desc.format = stencil_format;

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03292", "VUID-VkAttachmentDescription2-format-03306");
            attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03292", "VUID-VkAttachmentDescription2-format-03306");

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03293", "VUID-VkAttachmentDescription2-format-03307");
            attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-format-03293", "VUID-VkAttachmentDescription2-format-03307");

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        if (rp2Supported && depth_stencil_format) {
            attach_desc.format = depth_stencil_format;
            attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;

            auto attachment_description_stencil_layout = LvlInitStruct<VkAttachmentDescriptionStencilLayoutKHR>();
            attachment_description_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_GENERAL;
            attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_GENERAL;
            safe_VkRenderPassCreateInfo2 rpci2;
            ConvertVkRenderPassCreateInfoToV2KHR(rpci, &rpci2);
            rpci2.pAttachments[0].pNext = &attachment_description_stencil_layout;

            VkImageLayout forbidden_layouts[] = {
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR,
                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
            };
            auto forbidden_layouts_array_size = sizeof(forbidden_layouts) / sizeof(forbidden_layouts[0]);

            for (size_t i = 0; i < forbidden_layouts_array_size; ++i) {
                attachment_description_stencil_layout.stencilInitialLayout = forbidden_layouts[i];
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentDescriptionStencilLayout-stencilInitialLayout-03308");
            }
            attachment_description_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            for (size_t i = 0; i < forbidden_layouts_array_size; ++i) {
                attachment_description_stencil_layout.stencilFinalLayout = forbidden_layouts[i];
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03309");
            }
            attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310");
            attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentDescriptionStencilLayout-stencilFinalLayout-03310");

            rpci2.pAttachments[0].pNext = nullptr;
        }
    } else {
        if (depth_format) {
            attach_desc.format = depth_format;

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03298");
            attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03298");

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03299");
            attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03299");

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        if (stencil_format) {
            attach_desc.format = stencil_format;

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03298");
            attach_desc.initialLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03284",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03298");

            attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03299");
            attach_desc.finalLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                                 "VUID-VkAttachmentDescription-separateDepthStencilLayouts-03285",
                                 "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-03299");

            attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    // Test invalid layouts for color formats
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03280",
                         "VUID-VkAttachmentDescription2-format-03294");

    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03282",
                         "VUID-VkAttachmentDescription2-format-03296");

    // Test invalid layouts for depth/stencil format
    if (depth_stencil_format) {
        attach_desc.format = depth_stencil_format;
        attach_desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03281",
                             "VUID-VkAttachmentDescription2-format-03295");

        attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
        attach_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-03283",
                             "VUID-VkAttachmentDescription2-format-03297");
    }
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentsMisc) {
    TEST_DESCRIPTION(
        "Ensure that CreateRenderPass produces the expected validation errors when a subpass's attachments violate the valid usage "
        "conditions.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    std::vector<VkAttachmentDescription> attachments = {
        // input attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        // color attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth attachment
        {0, ds_format, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        // resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // preserve attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth non-resolve attachment
        {0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
    };

    std::vector<VkAttachmentReference> input = {
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    std::vector<VkAttachmentReference> color = {
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference depth = {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    std::vector<VkAttachmentReference> resolve = {
        {4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    std::vector<uint32_t> preserve = {5};
    std::vector<VkAttachmentReference> depth_1bit = {
        {6, VK_IMAGE_LAYOUT_GENERAL},
        {6, VK_IMAGE_LAYOUT_GENERAL},
    };

    VkSubpassDescription subpass = {0,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    (uint32_t)input.size(),
                                    input.data(),
                                    (uint32_t)color.size(),
                                    color.data(),
                                    resolve.data(),
                                    &depth,
                                    (uint32_t)preserve.size(),
                                    preserve.data()};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   (uint32_t)attachments.size(),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    // Test too many color attachments
    const uint32_t max_color_attachments = m_device->props.limits.maxColorAttachments;
    const uint32_t too_big_max_attachments = 65536 + 1;  // let's say this is too much to allocate
    if (max_color_attachments >= too_big_max_attachments) {
        printf(
            "%s VkPhysicalDeviceLimits::maxColorAttachments is too large to practically test against -- skipping part of test.\n",
            kSkipPrefix);
    } else {
        std::vector<VkAttachmentReference> too_many_colors(max_color_attachments + 1, color[0]);
        VkSubpassDescription test_subpass = subpass;
        test_subpass.colorAttachmentCount = (uint32_t)too_many_colors.size();
        test_subpass.pColorAttachments = too_many_colors.data();
        test_subpass.pResolveAttachments = NULL;
        VkRenderPassCreateInfo test_rpci = rpci;
        test_rpci.pSubpasses = &test_subpass;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &test_rpci, rp2Supported,
                             "VUID-VkSubpassDescription-colorAttachmentCount-00845",
                             "VUID-VkSubpassDescription2-colorAttachmentCount-03063");
    }

    // Test sample count mismatch between color buffers
    attachments[subpass.pColorAttachments[1].attachment].samples = VK_SAMPLE_COUNT_8_BIT;
    depth.attachment = VK_ATTACHMENT_UNUSED;  // Avoids triggering 01418

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pColorAttachments-06868",
                         "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872");

    depth.attachment = 3;
    attachments[subpass.pColorAttachments[1].attachment].samples = attachments[subpass.pColorAttachments[0].attachment].samples;

    // Test sample count mismatch between color buffers and depth buffer
    attachments[subpass.pDepthStencilAttachment->attachment].samples = VK_SAMPLE_COUNT_8_BIT;
    subpass.colorAttachmentCount = 1;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pDepthStencilAttachment-01418",
                         "VUID-VkSubpassDescription2-multisampledRenderToSingleSampled-06872");

    attachments[subpass.pDepthStencilAttachment->attachment].samples = attachments[subpass.pColorAttachments[0].attachment].samples;
    subpass.colorAttachmentCount = (uint32_t)color.size();

    // Test resolve attachment with UNUSED color attachment
    color[0].attachment = VK_ATTACHMENT_UNUSED;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00847",
                         "VUID-VkSubpassDescription2-pResolveAttachments-03065");

    color[0].attachment = 1;

    // Test resolve from a single-sampled color attachment
    attachments[subpass.pColorAttachments[0].attachment].samples = VK_SAMPLE_COUNT_1_BIT;
    subpass.colorAttachmentCount = 1;           // avoid mismatch (00337), and avoid double report
    subpass.pDepthStencilAttachment = nullptr;  // avoid mismatch (01418)

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00848",
                         "VUID-VkSubpassDescription2-pResolveAttachments-03066");

    attachments[subpass.pColorAttachments[0].attachment].samples = VK_SAMPLE_COUNT_4_BIT;
    subpass.colorAttachmentCount = (uint32_t)color.size();
    subpass.pDepthStencilAttachment = &depth;

    // Test resolve to a multi-sampled resolve attachment
    attachments[subpass.pResolveAttachments[0].attachment].samples = VK_SAMPLE_COUNT_4_BIT;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00849",
                         "VUID-VkSubpassDescription2-pResolveAttachments-03067");

    attachments[subpass.pResolveAttachments[0].attachment].samples = VK_SAMPLE_COUNT_1_BIT;

    // Test with color/resolve format mismatch
    attachments[subpass.pColorAttachments[0].attachment].format = VK_FORMAT_R8G8B8A8_SRGB;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pResolveAttachments-00850",
                         "VUID-VkSubpassDescription2-pResolveAttachments-03068");

    attachments[subpass.pColorAttachments[0].attachment].format = attachments[subpass.pResolveAttachments[0].attachment].format;

    // Test for UNUSED preserve attachments
    preserve[0] = VK_ATTACHMENT_UNUSED;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkSubpassDescription-attachment-00853",
                         "VUID-VkSubpassDescription2-attachment-03073");

    preserve[0] = 5;
    // Test for preserve attachments used elsewhere in the subpass
    color[0].attachment = preserve[0];

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pPreserveAttachments-00854",
                         "VUID-VkSubpassDescription2-pPreserveAttachments-03074");

    color[0].attachment = 1;
    input[0].attachment = 0;
    input[0].layout = VK_IMAGE_LAYOUT_GENERAL;

    // Test for attachment used first as input with loadOp=CLEAR
    {
        std::vector<VkSubpassDescription> subpasses = {subpass, subpass, subpass};
        subpasses[0].inputAttachmentCount = 0;
        subpasses[1].inputAttachmentCount = 0;
        attachments[input[0].attachment].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkRenderPassCreateInfo rpci_multipass = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                                 nullptr,
                                                 0,
                                                 (uint32_t)attachments.size(),
                                                 attachments.data(),
                                                 (uint32_t)subpasses.size(),
                                                 subpasses.data(),
                                                 0,
                                                 nullptr};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci_multipass, rp2Supported,
                             "VUID-VkSubpassDescription-loadOp-00846", "VUID-VkSubpassDescription2-loadOp-03064");

        attachments[input[0].attachment].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    // Test for depthStencil and color pointing to same attachment
    {
        // Both use same VkAttachmentReference
        VkSubpassDescription subpass_same = {
            0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, depth_1bit.data(), nullptr, depth_1bit.data(), 0, nullptr};

        VkRenderPassCreateInfo rpci_same = rpci;
        rpci_same.pSubpasses = &subpass_same;

        // only test rp1 so can ignore the expected 2nd error
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription-pColorAttachments-02648");
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci_same, rp2Supported,
                             "VUID-VkSubpassDescription-pDepthStencilAttachment-04438", nullptr);

        if (rp2Supported) {
            safe_VkRenderPassCreateInfo2 create_info2;
            ConvertVkRenderPassCreateInfoToV2KHR(rpci_same, &create_info2);
            m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pColorAttachments-02898");
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), create_info2.ptr(),
                                     "VUID-VkSubpassDescription2-pDepthStencilAttachment-04440");
        }

        // Same test but use 2 different VkAttachmentReference to point to same attachment
        subpass_same.pDepthStencilAttachment = &depth_1bit.data()[1];

        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription-pColorAttachments-02648");
        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci_same, rp2Supported,
                             "VUID-VkSubpassDescription-pDepthStencilAttachment-04438", nullptr);

        if (rp2Supported) {
            safe_VkRenderPassCreateInfo2 create_info2;
            ConvertVkRenderPassCreateInfoToV2KHR(rpci_same, &create_info2);
            m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pColorAttachments-02898");
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), create_info2.ptr(),
                                     "VUID-VkSubpassDescription2-pDepthStencilAttachment-04440");
        }
    }
}

TEST_F(VkLayerTest, InvalidRenderPassCreateRenderPassShaderResolveQCOM) {
    TEST_DESCRIPTION("Ensure RenderPass create meets the requirements for QCOM_render_pass_shader_resolve");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool rpShaderResolveSupport = DeviceExtensionSupported(gpu(), nullptr, VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME);
    if (rpShaderResolveSupport) {
        m_device_extension_names.push_back(VK_QCOM_RENDER_PASS_SHADER_RESOLVE_EXTENSION_NAME);
    } else {
        printf("%s test requires QCOM_render_pass_shader_resolve, not available.  Skipping.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    std::vector<VkAttachmentDescription> attachments = {
        // input attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        // color attachments
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth attachment
        {0, ds_format, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        // resolve attachment
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    std::vector<VkAttachmentReference> input = {
        {0, VK_IMAGE_LAYOUT_GENERAL},
    };
    std::vector<VkAttachmentReference> color = {
        {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference depth = {3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    std::vector<VkAttachmentReference> resolve = {
        {4, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    VkSubpassDescription subpass = {0,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    (uint32_t)input.size(),
                                    input.data(),
                                    (uint32_t)color.size(),
                                    color.data(),
                                    nullptr,
                                    &depth,
                                    0,
                                    nullptr};

    std::vector<VkSubpassDependency> dependency = {
        {0, 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_ACCESS_MEMORY_WRITE_BIT,
         VK_ACCESS_MEMORY_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                   nullptr,
                                   0,
                                   (uint32_t)attachments.size(),
                                   attachments.data(),
                                   1,
                                   &subpass,
                                   0,
                                   nullptr};

    // Create a resolve subpass where the pResolveattachments are not VK_ATTACHMENT_UNUSED
    VkSubpassDescription test_subpass = subpass;
    test_subpass.pResolveAttachments = resolve.data();
    test_subpass.flags = VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM;
    VkRenderPassCreateInfo test_rpci = rpci;
    test_rpci.pSubpasses = &test_subpass;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &test_rpci, rp2Supported, "VUID-VkSubpassDescription-flags-03341",
                         "VUID-VkRenderPassCreateInfo2-flags-04907");

    // Create a resolve subpass which is not the last subpass in the subpass dependency chain.
    {
        VkSubpassDescription subpasses[2] = {subpass, subpass};
        subpasses[0].pResolveAttachments = nullptr;
        subpasses[0].flags = VK_SUBPASS_DESCRIPTION_SHADER_RESOLVE_BIT_QCOM;
        subpasses[1].pResolveAttachments = nullptr;
        subpasses[1].flags = 0;

        VkRenderPassCreateInfo test2_rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                             nullptr,
                                             0,
                                             (uint32_t)attachments.size(),
                                             attachments.data(),
                                             2,
                                             subpasses,
                                             (uint32_t)dependency.size(),
                                             dependency.data()};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &test2_rpci, rp2Supported, "VUID-VkSubpassDescription-flags-03343",
                             "VUID-VkRenderPassCreateInfo2-flags-04909");
    }
}

TEST_F(VkLayerTest, RenderPassCreateAttachmentReferenceInvalidLayout) {
    TEST_DESCRIPTION("Attachment reference uses PREINITIALIZED or UNDEFINED layouts");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    }
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

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    const VkFormat depth_only_format = FindSupportedDepthOnlyFormat(gpu());
    const VkFormat stencil_only_format = FindSupportedStencilOnlyFormat(gpu());

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, ds_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference refs[] = {
        {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},          // color
        {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},  // depth stencil
    };
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &refs[0], nullptr, &refs[1], 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, attach, 1, subpasses, 0, nullptr};

    // Use UNDEFINED layout
    refs[0].layout = VK_IMAGE_LAYOUT_UNDEFINED;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentReference-layout-00857",
                         "VUID-VkAttachmentReference2-layout-03077");

    // Use PREINITIALIZED layout
    refs[0].layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkAttachmentReference-layout-00857",
                         "VUID-VkAttachmentReference2-layout-03077");

    if (rp2Supported) {
        safe_VkRenderPassCreateInfo2 rpci2;
        ConvertVkRenderPassCreateInfoToV2KHR(rpci, &rpci2);

        // set valid values to start
        rpci2.pSubpasses[0].pColorAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");
        rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  // reset

        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");
        rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;  // reset

        if (separate_depth_stencil_layouts_features.separateDepthStencilLayouts) {
            // No VkAttachmentReferenceStencilLayout set
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentReference2-attachment-04755");

            // Set a valid VkAttachmentReferenceStencilLayout since the feature bit is set
            auto attachment_reference_stencil_layout = LvlInitStruct<VkAttachmentReferenceStencilLayout>();
            attachment_reference_stencil_layout.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
            rpci2.pSubpasses[0].pDepthStencilAttachment->pNext = &attachment_reference_stencil_layout;

            // Tests invalid use of color attachment with layouts
            // Duels as tests for 04754 since the attachment format is not a depth/stencil format
            {
                rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentReference2-attachment-04754");
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");

                rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentReference2-attachment-04754");
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");

                rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentReference2-attachment-04754");
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");

                rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentReference2-attachment-04754");
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(), "VUID-VkSubpassDescription2-None-04439");
            }

            // reset to valid layout
            // The following tests originally were negative tests until it was noticed that the aspectMask only matters for input
            // attachments. These tests were converted into positive tests to catch regression
            rpci2.pSubpasses[0].pColorAttachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            {
                rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                PositiveTestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr());
                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                PositiveTestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr());
            }
            {
                rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
                PositiveTestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr());
                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
                PositiveTestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr());
            }
            {
                rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                PositiveTestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr());
                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                PositiveTestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr());
            }

            // Test using depth-only or stencil-only format with wrong layout for the attachment reference
            if (depth_only_format != VK_FORMAT_UNDEFINED) {
                rpci2.pAttachments[1].format = depth_only_format;
                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentReference2-attachment-04756");
                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentReference2-attachment-04756");
            }
            if (stencil_only_format != VK_FORMAT_UNDEFINED) {
                rpci2.pAttachments[1].format = stencil_only_format;
                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentReference2-attachment-04757");
                rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentReference2-attachment-04757");
            }
            rpci2.pAttachments[1].format = ds_format;                                                                // reset
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;  // reset

            VkImageLayout forbidden_layouts[] = {VK_IMAGE_LAYOUT_PREINITIALIZED,
                                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                 VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR,
                                                 VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR,
                                                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                                                 VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
                                                 VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
                                                 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
            rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
            for (size_t i = 0; i < (sizeof(forbidden_layouts) / sizeof(forbidden_layouts[0])); ++i) {
                attachment_reference_stencil_layout.stencilLayout = forbidden_layouts[i];
                TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                         "VUID-VkAttachmentReferenceStencilLayout-stencilLayout-03318");
            }

            attachment_reference_stencil_layout.stencilLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;

            auto attachment_description_stencil_layout = LvlInitStruct<VkAttachmentDescriptionStencilLayoutKHR>();
            attachment_description_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            attachment_description_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;

            rpci2.pAttachments[1].pNext = &attachment_description_stencil_layout;

            rpci2.pAttachments[1].initialLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            rpci2.pAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-06556");

            rpci2.pAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            rpci2.pAttachments[1].finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentDescription2-separateDepthStencilLayouts-06557");

            rpci2.pAttachments[1].pNext = nullptr;

            rpci2.pSubpasses[0].pDepthStencilAttachment->pNext = nullptr;
        } else {
            rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313");
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313");

            rpci2.pSubpasses[0].pDepthStencilAttachment->aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313");
            rpci2.pSubpasses[0].pDepthStencilAttachment->layout = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR;
            TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), rpci2.ptr(),
                                     "VUID-VkAttachmentReference2-separateDepthStencilLayouts-03313");
        }
    }
}

TEST_F(VkLayerTest, RenderPassCreateOverlappingCorrelationMasks) {
    TEST_DESCRIPTION("Create a subpass with overlapping correlation masks");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        } else {
            printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
            return;
        }
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multiview_features);
    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x3u};
    uint32_t correlationMasks[] = {0x1u, 0x3u};
    VkRenderPassMultiviewCreateInfo rpmvci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 1, viewMasks, 0, nullptr, 2, correlationMasks};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 1, &subpass, 0, nullptr};

    // Correlation masks must not overlap
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkRenderPassMultiviewCreateInfo-pCorrelationMasks-00841",
                         "VUID-VkRenderPassCreateInfo2-pCorrelatedViewMasks-03056");

    // Check for more specific "don't set any correlation masks when multiview is not enabled"
    if (rp2Supported) {
        viewMasks[0] = 0;
        correlationMasks[0] = 0;
        correlationMasks[1] = 0;
        safe_VkRenderPassCreateInfo2 safe_rpci2;
        ConvertVkRenderPassCreateInfoToV2KHR(rpci, &safe_rpci2);

        TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), safe_rpci2.ptr(),
                                 "VUID-VkRenderPassCreateInfo2-viewMask-03057");
    }
}

TEST_F(VkLayerTest, RenderPassCreateInvalidViewMasks) {
    TEST_DESCRIPTION("Create a subpass with the wrong number of view masks, or inconsistent setting of view masks");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        } else {
            printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
            return;
        }
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multiview_features);
    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };
    uint32_t viewMasks[] = {0x3u, 0u};
    VkRenderPassMultiviewCreateInfo rpmvci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 1, viewMasks, 0, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 2, subpasses, 0, nullptr};

    // Not enough view masks
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pNext-01928",
                         "VUID-VkRenderPassCreateInfo2-viewMask-03058");
}

TEST_F(VkLayerTest, RenderPassCreateInvalidInputAttachmentReferences) {
    TEST_DESCRIPTION("Create a subpass with the meta data aspect mask set for an input attachment");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};

    // Invalid aspect masks
    // Cannot/should not avoid getting the unxpected ones too
    iaar.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo-pNext-01963");
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964",
                         nullptr);

    iaar.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo-pNext-01963");
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-02250",
                         nullptr);

    // Aspect not present
    iaar.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01963",
                         "VUID-VkRenderPassCreateInfo2-attachment-02525");

    // Invalid subpass index
    iaar.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iaar.subpass = 1;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01926", nullptr);
    iaar.subpass = 0;

    // Invalid input attachment index
    iaar.inputAttachmentIndex = 1;
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01927", nullptr);
}

TEST_F(VkLayerTest, RenderPassCreateInvalidFragmentDensityMapReferences) {
    TEST_DESCRIPTION("Create a subpass with the wrong attachment information for a fragment density map ");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    } else {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
        } else {
            printf("%s Neither extension %s nor %s is not supported.\n", kSkipPrefix, VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME,
                   VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_LOAD,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_PREINITIALIZED,
                                      VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    // Set 1 instead of 0
    VkAttachmentReference ref = {1, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT,
                                                          nullptr, ref};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassCreateInfo-fragmentDensityMapAttachment-06471", nullptr);

    // Set wrong VkImageLayout
    ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT, nullptr, ref};
    rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02549", nullptr);

    // Set wrong load operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_CLEAR,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_PREINITIALIZED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT, nullptr, ref};
    rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02550", nullptr);

    // Set wrong store operation
    attach = {0,
              VK_FORMAT_R8G8_UNORM,
              VK_SAMPLE_COUNT_1_BIT,
              VK_ATTACHMENT_LOAD_OP_LOAD,
              VK_ATTACHMENT_STORE_OP_STORE,
              VK_ATTACHMENT_LOAD_OP_DONT_CARE,
              VK_ATTACHMENT_STORE_OP_DONT_CARE,
              VK_IMAGE_LAYOUT_PREINITIALIZED,
              VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};

    ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT, nullptr, ref};
    rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpfdmi, 0, 1, &attach, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false,
                         "VUID-VkRenderPassFragmentDensityMapCreateInfoEXT-fragmentDensityMapAttachment-02551", nullptr);
}

TEST_F(VkLayerTest, InvalidFragmentDensityMapLayerCount) {
    TEST_DESCRIPTION("Specify a fragment density map attachment with incorrect layerCount");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    VkPhysicalDeviceMultiviewFeatures multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    VkPhysicalDeviceFragmentDensityMapFeaturesEXT fdm_features =
        LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(&multiview_features);
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fdm_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (fdm_features.fragmentDensityMap != VK_TRUE) {
        GTEST_SKIP() << "requires fragmentDensityMap feature";
    } else if (multiview_features.multiview != VK_TRUE) {
        GTEST_SKIP() << "requires multiview feature";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentDescription2 attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT};
    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = {VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT,
                                                          nullptr, ref};

    // Create a renderPass with viewMask 0
    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.viewMask = 0;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>(&rpfdmi);
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    VkRenderPass rp;

    auto vkCreateRenderPass2KHR =
        reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR"));
    ASSERT_VK_SUCCESS(vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp));

    VkImageObj image(m_device);
    image.InitNoLayout(image.ImageCreateInfo2D(32, 32, 1, 2, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                               VK_IMAGE_TILING_OPTIMAL, 0));
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, 2,
                                             VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = 32;
    fb_info.height = 32;
    fb_info.layers = 1;

    VkFramebuffer fb;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-02747");
    vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();

    vk::DestroyRenderPass(m_device->device(), rp, NULL);
    rp = {};

    // Set viewMask to non-zero - requires multiview
    subpass.viewMask = 0x10;
    ASSERT_VK_SUCCESS(vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp));
    fb_info.renderPass = rp;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-02746");
    vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    vk::DestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, RenderPassCreateSubpassNonGraphicsPipeline) {
    TEST_DESCRIPTION("Create a subpass with the compute pipeline bind point");
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_COMPUTE, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, subpasses, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pipelineBindPoint-00844", "VUID-VkSubpassDescription2-pipelineBindPoint-03062");
}

TEST_F(VkLayerTest, RenderPassCreateSubpassMissingAttributesBitMultiviewNVX) {
    TEST_DESCRIPTION("Create a subpass with the VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX flag missing");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME) &&
        DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME);
        return;
    }

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[] = {
        {VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr,
         nullptr, 0, nullptr},
    };

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 1, subpasses, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkSubpassDescription-flags-00856",
                         "VUID-VkSubpassDescription2-flags-03076");
}

TEST_F(VkLayerTest, RenderPassCreate2SubpassInvalidInputAttachmentParameters) {
    TEST_DESCRIPTION("Create a subpass with parameters in the input attachment ref which are invalid");

    // Check for VK_KHR_get_physical_device_properties2
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkAttachmentDescription2KHR attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_UNDEFINED;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference2KHR reference = LvlInitStruct<VkAttachmentReference2>();
    reference.attachment = 0;
    reference.layout = VK_IMAGE_LAYOUT_GENERAL;
    reference.aspectMask = 0;

    auto subpass = LvlInitStruct<VkSubpassDescription2KHR>();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.viewMask = 0;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &reference;

    VkRenderPassCreateInfo2KHR rpci2 = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR, nullptr, 0, 1, &attach_desc, 1, &subpass, 0, nullptr, 0, nullptr};

    // Test for aspect mask of 0
    m_errorMonitor->SetUnexpectedError("VUID-VkAttachmentDescription2-format-06701");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pInputAttachments-02897");
    TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), &rpci2, "VUID-VkSubpassDescription2-attachment-02800");

    // Test for invalid aspect mask bits
    reference.aspectMask = 0x40000000;  // invalid VkImageAspectFlagBits value
    m_errorMonitor->SetUnexpectedError("VUID-VkAttachmentDescription2-format-06701");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pInputAttachments-02897");
    TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), &rpci2, "VUID-VkSubpassDescription2-attachment-02799");

    // Test for invalid use of VK_IMAGE_ASPECT_METADATA_BIT
    reference.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetUnexpectedError("VUID-VkAttachmentDescription2-format-06701");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pInputAttachments-02897");
    TestRenderPass2KHRCreate(m_errorMonitor, m_device->device(), &rpci2, "VUID-VkSubpassDescription2-attachment-02801");
}

TEST_F(VkLayerTest, RenderPassCreateInvalidSubpassDependencies) {
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    bool rp2_supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool multiviewSupported = rp2_supported;

    if (!rp2_supported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        multiviewSupported = true;
    }
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multiview_features);
    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    // Add a device features struct enabling NO features
    features2.features = {};
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        multiviewSupported = true;
    }

    // Create two dummy subpasses
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dependency;
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 2, subpasses, 1, &dependency};

    // Non graphics stages in subpass dependency
    dependency = {0, 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00838", "VUID-VkRenderPassCreateInfo2-pDependencies-03055");

    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00838", "VUID-VkRenderPassCreateInfo2-pDependencies-03055");

    dependency = {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00838", "VUID-VkRenderPassCreateInfo2-pDependencies-03055");

    dependency = {0, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                         "VUID-VkRenderPassCreateInfo-pDependencies-00837", "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    // Geometry shaders not enabled source
    dependency = {0, 1, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcStageMask-04090",
                         "VUID-VkSubpassDependency2-srcStageMask-04090");

    // Geometry shaders not enabled destination
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-dstStageMask-04090",
                         "VUID-VkSubpassDependency2-dstStageMask-04090");

    // Tessellation not enabled source
    dependency = {0, 1, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcStageMask-04091",
                         "VUID-VkSubpassDependency2-srcStageMask-04091");

    // Tessellation not enabled destination
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-dstStageMask-04091",
                         "VUID-VkSubpassDependency2-dstStageMask-04091");

    // Potential cyclical dependency
    dependency = {1, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00864",
                         "VUID-VkSubpassDependency2-srcSubpass-03084");

    // EXTERNAL to EXTERNAL dependency
    dependency = {
        VK_SUBPASS_EXTERNAL, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00865",
                         "VUID-VkSubpassDependency2-srcSubpass-03085");

    // srcStage contains framebuffer space, and dstStage contains non-framebuffer space
    dependency = {0,
                  0,
                  VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                  VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                  0,
                  0,
                  0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-06809",
                         "VUID-VkSubpassDependency2-srcSubpass-06810");

    // framebuffer space stages in self dependency with region bit
    dependency = {0, 0, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-02243",
                         "VUID-VkSubpassDependency2-srcSubpass-02245");

    // Same test but make sure the logical invalid order does not trip other VUID since both are framebuffer space stages
    dependency = {0, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-02243",
                         "VUID-VkSubpassDependency2-srcSubpass-02245");

    // Source access mask mismatch with source stage mask
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_UNIFORM_READ_BIT, 0, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcAccessMask-00868",
                         "VUID-VkSubpassDependency2-srcAccessMask-03088");

    // Destination access mask mismatch with destination stage mask
    dependency = {
        0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-dstAccessMask-00869",
                         "VUID-VkSubpassDependency2-dstAccessMask-03089");

    // srcSubpass larger than subpassCount
    dependency = {3, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-06866",
                         "VUID-VkRenderPassCreateInfo2-srcSubpass-02526");

    // dstSubpass larger than subpassCount
    dependency = {0, 3, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-06867",
                         "VUID-VkRenderPassCreateInfo2-dstSubpass-02527");

    if (multiviewSupported) {
        // VIEW_LOCAL_BIT but multiview is not enabled
        dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, nullptr,
                             "VUID-VkRenderPassCreateInfo2-viewMask-03059");

        // Enable multiview
        uint32_t pViewMasks[2] = {0x3u, 0x3u};
        int32_t pViewOffsets[2] = {0, 0};
        VkRenderPassMultiviewCreateInfo rpmvci = {
            VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 2, pViewMasks, 0, nullptr, 0, nullptr};
        rpci.pNext = &rpmvci;

        // Excessive view offsets
        dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};
        rpmvci.pViewOffsets = pViewOffsets;
        rpmvci.dependencyCount = 2;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01929", nullptr);

        rpmvci.dependencyCount = 0;

        // View offset with subpass self dependency
        dependency = {0, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};
        rpmvci.pViewOffsets = pViewOffsets;
        pViewOffsets[0] = 1;
        rpmvci.dependencyCount = 1;

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01930",
                             "VUID-VkSubpassDependency2-viewOffset-02530");

        rpmvci.dependencyCount = 0;

        // View offset with no view local bit
        if (rp2_supported) {
            dependency = {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};
            rpmvci.pViewOffsets = pViewOffsets;
            pViewOffsets[0] = 1;
            rpmvci.dependencyCount = 1;

            TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, nullptr,
                                 "VUID-VkSubpassDependency2-dependencyFlags-03092");

            rpmvci.dependencyCount = 0;
        }

        // EXTERNAL subpass with VIEW_LOCAL_BIT - source subpass
        dependency = {VK_SUBPASS_EXTERNAL,         1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                      VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                             "VUID-VkSubpassDependency-dependencyFlags-02520", "VUID-VkSubpassDependency2-dependencyFlags-03090");

        // EXTERNAL subpass with VIEW_LOCAL_BIT - destination subpass
        dependency = {0, VK_SUBPASS_EXTERNAL,         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                      0, VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported,
                             "VUID-VkSubpassDependency-dependencyFlags-02521", "VUID-VkSubpassDependency2-dependencyFlags-03091");

        // Multiple views but no view local bit in self-dependency
        dependency = {0, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

        TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00872",
                             "VUID-VkRenderPassCreateInfo2-pDependencies-03060");
    }
}

TEST_F(VkLayerTest, RenderPassCreateInvalidMixedAttachmentSamplesAMD) {
    TEST_DESCRIPTION("Verify error messages for supported and unsupported sample counts in render pass attachments.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_AMD_MIXED_ATTACHMENT_SAMPLES_EXTENSION_NAME);
        return;
    }

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    ASSERT_NO_FATAL_FAILURE(InitState());

    std::vector<VkAttachmentDescription> attachments;

    {
        VkAttachmentDescription att = {};
        att.format = VK_FORMAT_R8G8B8A8_UNORM;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments.push_back(att);

        att.format = VK_FORMAT_D16_UNORM;
        att.samples = VK_SAMPLE_COUNT_4_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(att);
    }

    VkAttachmentReference color_ref = {};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_ref = {};
    depth_ref.attachment = 1;
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = attachments.size();
    rpci.pAttachments = attachments.data();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    VkRenderPass rp;
    VkResult err;

    err = vk::CreateRenderPass(device(), &rpci, NULL, &rp);
    if (err == VK_SUCCESS) vk::DestroyRenderPass(m_device->device(), rp, nullptr);

    // Expect an error message for invalid sample counts
    attachments[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported,
                         "VUID-VkSubpassDescription-pColorAttachments-01506", "VUID-VkSubpassDescription2-pColorAttachments-03070");
}

TEST_F(VkLayerTest, RenderPassBeginInvalidRenderArea) {
    TEST_DESCRIPTION("Generate INVALID_RENDER_AREA error by beginning renderpass with extent outside of framebuffer");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (IsPlatform(kShieldTVb)) {
        printf("%s ShieldTV reports api version 1.1, but does not list VK_KHR_device_group, skipping for this platform.\n",
               kSkipPrefix);
        return;
    }
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool device_group_supported = false;
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEVICE_GROUP_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
        device_group_supported = true;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Framebuffer for render target is 256x256, exceed that for INVALID_RENDER_AREA
    m_renderPassBeginInfo.renderArea.extent.width = 257;
    m_renderPassBeginInfo.renderArea.extent.height = 256;

    const char *vuid =
        device_group_supported ? "VUID-VkRenderPassBeginInfo-pNext-02852" : "VUID-VkRenderPassBeginInfo-renderArea-02848";

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &m_renderPassBeginInfo, rp2Supported, vuid,
                        vuid);
}

TEST_F(VkLayerTest, RenderPassBeginWithinRenderPass) {
    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2KHR = nullptr;
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (rp2Supported) {
        vkCmdBeginRenderPass2KHR =
            (PFN_vkCmdBeginRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdBeginRenderPass2KHR");
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Bind a BeginRenderPass within an active RenderPass
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Just use a dummy Renderpass
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRenderPass-renderpass");
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        VkSubpassBeginInfoKHR subpassBeginInfo = {VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR, nullptr, VK_SUBPASS_CONTENTS_INLINE};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginRenderPass2-renderpass");
        vkCmdBeginRenderPass2KHR(m_commandBuffer->handle(), &m_renderPassBeginInfo, &subpassBeginInfo);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, RenderPassBeginIncompatibleFramebufferRenderPass) {
    TEST_DESCRIPTION("Test that renderpass begin is compatible with the framebuffer renderpass ");

    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    // Create a depth stencil image view
    VkImageObj image(m_device);

    image.Init(128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.initialized());

    VkImageView dsv;
    VkImageViewCreateInfo dsvci = LvlInitStruct<VkImageViewCreateInfo>();
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    dsvci.format = VK_FORMAT_D16_UNORM;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CreateImageView(m_device->device(), &dsvci, NULL, &dsv);

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentDescription description = {0,
                                           VK_FORMAT_D16_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_LOAD,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &description, 1, &subpass, 0, nullptr};
    VkRenderPass rp1, rp2;

    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp1);
    subpass.pDepthStencilAttachment = nullptr;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp2);

    // Create a framebuffer

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp1, 1, &dsv, 128, 128, 1};
    VkFramebuffer fb;

    vk::CreateFramebuffer(m_device->handle(), &fbci, nullptr, &fb);

    VkRenderPassBeginInfo rp_begin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp2, fb, {{0, 0}, {128, 128}}, 0, nullptr};

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, false,
                        "VUID-VkRenderPassBeginInfo-renderPass-00904", nullptr);

    vk::DestroyRenderPass(m_device->device(), rp1, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp2, nullptr);
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyImageView(m_device->device(), dsv, nullptr);
}

TEST_F(VkLayerTest, RenderPassBeginLayoutsFramebufferImageUsageMismatches) {
    TEST_DESCRIPTION(
        "Test that renderpass initial/final layouts match up with the usage bits set for each attachment of the framebuffer");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool maintenance2Supported = rp2Supported;

    // Check for VK_KHR_maintenance2
    if (!rp2Supported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        maintenance2Supported = true;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        maintenance2Supported = true;
    }

    // Create an input attachment view
    VkImageObj iai(m_device);

    iai.InitNoLayout(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(iai.initialized());

    VkImageView iav;
    VkImageViewCreateInfo iavci = LvlInitStruct<VkImageViewCreateInfo>();
    iavci.image = iai.handle();
    iavci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iavci.format = VK_FORMAT_R8G8B8A8_UNORM;
    iavci.subresourceRange.layerCount = 1;
    iavci.subresourceRange.baseMipLevel = 0;
    iavci.subresourceRange.levelCount = 1;
    iavci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CreateImageView(m_device->device(), &iavci, NULL, &iav);

    // Create an input depth attachment view
    VkImageObj iadi(m_device);

    VkFormat dformat = FindSupportedDepthStencilFormat(gpu());
    iadi.InitNoLayout(128, 128, 1, dformat, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(iadi.initialized());

    VkImageView iadv = iadi.targetView(dformat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1);

    // Create a color attachment view
    VkImageObj cai(m_device);

    cai.InitNoLayout(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(cai.initialized());

    VkImageView cav;
    VkImageViewCreateInfo cavci = LvlInitStruct<VkImageViewCreateInfo>();
    cavci.image = cai.handle();
    cavci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    cavci.format = VK_FORMAT_R8G8B8A8_UNORM;
    cavci.subresourceRange.layerCount = 1;
    cavci.subresourceRange.baseMipLevel = 0;
    cavci.subresourceRange.levelCount = 1;
    cavci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CreateImageView(m_device->device(), &cavci, NULL, &cav);

    // Create a renderPass with those attachments
    VkAttachmentDescription descriptions[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        {1, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL}};

    VkAttachmentReference input_ref = {0, VK_IMAGE_LAYOUT_GENERAL};
    VkAttachmentReference color_ref = {1, VK_IMAGE_LAYOUT_GENERAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &input_ref, 1, &color_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, descriptions, 1, &subpass, 0, nullptr};

    // Create a framebuffer

    VkImageView views[] = {iav, cav};

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, VK_NULL_HANDLE, 2, views, 128, 128, 1};

    VkClearValue clearValues[2];
    clearValues[0].color = {{0, 0, 0, 0}};
    clearValues[1].color = {{0, 0, 0, 0}};
    VkRenderPassBeginInfo rp_begin = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, VK_NULL_HANDLE, VK_NULL_HANDLE, {{0, 0}, {128, 128}}, 2, clearValues};

    auto test_layout_helper = [this, &rpci, &rp_begin, rp2Supported, &fbci](const char *rp1_vuid, const char *rp2_vuid) {
        vk_testing::RenderPass rp_invalid(*m_device, rpci);
        fbci.renderPass = rp_invalid.handle();
        vk_testing::Framebuffer fb_invalid(*m_device, fbci);
        rp_begin.renderPass = rp_invalid.handle();
        rp_begin.framebuffer = fb_invalid.handle();
        TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported, rp1_vuid,
                            rp2_vuid);
    };

    // Initial layout is VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL but attachment doesn't support IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00895", "VUID-vkCmdBeginRenderPass2-initialLayout-03094");

    // Initial layout is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    // / VK_IMAGE_USAGE_SAMPLED_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00897", "VUID-vkCmdBeginRenderPass2-initialLayout-03097");

    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_GENERAL;

    // Initial layout is VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00898", "VUID-vkCmdBeginRenderPass2-initialLayout-03098");

    // Initial layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL but attachment doesn't support VK_IMAGE_USAGE_TRANSFER_DST_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-00899", "VUID-vkCmdBeginRenderPass2-initialLayout-03099");

    // Change to depth views since we are changing format
    descriptions[0].format = dformat;
    views[0] = iadv;

    // Initial layout is VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL but attachment doesn't support
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    const char *initial_layout_vuid_rp1 =
        maintenance2Supported ? "VUID-vkCmdBeginRenderPass-initialLayout-01758" : "VUID-vkCmdBeginRenderPass-initialLayout-00896";
    test_layout_helper(initial_layout_vuid_rp1, "VUID-vkCmdBeginRenderPass2-initialLayout-03096");

    // Initial layout is VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL but attachment doesn't support
    // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    test_layout_helper(initial_layout_vuid_rp1, "VUID-vkCmdBeginRenderPass2-initialLayout-03096");

    if (maintenance2Supported || rp2Supported) {
        // Initial layout is VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL but attachment doesn't support
        // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2-initialLayout-03096");

        // Initial layout is VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL but attachment doesn't support
        // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        descriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        test_layout_helper("VUID-vkCmdBeginRenderPass-initialLayout-01758", "VUID-vkCmdBeginRenderPass2-initialLayout-03096");
    }

    vk::DestroyImageView(m_device->device(), iav, nullptr);
    vk::DestroyImageView(m_device->device(), cav, nullptr);
}

TEST_F(VkLayerTest, RenderPassBeginClearOpMismatch) {
    TEST_DESCRIPTION(
        "Begin a renderPass where clearValueCount is less than the number of renderPass attachments that use "
        "loadOp VK_ATTACHMENT_LOAD_OP_CLEAR.");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    // Set loadOp to CLEAR
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    VkRenderPass rp;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);

    VkRenderPassBeginInfo rp_begin = LvlInitStruct<VkRenderPassBeginInfo>();
    rp_begin.renderPass = renderPass();
    rp_begin.framebuffer = framebuffer();
    rp_begin.clearValueCount = 0;  // Should be 1

    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, rp2Supported,
                        "VUID-VkRenderPassBeginInfo-clearValueCount-00902", "VUID-VkRenderPassBeginInfo-clearValueCount-00902");

    vk::DestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, RenderPassBeginSampleLocationsInvalidIndicesEXT) {
    TEST_DESCRIPTION("Test that attachment indices and subpass indices specifed by sample locations structures are valid");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    assert(vkGetPhysicalDeviceProperties2KHR != nullptr);

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props =
        LvlInitStruct<VkPhysicalDeviceSampleLocationsPropertiesEXT>();
    VkPhysicalDeviceProperties2KHR prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&sample_locations_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        printf("%s VK_SAMPLE_COUNT_1_BIT sampleLocationSampleCounts is not supported.\n", kSkipPrefix);
        return;
    }

    // Create a depth stencil image view
    VkImageObj image(m_device);

    image.Init(128, 128, 1, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image.initialized());

    VkImageView dsv;
    VkImageViewCreateInfo dsvci = LvlInitStruct<VkImageViewCreateInfo>();
    dsvci.image = image.handle();
    dsvci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    dsvci.format = VK_FORMAT_D16_UNORM;
    dsvci.subresourceRange.layerCount = 1;
    dsvci.subresourceRange.baseMipLevel = 0;
    dsvci.subresourceRange.levelCount = 1;
    dsvci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk::CreateImageView(m_device->device(), &dsvci, NULL, &dsv);

    // Create a renderPass with a single attachment that uses loadOp CLEAR
    VkAttachmentDescription description = {0,
                                           VK_FORMAT_D16_UNORM,
                                           VK_SAMPLE_COUNT_1_BIT,
                                           VK_ATTACHMENT_LOAD_OP_LOAD,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                           VK_IMAGE_LAYOUT_GENERAL,
                                           VK_IMAGE_LAYOUT_GENERAL};

    VkAttachmentReference depth_stencil_ref = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0,      VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, &depth_stencil_ref, 0,
                                    nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &description, 1, &subpass, 0, nullptr};
    VkRenderPass rp;

    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);

    // Create a framebuffer

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &dsv, 128, 128, 1};
    VkFramebuffer fb;

    vk::CreateFramebuffer(m_device->handle(), &fbci, nullptr, &fb);

    VkSampleLocationEXT sample_location = {0.5, 0.5};

    VkSampleLocationsInfoEXT sample_locations_info = {
        VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT, nullptr, VK_SAMPLE_COUNT_1_BIT, {1, 1}, 1, &sample_location};

    VkAttachmentSampleLocationsEXT attachment_sample_locations = {0, sample_locations_info};
    VkSubpassSampleLocationsEXT subpass_sample_locations = {0, sample_locations_info};

    VkRenderPassSampleLocationsBeginInfoEXT rp_sl_begin = {VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT,
                                                           nullptr,
                                                           1,
                                                           &attachment_sample_locations,
                                                           1,
                                                           &subpass_sample_locations};

    VkRenderPassBeginInfo rp_begin = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, &rp_sl_begin, rp, fb, {{0, 0}, {128, 128}}, 0, nullptr};

    attachment_sample_locations.attachmentIndex = 1;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, false,
                        "VUID-VkAttachmentSampleLocationsEXT-attachmentIndex-01531", nullptr);
    attachment_sample_locations.attachmentIndex = 0;

    subpass_sample_locations.subpassIndex = 1;
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rp_begin, false,
                        "VUID-VkSubpassSampleLocationsEXT-subpassIndex-01532", nullptr);
    subpass_sample_locations.subpassIndex = 0;

    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyImageView(m_device->device(), dsv, nullptr);
}

TEST_F(VkLayerTest, InvalidSampleLocations) {
    TEST_DESCRIPTION("Test invalid cases of VK_EXT_sample_location");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
    } else {
        printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    assert(vkGetPhysicalDeviceProperties2KHR != nullptr);
    PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT vkGetPhysicalDeviceMultisamplePropertiesEXT =
        (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vk::GetInstanceProcAddr(instance(),
                                                                                 "vkGetPhysicalDeviceMultisamplePropertiesEXT");
    assert(vkGetPhysicalDeviceMultisamplePropertiesEXT != nullptr);
    PFN_vkCmdSetSampleLocationsEXT vkCmdSetSampleLocationsEXT =
        (PFN_vkCmdSetSampleLocationsEXT)vk::GetInstanceProcAddr(instance(), "vkCmdSetSampleLocationsEXT");
    assert(vkCmdSetSampleLocationsEXT != nullptr);

    VkPhysicalDeviceSampleLocationsPropertiesEXT sample_locations_props =
        LvlInitStruct<VkPhysicalDeviceSampleLocationsPropertiesEXT>();
    VkPhysicalDeviceProperties2KHR prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&sample_locations_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    if ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_1_BIT) == 0) {
        printf("%s VK_SAMPLE_COUNT_1_BIT sampleLocationSampleCounts is not supported.\n", kSkipPrefix);
        return;
    }

    const bool support_64_sample_count = ((sample_locations_props.sampleLocationSampleCounts & VK_SAMPLE_COUNT_64_BIT) != 0);

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = 128;
    image_create_info.extent.height = 128;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // If S8_UINT is supported, check not having depth with sample location compatible bit
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), VK_FORMAT_S8_UINT, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
        image_create_info.flags = VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;
        image_create_info.format = VK_FORMAT_S8_UINT;
        VkImage temp_image;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-flags-01533");
        vk::CreateImage(m_device->device(), &image_create_info, nullptr, &temp_image);
        m_errorMonitor->VerifyFound();
    }

    const VkFormat depth_format = FindSupportedDepthStencilFormat(gpu());
    if (depth_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    image_create_info.flags = 0;  // image will not have needed flag
    image_create_info.format = depth_format;
    VkImageObj depth_image(m_device);
    depth_image.init(&image_create_info);
    ASSERT_TRUE(depth_image.initialized());
    VkImageView depth_image_view = depth_image.targetView(depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkImageObj color_image(m_device);
    color_image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(color_image.initialized());
    VkImageView color_image_view = color_image.targetView(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

    VkAttachmentDescription descriptions[2] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, depth_format, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}};
    m_renderPass_attachments.push_back(descriptions[0]);
    m_renderPass_attachments.push_back(descriptions[1]);
    VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_stencil_ref = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &color_ref, nullptr, &depth_stencil_ref, 0, nullptr};
    m_renderPass_subpasses.push_back(subpass);
    m_renderPass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 2, descriptions, 1, &subpass, 0, nullptr};
    vk::CreateRenderPass(m_device->device(), &m_renderPass_info, NULL, &m_renderPass);

    // Create a framebuffer
    m_framebuffer_attachments.push_back(color_image_view);
    m_framebuffer_attachments.push_back(depth_image_view);
    m_framebuffer_info = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 2, m_framebuffer_attachments.data(), 128, 128, 1};
    vk::CreateFramebuffer(m_device->handle(), &m_framebuffer_info, nullptr, &m_framebuffer);

    VkMultisamplePropertiesEXT multisample_prop = LvlInitStruct<VkMultisamplePropertiesEXT>();
    vkGetPhysicalDeviceMultisamplePropertiesEXT(gpu(), VK_SAMPLE_COUNT_1_BIT, &multisample_prop);
    // 1 from VK_SAMPLE_COUNT_1_BIT
    const uint32_t valid_count =
        multisample_prop.maxSampleLocationGridSize.width * multisample_prop.maxSampleLocationGridSize.height * 1;

    std::vector<VkSampleLocationEXT> sample_location(valid_count,{0.5, 0.5});
    VkSampleLocationsInfoEXT sample_locations_info = LvlInitStruct<VkSampleLocationsInfoEXT>();
    sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
    sample_locations_info.sampleLocationGridSize = multisample_prop.maxSampleLocationGridSize;
    sample_locations_info.sampleLocationsCount = valid_count;
    sample_locations_info.pSampleLocations = sample_location.data();

    VkPipelineSampleLocationsStateCreateInfoEXT sample_location_state =
        LvlInitStruct<VkPipelineSampleLocationsStateCreateInfoEXT>();
    sample_location_state.sampleLocationsEnable = VK_TRUE;
    sample_location_state.sampleLocationsInfo = sample_locations_info;

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci =
        LvlInitStruct<VkPipelineMultisampleStateCreateInfo>(&sample_location_state);
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    VkPipelineDepthStencilStateCreateInfo pipe_ds_state_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe_ds_state_ci.depthTestEnable = VK_TRUE;
    pipe_ds_state_ci.stencilTestEnable = VK_FALSE;

    {
        CreatePipelineHelper pipe(*this);
        pipe.InitInfo();
        pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
        pipe.InitState();
        pipe.gp_ci_.pDepthStencilState = &pipe_ds_state_ci;

        // Set invalid grid size width
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.width =
            multisample_prop.maxSampleLocationGridSize.width + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01521");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.width = multisample_prop.maxSampleLocationGridSize.width;

        // Set invalid grid size height
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
            multisample_prop.maxSampleLocationGridSize.height + 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01522");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height = multisample_prop.maxSampleLocationGridSize.height;

        // Test to make sure the modulo is correct due to akward wording in spec
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
            multisample_prop.maxSampleLocationGridSize.height * 2;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01522");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height = multisample_prop.maxSampleLocationGridSize.height;

        if (multisample_prop.maxSampleLocationGridSize.height > 1) {
            // Expects there to be no 01522 vuid
            sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
                multisample_prop.maxSampleLocationGridSize.height / 2;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
            pipe.CreateGraphicsPipeline();
            m_errorMonitor->VerifyFound();
            sample_location_state.sampleLocationsInfo.sampleLocationGridSize.height =
                multisample_prop.maxSampleLocationGridSize.height;
        }

        // non-matching rasterizationSamples
        pipe.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01523");
        // if grid size is different
        m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01521");
        m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-pDynamicStates-01522");
        m_errorMonitor->SetUnexpectedError("VUID-VkGraphicsPipelineCreateInfo-multisampledRenderToSingleSampled-06853");
        pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
        pipe.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    // Creates valid pipelines with dynamic state
    const VkDynamicState dyn_state = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
    VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
    dyn_state_ci.dynamicStateCount = 1;
    dyn_state_ci.pDynamicStates = &dyn_state;

    CreatePipelineHelper dynamic_pipe(*this);
    dynamic_pipe.InitInfo();
    dynamic_pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    dynamic_pipe.dyn_state_ci_ = dyn_state_ci;
    dynamic_pipe.InitState();
    dynamic_pipe.gp_ci_.pDepthStencilState = &pipe_ds_state_ci;
    dynamic_pipe.CreateGraphicsPipeline();

    VkRenderPassBeginInfo rp_begin = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, m_renderPass, m_framebuffer, {{0, 0}, {128, 128}}, 0, nullptr};
    const float vbo_data[3] = {1.f, 0.f, 1.f};
    VkConstantBufferObj vbo(m_device, sizeof(vbo_data), (const void *)&vbo_data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(rp_begin);
    m_commandBuffer->BindVertexBuffer(&vbo, 0, 1);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, dynamic_pipe.pipeline_);

    // test trying to use unsupported sample count
    if (support_64_sample_count == false) {
        sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_64_BIT;
        sample_locations_info.sampleLocationsCount = valid_count * 64;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsPerPixel-01526");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdSetSampleLocationsEXT-sampleLocationsPerPixel-01529");
        vkCmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
        m_errorMonitor->VerifyFound();

        sample_locations_info.sampleLocationsPerPixel = VK_SAMPLE_COUNT_1_BIT;
        sample_locations_info.sampleLocationsCount = valid_count;
    }

    // Test invalid sample location count
    sample_locations_info.sampleLocationsCount = valid_count + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527");
    vkCmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    m_errorMonitor->VerifyFound();
    sample_locations_info.sampleLocationsCount = valid_count;

    // Test image was never created with VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT
    vkCmdSetSampleLocationsEXT(m_commandBuffer->handle(), &sample_locations_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-sampleLocationsEnable-02689");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, RenderPassNextSubpassExcessive) {
    TEST_DESCRIPTION("Test that an error is produced when CmdNextSubpass is called too many times in a renderpass instance");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkCmdNextSubpass2KHR vkCmdNextSubpass2KHR = nullptr;
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    if (rp2Supported) {
        vkCmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdNextSubpass2KHR");
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdNextSubpass-None-00909");
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        VkSubpassBeginInfoKHR subpassBeginInfo = {VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO_KHR, nullptr, VK_SUBPASS_CONTENTS_INLINE};
        VkSubpassEndInfoKHR subpassEndInfo = {VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR, nullptr};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdNextSubpass2-None-03102");

        vkCmdNextSubpass2KHR(m_commandBuffer->handle(), &subpassBeginInfo, &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, RenderPassEndBeforeFinalSubpass) {
    TEST_DESCRIPTION("Test that an error is produced when CmdEndRenderPass is called before the final subpass has been reached");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    PFN_vkCmdEndRenderPass2KHR vkCmdEndRenderPass2KHR = nullptr;
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    if (rp2Supported) {
        vkCmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndRenderPass2KHR");
    }

    VkSubpassDescription sd[2] = {{0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
                                  {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr}};

    VkRenderPassCreateInfo rcpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 0, nullptr, 2, sd, 0, nullptr};

    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rcpi, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 0, nullptr, 16, 16, 1};

    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp, fb, {{0, 0}, {16, 16}}, 0, nullptr};

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass-None-00910");
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        VkSubpassEndInfoKHR subpassEndInfo = {VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR, nullptr};

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass2-None-03103");
        vkCmdEndRenderPass2KHR(m_commandBuffer->handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }

    // Clean up.
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, InvalidRenderPassEndFragmentDensityMapOffsetQCOM) {
    TEST_DESCRIPTION("Ensure RenderPass end meets the requirements for VK_QCOM_fragment_density_map_offset");

    AddRequiredExtensions(VK_QCOM_FRAGMENT_DENSITY_MAP_OFFSET_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&multiview_features);
    auto vkGetPhysicalDeviceFeatures2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    PFN_vkCmdEndRenderPass2KHR vkCmdEndRenderPass2KHR = nullptr;
    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR = nullptr;

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (rp2Supported) {
        vkCmdEndRenderPass2KHR =
            reinterpret_cast<PFN_vkCmdEndRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndRenderPass2KHR"));
        vkCreateRenderPass2KHR =
            reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR"));
    }

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    std::vector<VkAttachmentDescription2> attachments = {
        // FDM attachments
        {VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr, 0, VK_FORMAT_R8G8_UNORM, VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT},
        // input attachments
        {VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr, 0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        // color attachments
        {VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr, 0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr, 0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // depth attachment
        {VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr, 0, ds_format, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        // resolve attachment
        {VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr, 0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        // preserve attachments
        {VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2, nullptr, 0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_4_BIT,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };

    std::vector<VkAttachmentReference2> fdm = {
        {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 0, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT},
    };

    std::vector<VkAttachmentReference2> input = {
        {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 1, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT},
    };

    std::vector<VkAttachmentReference2> color = {
        {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT},
        {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT},
    };
    VkAttachmentReference2 depth = {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 4,
                                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT};
    std::vector<VkAttachmentReference2> resolve = {
        {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, 5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT},
        {VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 0},
    };
    std::vector<uint32_t> preserve = {6};

    VkSubpassDescription2 subpass = {VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
                                     nullptr,
                                     0,
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     0,
                                     (uint32_t)input.size(),
                                     input.data(),
                                     (uint32_t)color.size(),
                                     color.data(),
                                     resolve.data(),
                                     &depth,
                                     (uint32_t)preserve.size(),
                                     preserve.data()};

    // Create a renderPass with a single color attachment for fragment density map
    auto fragment_density_map_create_info = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
    fragment_density_map_create_info.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
                                    &fragment_density_map_create_info,
                                    0,
                                    (uint32_t)attachments.size(),
                                    attachments.data(),
                                    1,
                                    &subpass,
                                    0,
                                    nullptr,
                                    0,
                                    nullptr};

    // Create rp2[0] without Multiview (zero viewMask), rp2[1] with Multiview
    VkRenderPass rp2[2];
    VkResult err = vkCreateRenderPass2KHR(m_device->device(), &rpci, nullptr, &rp2[0]);
    ASSERT_VK_SUCCESS(err);

    subpass.viewMask = 0x3u;
    err = vkCreateRenderPass2KHR(m_device->device(), &rpci, nullptr, &rp2[1]);
    ASSERT_VK_SUCCESS(err);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8_UNORM;
    image_create_info.extent.width = 16;
    image_create_info.extent.height = 16;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 3;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
    image_create_info.flags = 0;

    VkImageObj fdm_image(m_device);
    fdm_image.init(&image_create_info);
    ASSERT_TRUE(fdm_image.initialized());

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;

    VkImageObj input_image(m_device);
    input_image.init(&image_create_info);
    ASSERT_TRUE(input_image.initialized());

    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj color_image1(m_device);
    color_image1.init(&image_create_info);
    ASSERT_TRUE(color_image1.initialized());

    VkImageObj color_image2(m_device);
    color_image2.init(&image_create_info);
    ASSERT_TRUE(color_image2.initialized());

    image_create_info.format = ds_format;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageObj depth_image(m_device);
    depth_image.init(&image_create_info);
    ASSERT_TRUE(depth_image.initialized());

    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImageObj resolve_image(m_device);
    resolve_image.init(&image_create_info);
    ASSERT_TRUE(resolve_image.initialized());

    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    VkImageObj preserve_image(m_device);
    preserve_image.init(&image_create_info);
    ASSERT_TRUE(preserve_image.initialized());

    // Create view attachment
    VkImageView iv[7];
    vk_testing::ImageView iv0;
    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = fdm_image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_R8G8_UNORM;
    ivci.flags = 0;
    ivci.subresourceRange.layerCount = 3;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv0.init(*m_device, ivci);
    ASSERT_TRUE(iv0.initialized());
    iv[0] = iv0.handle();

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.image = input_image.handle();
    vk_testing::ImageView iv1;
    iv1.init(*m_device, ivci);
    ASSERT_TRUE(iv1.initialized());
    iv[1] = iv1.handle();

    ivci.image = color_image1.handle();
    vk_testing::ImageView iv2;
    iv2.init(*m_device, ivci);
    ASSERT_TRUE(iv2.initialized());
    iv[2] = iv2.handle();

    ivci.image = color_image2.handle();
    vk_testing::ImageView iv3;
    iv3.init(*m_device, ivci);
    ASSERT_TRUE(iv3.initialized());
    iv[3] = iv3.handle();

    ivci.format = ds_format;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    ivci.image = depth_image.handle();
    vk_testing::ImageView iv4;
    iv4.init(*m_device, ivci);
    ASSERT_TRUE(iv4.initialized());
    iv[4] = iv4.handle();

    ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivci.image = resolve_image.handle();
    vk_testing::ImageView iv5;
    iv5.init(*m_device, ivci);
    ASSERT_TRUE(iv5.initialized());
    iv[5] = iv5.handle();

    ivci.image = preserve_image.handle();
    vk_testing::ImageView iv6;
    iv6.init(*m_device, ivci);
    ASSERT_TRUE(iv6.initialized());
    iv[6] = iv6.handle();
    ASSERT_VK_SUCCESS(err);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.flags = 0;
    fbci.width = 16;
    fbci.height = 16;
    fbci.layers = 1;
    fbci.renderPass = rp2[0];
    fbci.attachmentCount = 7;
    fbci.pAttachments = iv;

    vk_testing::Framebuffer fb1(*m_device, fbci);

    fbci.renderPass = rp2[1];
    vk_testing::Framebuffer fb2(*m_device, fbci);

    // define renderpass begin info
    VkRenderPassBeginInfo rpbi1 = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp2[0], fb1.handle(), {{0, 0}, {16, 16}}, 0, nullptr};
    VkRenderPassBeginInfo rpbi2 = {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, rp2[1], fb2.handle(), {{0, 0}, {16, 16}}, 0, nullptr};

    if (rp2Supported) {
        auto offsetting = LvlInitStruct<VkSubpassFragmentDensityMapOffsetEndInfoQCOM>();
        auto subpassEndInfo = LvlInitStruct<VkSubpassEndInfoKHR>(&offsetting);
        VkOffset2D m_vOffsets[2];
        offsetting.pFragmentDensityOffsets = m_vOffsets;
        offsetting.fragmentDensityOffsetCount = 2;

        PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
            (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
        ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
        VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM fdm_offset_properties =
            LvlInitStruct<VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM>();
        VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fdm_offset_properties);
        vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

        m_vOffsets[0].x = 1;
        m_vOffsets[0].y = 1;

        m_vOffsets[1].x = 1;
        m_vOffsets[1].y = 1;

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        // begin renderpass that uses rbpi1 renderpass begin info
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi1, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapOffsets-06503");

        if (fdm_offset_properties.fragmentDensityOffsetGranularity.width > 1) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        }

        if (fdm_offset_properties.fragmentDensityOffsetGranularity.height > 1) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        }

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapAttachment-06504");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pInputAttachments-06506");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pDepthStencilAttachment-06505");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pResolveAttachments-06508");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pPreserveAttachments-06509");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06511");
        vkCmdEndRenderPass2KHR(m_commandBuffer->handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->reset();
        m_commandBuffer->begin();

        // begin renderpass that uses rbpi2 renderpass begin info
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi2, VK_SUBPASS_CONTENTS_INLINE);
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-x-06512");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-y-06513");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-06502");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapOffsets-06503");
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit, "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityMapAttachment-06504");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pInputAttachments-06506");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pColorAttachments-06507");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pDepthStencilAttachment-06505");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pResolveAttachments-06508");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-pPreserveAttachments-06509");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkSubpassFragmentDensityMapOffsetEndInfoQCOM-fragmentDensityOffsetCount-06510");
        vkCmdEndRenderPass2KHR(m_commandBuffer->handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroyRenderPass(m_device->device(), rp2[0], nullptr);
    vk::DestroyRenderPass(m_device->device(), rp2[1], nullptr);
}

TEST_F(VkLayerTest, RenderPassDestroyWhileInUse) {
    TEST_DESCRIPTION("Delete in-use renderPass.");

    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create simple renderpass
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    m_commandBuffer->begin();
    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = m_framebuffer;
    rpbi.renderPass = rp;
    m_commandBuffer->BeginRenderPass(rpbi);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkDestroyRenderPass-renderPass-00873");
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    m_errorMonitor->VerifyFound();

    // Wait for queue to complete so we can safely destroy rp
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError("If renderPass is not VK_NULL_HANDLE, renderPass must be a valid VkRenderPass handle");
    m_errorMonitor->SetUnexpectedError("Was it created? Has it already been destroyed?");
    vk::DestroyRenderPass(m_device->device(), rp, nullptr);
}

TEST_F(VkLayerTest, FramebufferCreateErrors) {
    TEST_DESCRIPTION("VUIDs related to framebuffer creation");

    AddOptionalExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    bool imageless_framebuffer_support = IsExtensionsEnabled(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    bool push_fragment_density_support = IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME) ||
                                         IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME);
    if (!push_fragment_density_support) {
        GTEST_SKIP() << "Neither " << VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME << " nor "
                     << VK_EXT_FRAGMENT_DENSITY_MAP_2_EXTENSION_NAME << " are supported.";
    }

    auto fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    auto features2 = GetPhysicalDeviceFeatures2(fdm_features);
    if (!fdm_features.fragmentDensityMap) {
        GTEST_SKIP() << "fragmentDensityMap not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-attachmentCount-00876");

    bool rp2_supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    bool multiviewSupported = rp2_supported || (DeviceValidationVersion() >= VK_API_VERSION_1_1);

    if (!multiviewSupported && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        multiviewSupported = true;
    }

    // Create a renderPass with a single color attachment
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    VkImageView ivs[2];
    ivs[0] = m_renderTargets[0]->targetView(VK_FORMAT_B8G8R8A8_UNORM);
    ivs[1] = m_renderTargets[0]->targetView(VK_FORMAT_B8G8R8A8_UNORM);
    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp;
    // Set mis-matching attachmentCount
    fb_info.attachmentCount = 2;
    fb_info.pAttachments = ivs;
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = 1;

    VkFramebuffer fb;
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    vk::DestroyRenderPass(m_device->device(), rp, NULL);

    // Create a renderPass with a depth-stencil attachment created with
    // IMAGE_USAGE_COLOR_ATTACHMENT
    // Add our color attachment to pDepthStencilAttachment
    subpass.pDepthStencilAttachment = &attach;
    subpass.pColorAttachments = NULL;
    VkRenderPass rp_ds;
    err = vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp_ds);
    ASSERT_VK_SUCCESS(err);
    // Set correct attachment count, but attachment has COLOR usage bit set
    fb_info.attachmentCount = 1;
    fb_info.renderPass = rp_ds;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-02633");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    vk::DestroyRenderPass(m_device->device(), rp_ds, NULL);

    {
        VkImageCreateInfo image_ci = LvlInitStruct<VkImageCreateInfo>();
        image_ci.imageType = VK_IMAGE_TYPE_3D;
        image_ci.format = VK_FORMAT_B8G8R8A8_UNORM;
        image_ci.extent.width = 256;
        image_ci.extent.height = 256;
        image_ci.extent.depth = 1;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.flags = 0;
        VkImageObj image(m_device);
        image.init(&image_ci);

        VkImageView view = image.targetView(VK_FORMAT_D16_UNORM);

        VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, m_renderPass, 1, &view, 256, 256, 1};
        VkFramebuffer framebuffer;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-00891");
        m_errorMonitor->SetUnexpectedError("VUID-VkFramebufferCreateInfo-pAttachments-00880");
        vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &framebuffer);
        m_errorMonitor->VerifyFound();
    }

    // Create new renderpass with alternate attachment format from fb
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    subpass.pDepthStencilAttachment = NULL;
    subpass.pColorAttachments = &attach;
    err = vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    // Cause error due to mis-matched formats between rp & fb
    //  rp attachment 0 now has RGBA8 but corresponding fb attach is BGRA8
    fb_info.renderPass = rp;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-00880");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    vk::DestroyRenderPass(m_device->device(), rp, NULL);

    // Create new renderpass with alternate sample count from fb
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_4_BIT;
    err = vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    ASSERT_VK_SUCCESS(err);

    // Cause error due to mis-matched sample count between rp & fb
    fb_info.renderPass = rp;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-00881");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);

    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }

    vk::DestroyRenderPass(m_device->device(), rp, NULL);

    {
        // Create an image with 2 mip levels.
        VkImageObj image(m_device);
        image.Init(128, 128, 2, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image.initialized());

        // Create a image view with two mip levels.
        VkImageView view;
        VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
        ivci.image = image.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        // Set level count to 2 (only 1 is allowed for FB attachment)
        ivci.subresourceRange.levelCount = 2;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        err = vk::CreateImageView(m_device->device(), &ivci, NULL, &view);
        ASSERT_VK_SUCCESS(err);

        // Re-create renderpass to have matching sample count
        attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        subpass.colorAttachmentCount = 1;
        err = vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
        subpass.colorAttachmentCount = 0;
        ASSERT_VK_SUCCESS(err);

        fb_info.renderPass = rp;
        fb_info.pAttachments = &view;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-00883");
        err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);

        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vk::DestroyFramebuffer(m_device->device(), fb, NULL);
        }
        vk::DestroyImageView(m_device->device(), view, NULL);
    }

    // Update view to original color buffer and grow FB dimensions too big
    fb_info.pAttachments = ivs;
    fb_info.width = 1024;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04533");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    fb_info.width = 256;

    fb_info.height = 1024;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04534");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    fb_info.height = 256;

    fb_info.layers = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04535");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    fb_info.layers = 1;

    {
        if (!push_fragment_density_support || !imageless_framebuffer_support) {
            printf("%s VK_EXT_fragment_density_map or VK_KHR_imageless_framebuffer extension not supported, skipping tests\n",
                   kSkipPrefix);
        } else {
            uint32_t attachment_width = 512;
            uint32_t attachment_height = 512;
            VkFormat attachment_format = VK_FORMAT_R8G8_UNORM;
            uint32_t frame_width = 512;
            uint32_t frame_height = 512;

            // Create a renderPass with a single color attachment for fragment density map
            VkAttachmentDescription attach_desc_fragment_density_map = {};
            attach_desc_fragment_density_map.format = attachment_format;
            attach_desc_fragment_density_map.samples = VK_SAMPLE_COUNT_1_BIT;
            attach_desc_fragment_density_map.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
            VkRenderPassFragmentDensityMapCreateInfoEXT fragment_density_map_create_info =
                LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
            fragment_density_map_create_info.fragmentDensityMapAttachment.layout = VK_IMAGE_LAYOUT_GENERAL;
            VkSubpassDescription subpass_fragment_density_map = {};
            VkRenderPassCreateInfo rpci_fragment_density_map =
                LvlInitStruct<VkRenderPassCreateInfo>(&fragment_density_map_create_info);
            rpci_fragment_density_map.subpassCount = 1;
            rpci_fragment_density_map.pSubpasses = &subpass_fragment_density_map;
            rpci_fragment_density_map.attachmentCount = 1;
            rpci_fragment_density_map.pAttachments = &attach_desc_fragment_density_map;
            VkRenderPass rp_fragment_density_map;

            err = vk::CreateRenderPass(m_device->device(), &rpci_fragment_density_map, NULL, &rp_fragment_density_map);
            ASSERT_VK_SUCCESS(err);

            // Create view attachment
            VkImageView view_fragment_density_map;
            VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
            ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ivci.format = attachment_format;
            ivci.flags = 0;
            ivci.subresourceRange.layerCount = 1;
            ivci.subresourceRange.baseMipLevel = 0;
            ivci.subresourceRange.levelCount = 1;
            ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

            VkFramebufferAttachmentImageInfoKHR fb_fdm = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
            fb_fdm.usage = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
            fb_fdm.width = frame_width;
            fb_fdm.height = frame_height;
            fb_fdm.layerCount = 1;
            fb_fdm.viewFormatCount = 1;
            fb_fdm.pViewFormats = &attachment_format;
            VkFramebufferAttachmentsCreateInfoKHR fb_aci_fdm = LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
            fb_aci_fdm.attachmentImageInfoCount = 1;
            fb_aci_fdm.pAttachmentImageInfos = &fb_fdm;

            VkFramebufferCreateInfo fbci = LvlInitStruct<VkFramebufferCreateInfo>(&fb_aci_fdm);
            fbci.flags = 0;
            fbci.width = frame_width;
            fbci.height = frame_height;
            fbci.layers = 1;
            fbci.renderPass = rp_fragment_density_map;
            fbci.attachmentCount = 1;
            fbci.pAttachments = &view_fragment_density_map;

            // Set small width
            VkImageObj image2(m_device);
            image2.Init(16, attachment_height, 1, attachment_format, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
                        VK_IMAGE_TILING_LINEAR, 0);
            ASSERT_TRUE(image2.initialized());

            ivci.image = image2.handle();
            err = vk::CreateImageView(m_device->device(), &ivci, NULL, &view_fragment_density_map);
            ASSERT_VK_SUCCESS(err);

            fbci.pAttachments = &view_fragment_density_map;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-02555");
            err = vk::CreateFramebuffer(device(), &fbci, NULL, &fb);

            m_errorMonitor->VerifyFound();
            if (err == VK_SUCCESS) {
                vk::DestroyFramebuffer(m_device->device(), fb, NULL);
            }

            vk::DestroyImageView(m_device->device(), view_fragment_density_map, NULL);

            // Set small height
            VkImageObj image3(m_device);
            image3.Init(attachment_width, 16, 1, attachment_format, VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
                        VK_IMAGE_TILING_LINEAR, 0);
            ASSERT_TRUE(image3.initialized());

            ivci.image = image3.handle();
            err = vk::CreateImageView(m_device->device(), &ivci, NULL, &view_fragment_density_map);
            ASSERT_VK_SUCCESS(err);

            fbci.pAttachments = &view_fragment_density_map;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-02556");
            err = vk::CreateFramebuffer(device(), &fbci, NULL, &fb);

            m_errorMonitor->VerifyFound();
            if (err == VK_SUCCESS) {
                vk::DestroyFramebuffer(m_device->device(), fb, NULL);
            }

            vk::DestroyImageView(m_device->device(), view_fragment_density_map, NULL);

            vk::DestroyRenderPass(m_device->device(), rp_fragment_density_map, NULL);
        }
    }

    {
        // Create an image with one mip level.
        VkImageObj image(m_device);
        image.Init(128, 128, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image.initialized());

        // Create view attachment with non-identity swizzle
        VkImageView view;
        VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
        ivci.image = image.handle();
        ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.components.r = VK_COMPONENT_SWIZZLE_G;
        ivci.components.g = VK_COMPONENT_SWIZZLE_R;
        ivci.components.b = VK_COMPONENT_SWIZZLE_A;
        ivci.components.a = VK_COMPONENT_SWIZZLE_B;
        err = vk::CreateImageView(m_device->device(), &ivci, NULL, &view);
        ASSERT_VK_SUCCESS(err);

        fb_info.pAttachments = &view;
        fb_info.height = 100;
        fb_info.width = 100;
        fb_info.layers = 1;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-00884");
        err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);

        m_errorMonitor->VerifyFound();
        if (err == VK_SUCCESS) {
            vk::DestroyFramebuffer(m_device->device(), fb, NULL);
        }
        vk::DestroyImageView(m_device->device(), view, NULL);
    }

    {
        if (!multiviewSupported) {
            printf("%s VK_KHR_Multiview Extension not supported, skipping tests\n", kSkipPrefix);
        } else {
            // Test multiview renderpass with more than 1 layer
            uint32_t viewMasks[] = {0x3u};
            VkRenderPassMultiviewCreateInfo rpmvci = {
                VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 1, viewMasks, 0, nullptr, 0, nullptr};

            VkRenderPassCreateInfo rpci_mv = {
                VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 1, &subpass, 0, nullptr};

            VkRenderPass rp_mv;
            err = vk::CreateRenderPass(m_device->device(), &rpci_mv, NULL, &rp_mv);
            ASSERT_VK_SUCCESS(err);

            VkFramebufferCreateInfo fb_info_mv = fb_info;
            fb_info_mv.layers = 2;
            fb_info_mv.attachmentCount = 0;
            fb_info_mv.renderPass = rp_mv;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-02531");
            err = vk::CreateFramebuffer(device(), &fb_info_mv, NULL, &fb);
            m_errorMonitor->VerifyFound();
            if (err == VK_SUCCESS) {
                vk::DestroyFramebuffer(m_device->device(), fb, NULL);
            }

            vk::DestroyRenderPass(m_device->device(), rp_mv, NULL);

            VkAttachmentDescription attach_desc_mv = {};
            attach_desc_mv.format = VK_FORMAT_R8G8B8A8_UNORM;
            attach_desc_mv.finalLayout = attach_desc_mv.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
            attach_desc_mv.samples = VK_SAMPLE_COUNT_1_BIT;
            rpci_mv.attachmentCount = 1;
            rpci_mv.pAttachments = &attach_desc_mv;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &attach;
            err = vk::CreateRenderPass(m_device->device(), &rpci_mv, NULL, &rp_mv);
            ASSERT_VK_SUCCESS(err);

            // Create an image with 1 layer
            VkImageObj image(m_device);
            image.Init(128, 128, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
            ASSERT_TRUE(image.initialized());

            VkImageView view;
            VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
            ivci.image = image.handle();
            ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
            ivci.subresourceRange.layerCount = 1;
            ivci.subresourceRange.baseMipLevel = 0;
            ivci.subresourceRange.levelCount = 1;
            ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            err = vk::CreateImageView(m_device->device(), &ivci, NULL, &view);
            ASSERT_VK_SUCCESS(err);

            fb_info_mv.renderPass = rp_mv;
            fb_info_mv.layers = 1;
            fb_info_mv.pAttachments = &view;
            fb_info_mv.attachmentCount = 1;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-renderPass-04536");
            err = vk::CreateFramebuffer(device(), &fb_info_mv, NULL, &fb);
            m_errorMonitor->VerifyFound();
            if (err == VK_SUCCESS) {
                vk::DestroyFramebuffer(m_device->device(), fb, NULL);
            }

            vk::DestroyRenderPass(m_device->device(), rp_mv, NULL);
            vk::DestroyImageView(m_device->device(), view, NULL);
        }
    }

    // reset attachment to color attachment
    fb_info.pAttachments = ivs;

    // Request fb that exceeds max width
    fb_info.width = m_device->props.limits.maxFramebufferWidth + 1;
    fb_info.height = 100;
    fb_info.layers = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-width-00886");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04533");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    // and width=0
    fb_info.width = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-width-00885");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }

    // Request fb that exceeds max height
    fb_info.width = 100;
    fb_info.height = m_device->props.limits.maxFramebufferHeight + 1;
    fb_info.layers = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-height-00888");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04534");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    // and height=0
    fb_info.height = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-height-00887");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }

    // Request fb that exceeds max layers
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = m_device->props.limits.maxFramebufferLayers + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-layers-00890");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04535");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }
    // and layers=0
    fb_info.layers = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-layers-00889");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }

    // Try to create with pAttachments = NULL
    fb_info.layers = 1;
    fb_info.pAttachments = NULL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID_Undefined");
    err = vk::CreateFramebuffer(device(), &fb_info, NULL, &fb);
    m_errorMonitor->VerifyFound();
    if (err == VK_SUCCESS) {
        vk::DestroyFramebuffer(m_device->device(), fb, NULL);
    }

    vk::DestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, AllocDescriptorFromEmptyPool) {
    TEST_DESCRIPTION("Attempt to allocate more sets and descriptors than descriptor pool has available.");
    VkResult err;
    SetTargetApiVersion(VK_API_VERSION_1_0);

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // This test is valid for Vulkan 1.0 only -- skip if device has an API version greater than 1.0.
    if (DeviceValidationVersion() >= VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests for 1.0 only";
    }

    // Create Pool w/ 1 Sampler descriptor, but try to alloc Uniform Buffer
    // descriptor from it
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_samp(m_device, {dsl_binding_samp});

    // Try to allocate 2 sets when pool only has 1 set
    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout_samp.handle(), ds_layout_samp.handle()};
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = set_layouts;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetAllocateInfo-descriptorSetCount-00306");
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets);
    m_errorMonitor->VerifyFound();

    alloc_info.descriptorSetCount = 1;
    // Create layout w/ descriptor type not available in pool
    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_ub(m_device, {dsl_binding});

    VkDescriptorSet descriptor_set;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ds_layout_ub.handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetAllocateInfo-descriptorPool-00307");
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_set);

    m_errorMonitor->VerifyFound();

    vk::DestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, FreeDescriptorFromOneShotPool) {
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeDescriptorSets-descriptorPool-00312");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = 0;
    // Not specifying VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT means
    // app can only call vk::ResetDescriptorPool on this pool.;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);
    ASSERT_VK_SUCCESS(err);

    err = vk::FreeDescriptorSets(m_device->device(), ds_pool, 1, &descriptorSet);
    m_errorMonitor->VerifyFound();

    vk::DestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, InvalidDescriptorPool) {
    // Attempt to clear Descriptor Pool with bad object.
    // ObjectTracker should catch this.

    ASSERT_NO_FATAL_FAILURE(Init());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetDescriptorPool-descriptorPool-parameter");
    uint64_t fake_pool_handle = 0xbaad6001;
    VkDescriptorPool bad_pool = reinterpret_cast<VkDescriptorPool &>(fake_pool_handle);
    vk::ResetDescriptorPool(device(), bad_pool, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDescriptorSet) {
    // Attempt to bind an invalid Descriptor Set to a valid Command Buffer
    // ObjectTracker should catch this.
    // Create a valid cmd buffer
    // call vk::CmdBindDescriptorSets w/ false Descriptor Set
    ASSERT_NO_FATAL_FAILURE(Init());

    uint64_t fake_set_handle = 0xbaad6001;
    VkDescriptorSet bad_set = reinterpret_cast<VkDescriptorSet &>(fake_set_handle);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {layout_binding});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout});

    m_commandBuffer->begin();
    // Set invalid set
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-parameter");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1, &bad_set,
                              0, NULL);
    m_errorMonitor->VerifyFound();

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
                                                 });
    VkDescriptorSet good_set = descriptor_set.set_;

    // Set out of range firstSet and descriptorSetCount sum
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-firstSet-00360");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindDescriptorSets-pDescriptorSets-00358");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 2, 1, &good_set,
                              0, NULL);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidDescriptorSetLayout) {
    // Attempt to create a Pipeline Layout with an invalid Descriptor Set Layout.
    // ObjectTracker should catch this.
    uint64_t fake_layout_handle = 0xbaad6001;
    VkDescriptorSetLayout bad_layout = reinterpret_cast<VkDescriptorSetLayout &>(fake_layout_handle);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-parameter");
    ASSERT_NO_FATAL_FAILURE(Init());
    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo plci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &bad_layout;
    vk::CreatePipelineLayout(device(), &plci, NULL, &pipeline_layout);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, WriteDescriptorSetIntegrityCheck) {
    TEST_DESCRIPTION(
        "This test verifies some requirements of chapter 13.2.3 of the Vulkan Spec "
        "1) A uniform buffer update must have a valid buffer index. "
        "2) When using an array of descriptors in a single WriteDescriptor, the descriptor types and stageflags "
        "must all be the same. "
        "3) Immutable Sampler state must match across descriptors. "
        "4) That sampled image descriptors have required layouts. "
        "5) That it is prohibited to write to an immutable sampler. ");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkResult err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    OneOffDescriptorSet::Bindings bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, NULL},
        {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL},
        {2, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<VkSampler *>(&sampler)},
        {3, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL},
        {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, VK_SHADER_STAGE_FRAGMENT_BIT, NULL},
        {5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, NULL},
        {6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, NULL}};
    OneOffDescriptorSet descriptor_set(m_device, bindings);
    ASSERT_TRUE(descriptor_set.Initialized());

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    // 1) The uniform buffer is intentionally invalid here
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00324");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // Create a buffer to update the descriptor with
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBufferObj dynamic_uniform_buffer;
    dynamic_uniform_buffer.init(*m_device, buffCI);

    VkDescriptorBufferInfo buffInfo[5] = {};
    for (int i = 0; i < 5; ++i) {
        buffInfo[i].buffer = dynamic_uniform_buffer.handle();
        buffInfo[i].offset = 0;
        buffInfo[i].range = 1024;
    }
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = buffInfo;

    // 2) The stateFlags and type don't match between the first and second descriptor
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorCount-00317");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstBinding = 4;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorCount-00317");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstBinding = 4;
    descriptor_write.dstArrayElement = 1;
    descriptor_write.descriptorCount = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorCount-00317");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstBinding = 4;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 4;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    descriptor_write.dstBinding = 4;
    descriptor_write.dstArrayElement = 1;
    descriptor_write.descriptorCount = 3;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // 3) The second descriptor has a null_ptr pImmutableSamplers and the third descriptor contains an immutable sampler
    descriptor_write.dstBinding = 1;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

    // Make pImageInfo index non-null to avoid complaints of it missing
    VkDescriptorImageInfo imageInfo[2] = {};
    imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_write.pImageInfo = imageInfo;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorCount-00318");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // 4) That sampled image descriptors have required layouts -- create images to update the descriptor with
    VkImageObj image(m_device);
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    // Attmept write with incorrect layout for sampled descriptor
    imageInfo[0].sampler = VK_NULL_HANDLE;
    imageInfo[0].imageView = image.targetView(tex_format);
    imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    descriptor_write.dstBinding = 3;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-04149");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // 5) Attempt to update an immutable sampler
    descriptor_write.dstBinding = 2;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02752");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    vk::DestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, WriteDescriptorSetIdentitySwizzle) {
    TEST_DESCRIPTION("Test descriptors that need to have identity swizzle set");
    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkImageObj image_obj(m_device);
    image_obj.Init(64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
    ASSERT_TRUE(image_obj.initialized());
    VkImage image = image_obj.image();

    VkImageViewCreateInfo image_view_ci = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_ci.image = image;
    image_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.subresourceRange.layerCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // G and B are swizzled
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_B;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_G;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;

    VkImageView image_view;
    vk::CreateImageView(m_device->device(), &image_view_ci, NULL, &image_view);
    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00336");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    vk::DestroyImageView(m_device->device(), image_view, NULL);
}

TEST_F(VkLayerTest, WriteDescriptorSetConsecutiveUpdates) {
    TEST_DESCRIPTION(
        "Verifies that updates rolling over to next descriptor work correctly by destroying buffer from consecutive update known "
        "to be used in descriptor set and verifying that error is flagged.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 2048;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer0;
    buffer0.init(*m_device, bci);
    CreatePipelineHelper pipe(*this);
    {  // Scope 2nd buffer to cause early destruction
        VkBufferObj buffer1;
        bci.size = 1024;
        buffer1.init(*m_device, bci);

        VkDescriptorBufferInfo buffer_info[3] = {};
        buffer_info[0].buffer = buffer0.handle();
        buffer_info[0].offset = 0;
        buffer_info[0].range = 1024;
        buffer_info[1].buffer = buffer0.handle();
        buffer_info[1].offset = 1024;
        buffer_info[1].range = 1024;
        buffer_info[2].buffer = buffer1.handle();
        buffer_info[2].offset = 0;
        buffer_info[2].range = 1024;

        VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstSet = descriptor_set.set_;  // descriptor_set;
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 3;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.pBufferInfo = buffer_info;

        // Update descriptor
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        // Create PSO that uses the uniform buffers
        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
            layout(set=0) layout(binding=1) uniform blah { int x; } duh;
            void main(){
               x = vec4(duh.x, bar.y, bar.x, 1);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        pipe.InitInfo();
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
        pipe.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &descriptor_set.set_, 0, nullptr);

        VkViewport viewport = {0, 0, 16, 16, 0, 1};
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        VkRect2D scissor = {{0, 0}, {16, 16}};
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();
    }
    // buffer2 just went out of scope and was destroyed
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, WriteDescriptorSetYcbcr) {
    TEST_DESCRIPTION("Attempt to use VkSamplerYcbcrConversion ImageView to update descriptors that are not allowed.");

    // Enable KHR multiplane req'd extensions
    AddRequiredExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    // Enable Sampler YCbCr Conversion
    auto vkGetPhysicalDeviceFeatures2FunctionKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2FunctionKHR != nullptr);
    auto ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ycbcr_features);
    vkGetPhysicalDeviceFeatures2FunctionKHR(gpu(), &features2);
    if (ycbcr_features.samplerYcbcrConversion == VK_FALSE) {
        GTEST_SKIP() << "samplerYcbcrConversion feature not supported. Skipped.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    PFN_vkCreateSamplerYcbcrConversionKHR vkCreateSamplerYcbcrConversionFunction =
        (PFN_vkCreateSamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR");
    PFN_vkDestroySamplerYcbcrConversionKHR vkDestroySamplerYcbcrConversionFunction =
        (PFN_vkDestroySamplerYcbcrConversionKHR)vk::GetDeviceProcAddr(m_device->handle(), "vkDestroySamplerYcbcrConversionKHR");
    ASSERT_NE(vkCreateSamplerYcbcrConversionFunction, nullptr);
    ASSERT_NE(vkDestroySamplerYcbcrConversionFunction, nullptr);

    // Create Ycbcr conversion
    VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;  // guaranteed sampling support
    VkSamplerYcbcrConversionCreateInfo ycbcr_create_info = {VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO,
                                                            NULL,
                                                            mp_format,
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

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkImageObj image_obj(m_device);
    VkImageCreateInfo image_ci = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                  nullptr,
                                  VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,  // need for multi-planar
                                  VK_IMAGE_TYPE_2D,
                                  mp_format,
                                  {64, 64, 1},
                                  1,
                                  1,
                                  VK_SAMPLE_COUNT_1_BIT,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_IMAGE_USAGE_SAMPLED_BIT,
                                  VK_SHARING_MODE_EXCLUSIVE,
                                  0,
                                  nullptr,
                                  VK_IMAGE_LAYOUT_UNDEFINED};
    image_obj.init(&image_ci);
    ASSERT_TRUE(image_obj.initialized());

    VkSamplerYcbcrConversionInfo ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
    ycbcr_info.conversion = conversion;

    VkImageView image_view;
    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>(&ycbcr_info);
    image_view_create_info.image = image_obj.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = mp_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk::CreateImageView(m_device->device(), &image_view_create_info, nullptr, &image_view);

    descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-01946");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vk::DestroyImageView(m_device->device(), image_view, NULL);
    vkDestroySamplerYcbcrConversionFunction(m_device->device(), conversion, nullptr);
}

TEST_F(VkLayerTest, InvalidCmdBufferDescriptorSetBufferDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a bound descriptor set with a buffer dependency being "
        "destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    {
        // Create a buffer to update the descriptor with
        uint32_t qfi = 0;
        VkBufferCreateInfo buffCI = LvlInitStruct<VkBufferCreateInfo>();
        buffCI.size = 1024;
        buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffCI.queueFamilyIndexCount = 1;
        buffCI.pQueueFamilyIndices = &qfi;

        VkBufferObj buffer;
        buffer.init(*m_device, buffCI);

        // Create PSO to be used for draw-time errors below
        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
            void main(){
               x = vec4(bar.y);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        pipe.InitInfo();
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Correctly update descriptor to avoid "NOT_UPDATED" error
        pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024);
        pipe.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                                  &pipe.descriptor_set_->set_, 0, NULL);

        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &m_viewports[0]);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &m_scissors[0]);

        m_commandBuffer->Draw(1, 0, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
    // Destroy buffer should invalidate the cmd buffer, causing error on submit

    // Attempt to submit cmd buffer
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // Invalid VkBuffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkBuffer");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

// This is similar to the InvalidCmdBufferDescriptorSetBufferDestroyed test above except that the buffer
// is destroyed before recording the Draw cmd.
TEST_F(VkLayerTest, InvalidDrawDescriptorSetBufferDestroyed) {
    TEST_DESCRIPTION("Attempt to bind a descriptor set that is invalid at Draw time due to its buffer dependency being destroyed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    {
        // Create a buffer to update the descriptor with
        uint32_t qfi = 0;
        VkBufferCreateInfo buffCI = LvlInitStruct<VkBufferCreateInfo>();
        buffCI.size = 1024;
        buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffCI.queueFamilyIndexCount = 1;
        buffCI.pQueueFamilyIndices = &qfi;

        VkBufferObj buffer;
        buffer.init(*m_device, buffCI);

        // Create PSO to be used for draw-time errors below
        char const *fsSource = R"glsl(
            #version 450
            layout(location=0) out vec4 x;
            layout(set=0) layout(binding=0) uniform foo { int x; int y; } bar;
            void main(){
               x = vec4(bar.y);
            }
        )glsl";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
        pipe.InitInfo();
        pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dyn_state_ci = LvlInitStruct<VkPipelineDynamicStateCreateInfo>();
        dyn_state_ci.dynamicStateCount = size(dyn_states);
        dyn_state_ci.pDynamicStates = dyn_states;
        pipe.dyn_state_ci_ = dyn_state_ci;
        pipe.InitState();
        pipe.CreateGraphicsPipeline();

        // Correctly update descriptor to avoid "NOT_UPDATED" error
        pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024);
        pipe.descriptor_set_->UpdateDescriptorSets();
    }

    // The buffer has now been destroyed, but it has been written into the descriptor set.

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, NULL);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &m_viewports[0]);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &m_scissors[0]);

    // Invalid VkBuffer - The check is made at Draw time.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "that is invalid or has been destroyed");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidCmdBufferDescriptorSetImageSamplerDestroyed) {
    TEST_DESCRIPTION(
        "Attempt to draw with a command buffer that is invalid due to a bound descriptor sets with a combined image sampler having "
        "their image, sampler, and descriptor set each respectively destroyed and then attempting to submit associated cmd "
        "buffers. Attempt to destroy a DescriptorSet that is in use.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool;
    VkResult err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding});

    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = &ds_layout.handle();
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptorSet);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layout});

    // Create images to update the descriptor with
    VkImage image;
    VkImage image2;
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
    err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image2);
    ASSERT_VK_SUCCESS(err);

    VkMemoryRequirements memory_reqs;
    VkDeviceMemory image_memory;
    bool pass;
    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    // Allocate enough memory for both images
    VkDeviceSize align_mod = memory_reqs.size % memory_reqs.alignment;
    VkDeviceSize aligned_size = ((align_mod == 0) ? memory_reqs.size : (memory_reqs.size + memory_reqs.alignment - align_mod));
    memory_info.allocationSize = aligned_size * 2;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &image_memory);
    ASSERT_VK_SUCCESS(err);
    err = vk::BindImageMemory(m_device->device(), image, image_memory, 0);
    ASSERT_VK_SUCCESS(err);
    // Bind second image to memory right after first image
    err = vk::BindImageMemory(m_device->device(), image2, image_memory, aligned_size);
    ASSERT_VK_SUCCESS(err);

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView tmp_view;  // First test deletes this view
    VkImageView view;
    VkImageView view2;
    err = vk::CreateImageView(m_device->device(), &image_view_create_info, NULL, &tmp_view);
    ASSERT_VK_SUCCESS(err);
    err = vk::CreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    image_view_create_info.image = image2;
    err = vk::CreateImageView(m_device->device(), &image_view_create_info, NULL, &view2);
    ASSERT_VK_SUCCESS(err);
    // Create Samplers
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkSampler sampler2;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler2);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler;
    img_info.imageView = tmp_view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptorSet;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to be used for draw-time errors below
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

    // First error case is destroying sampler prior to cmd buffer submission
    m_commandBuffer->begin();

    // Transit image layout from VK_IMAGE_LAYOUT_UNDEFINED into VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier barrier = LvlInitStruct<VkImageMemoryBarrier>();
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptorSet, 0, NULL);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    // This first submit should be successful
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    // Now destroy imageview and reset cmdBuffer
    vk::DestroyImageView(m_device->device(), tmp_view, NULL);
    m_commandBuffer->reset(0);
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptorSet, 0, NULL);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " that is invalid or has been destroyed.");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Re-update descriptor with new view
    img_info.imageView = view;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    // Now test destroying sampler prior to cmd buffer submission
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptorSet, 0, NULL);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Destroy sampler invalidates the cmd buffer, causing error on submit
    vk::DestroySampler(m_device->device(), sampler, NULL);
    // Attempt to submit cmd buffer
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkSampler");
    submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Now re-update descriptor with valid sampler and delete image
    img_info.sampler = sampler2;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    VkCommandBufferBeginInfo info = LvlInitStruct<VkCommandBufferBeginInfo>();
    info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkImage");
    m_commandBuffer->begin(&info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptorSet, 0, NULL);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Destroy image invalidates the cmd buffer, causing error on submit
    vk::DestroyImage(m_device->device(), image, NULL);
    // Attempt to submit cmd buffer
    submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    // Now update descriptor to be valid, but then update and free descriptor
    img_info.imageView = view2;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_commandBuffer->begin(&info);

    // Transit image2 layout from VK_IMAGE_LAYOUT_UNDEFINED into VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    barrier.image = image2;
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,
                           nullptr, 0, nullptr, 1, &barrier);

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptorSet, 0, NULL);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);

    // Immediately try to update the descriptor set in the active command buffer - failure expected
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkUpdateDescriptorSets-None-03047");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    // Immediately try to destroy the descriptor set in the active command buffer - failure expected
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkFreeDescriptorSets-pDescriptorSets-00309");
    vk::FreeDescriptorSets(m_device->device(), ds_pool, 1, &descriptorSet);
    m_errorMonitor->VerifyFound();

    // Try again once the queue is idle - should succeed w/o error
    // TODO - though the particular error above doesn't re-occur, there are other 'unexpecteds' still to clean up
    vk::QueueWaitIdle(m_device->m_queue);
    m_errorMonitor->SetUnexpectedError(
        "pDescriptorSets must be a valid pointer to an array of descriptorSetCount VkDescriptorSet handles, each element of which "
        "must either be a valid handle or VK_NULL_HANDLE");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorSet obj");
    vk::FreeDescriptorSets(m_device->device(), ds_pool, 1, &descriptorSet);

    // Attempt to submit cmd buffer containing the freed descriptor set
    submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDescriptorSet");
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Cleanup
    vk::FreeMemory(m_device->device(), image_memory, NULL);
    vk::DestroySampler(m_device->device(), sampler2, NULL);
    vk::DestroyImage(m_device->device(), image2, NULL);
    vk::DestroyImageView(m_device->device(), view, NULL);
    vk::DestroyImageView(m_device->device(), view2, NULL);
    vk::DestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, InvalidDescriptorSetSamplerDestroyed) {
    TEST_DESCRIPTION("Attempt to draw with a bound descriptor sets with a combined image sampler where sampler has been deleted.");
    ASSERT_NO_FATAL_FAILURE(Init(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    // Create images to update the descriptor with
    VkImageObj image(m_device);
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    image.Init(32, 32, 1, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_create_info.image = image.handle();
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView view;
    VkResult err = vk::CreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    // Create Samplers
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    VkSampler sampler1;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler1);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler;
    img_info.imageView = view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo img_info1 = img_info;
    img_info1.sampler = sampler1;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    std::array<VkWriteDescriptorSet, 2> descriptor_writes = {{descriptor_write, descriptor_write}};
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].pImageInfo = &img_info1;

    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes.data(), 0, NULL);

    // Destroy the sampler before it's bound to the cmd buffer
    vk::DestroySampler(m_device->device(), sampler1, NULL);

    // Create PSO to be used for draw-time errors below
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D s;
        layout(set=0, binding=1) uniform sampler2D s1;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(s, vec2(1));
           x = texture(s1, vec2(1));
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    // First error case is destroying sampler prior to cmd buffer submission
    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " Descriptor in binding #1 index 0 is using sampler ");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroySampler(m_device->device(), sampler, NULL);
    vk::DestroyImageView(m_device->device(), view, NULL);
}

TEST_F(VkLayerTest, ImageDescriptorLayoutMismatch) {
    TEST_DESCRIPTION("Create an image sampler layout->image layout mismatch within/without a command buffer");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool maint2_support = DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    if (maint2_support) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    } else {
        printf("%s Relaxed layout matching subtest requires API >= 1.1 or KHR_MAINTENANCE2 extension, unavailable - skipped.\n",
               kSkipPrefix);
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });
    VkDescriptorSet descriptorSet = descriptor_set.set_;

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    // Create image, view, and sampler
    const VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImageObj image(m_device);
    image.Init(32, 32, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_TILING_OPTIMAL,
               0);
    ASSERT_TRUE(image.initialized());

    vk_testing::ImageView view;
    auto image_view_create_info = SafeSaneImageViewCreateInfo(image, format, VK_IMAGE_ASPECT_COLOR_BIT);
    view.init(*m_device, image_view_create_info);
    ASSERT_TRUE(view.initialized());

    // Create Sampler
    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);
    ASSERT_TRUE(sampler.initialized());

    // Setup structure for descriptor update with sampler, for update in do_test below
    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler.handle();

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptorSet;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;

    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};

    VkCommandBufferObj cmd_buf(m_device, m_commandPool);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buf.handle();

    enum TestType {
        kInternal,  // Image layout mismatch is *within* a given command buffer
        kExternal   // Image layout mismatch is with the current state of the image, found at QueueSubmit
    };
    std::array<TestType, 2> test_list = {{kInternal, kExternal}};
    const std::vector<std::string> internal_errors = {"VUID-VkDescriptorImageInfo-imageLayout-00344", "VUID-vkCmdDraw-None-02699"};
    const std::vector<std::string> external_errors = {"UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout"};

    // Common steps to create the two classes of errors (or two classes of positives)
    auto do_test = [&](VkImageObj *image, vk_testing::ImageView *view, VkImageAspectFlags aspect_mask, VkImageLayout image_layout,
                       VkImageLayout descriptor_layout, const bool positive_test) {
        // Set up the descriptor
        img_info.imageView = view->handle();
        img_info.imageLayout = descriptor_layout;
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        for (TestType test_type : test_list) {
            cmd_buf.begin();
            // record layout different than actual descriptor layout.
            const VkFlags read_write = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            auto image_barrier = image->image_memory_barrier(read_write, read_write, VK_IMAGE_LAYOUT_UNDEFINED, image_layout,
                                                             image->subresource_range(aspect_mask));
            cmd_buf.PipelineBarrier(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr, 0,
                                    nullptr, 1, &image_barrier);
            image->Layout(image_layout);

            if (test_type == kExternal) {
                // The image layout is external to the command buffer we are recording to test.  Submit to push to instance scope.
                cmd_buf.end();
                vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
                vk::QueueWaitIdle(m_device->m_queue);
                cmd_buf.begin();
            }

            cmd_buf.BeginRenderPass(m_renderPassBeginInfo);
            vk::CmdBindPipeline(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
            vk::CmdBindDescriptorSets(cmd_buf.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                      &descriptorSet, 0, NULL);
            vk::CmdSetViewport(cmd_buf.handle(), 0, 1, &viewport);
            vk::CmdSetScissor(cmd_buf.handle(), 0, 1, &scissor);

            // At draw time the update layout will mis-match the actual layout
            if (positive_test || (test_type == kExternal)) {
            } else {
                for (const auto &err : internal_errors) {
                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, err.c_str());
                }
            }
            cmd_buf.Draw(1, 0, 0, 0);
            if (positive_test || (test_type == kExternal)) {
            } else {
                m_errorMonitor->VerifyFound();
            }

            cmd_buf.EndRenderPass();
            cmd_buf.end();

            // Submit cmd buffer
            if (positive_test || (test_type == kInternal)) {
            } else {
                for (const auto &err : external_errors) {
                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, err.c_str());
                }
            }
            vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
            vk::QueueWaitIdle(m_device->m_queue);
            if (positive_test || (test_type == kInternal)) {
            } else {
                m_errorMonitor->VerifyFound();
            }
        }
    };
    do_test(&image, &view, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, /* positive */ false);

    // Create depth stencil image and views
    const VkFormat format_ds = m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());
    bool ds_test_support = maint2_support && (format_ds != VK_FORMAT_UNDEFINED);
    VkImageObj image_ds(m_device);
    vk_testing::ImageView stencil_view;
    vk_testing::ImageView depth_view;
    const VkImageLayout ds_image_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    const VkImageLayout depth_descriptor_layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    const VkImageLayout stencil_descriptor_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    const VkImageAspectFlags depth_stencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    if (ds_test_support) {
        image_ds.Init(32, 32, 1, format_ds, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_IMAGE_TILING_OPTIMAL, 0);
        ASSERT_TRUE(image_ds.initialized());
        auto ds_view_ci = SafeSaneImageViewCreateInfo(image_ds, format_ds, VK_IMAGE_ASPECT_DEPTH_BIT);
        depth_view.init(*m_device, ds_view_ci);
        ds_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        stencil_view.init(*m_device, ds_view_ci);
        do_test(&image_ds, &depth_view, depth_stencil, ds_image_layout, depth_descriptor_layout, /* positive */ true);
        do_test(&image_ds, &depth_view, depth_stencil, ds_image_layout, VK_IMAGE_LAYOUT_GENERAL, /* positive */ false);
        do_test(&image_ds, &stencil_view, depth_stencil, ds_image_layout, stencil_descriptor_layout, /* positive */ true);
        do_test(&image_ds, &stencil_view, depth_stencil, ds_image_layout, VK_IMAGE_LAYOUT_GENERAL, /* positive */ false);
    }
}

TEST_F(VkLayerTest, DescriptorPoolInUseResetSignaled) {
    TEST_DESCRIPTION("Reset a DescriptorPool with a DescriptorSet that is in use.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    // Create image to update the descriptor with
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);
    // Create Sampler
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    VkResult err = vk::CreateSampler(m_device->device(), &sampler_ci, nullptr, &sampler);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    descriptor_set.UpdateDescriptorSets();

    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragSamplerShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Submit cmd buffer to put pool in-flight
    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    // Reset pool while in-flight, causing error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkResetDescriptorPool-descriptorPool-00313");
    vk::ResetDescriptorPool(m_device->device(), descriptor_set.pool_, 0);
    m_errorMonitor->VerifyFound();
    vk::QueueWaitIdle(m_device->m_queue);
    // Cleanup
    vk::DestroySampler(m_device->device(), sampler, nullptr);
    m_errorMonitor->SetUnexpectedError(
        "If descriptorPool is not VK_NULL_HANDLE, descriptorPool must be a valid VkDescriptorPool handle");
    m_errorMonitor->SetUnexpectedError("Unable to remove DescriptorPool obj");
}

TEST_F(VkLayerTest, DescriptorImageUpdateNoMemoryBound) {
    TEST_DESCRIPTION("Attempt an image descriptor set update where image's bound memory has been freed.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    // Create images to update the descriptor with
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
    VkResult err = vk::CreateImage(m_device->device(), &image_create_info, NULL, &image);
    ASSERT_VK_SUCCESS(err);
    // Initially bind memory to avoid error at bind view time. We'll break binding before update.
    VkMemoryRequirements memory_reqs;
    VkDeviceMemory image_memory;
    bool pass;
    VkMemoryAllocateInfo memory_info = LvlInitStruct<VkMemoryAllocateInfo>();
    memory_info.allocationSize = 0;
    memory_info.memoryTypeIndex = 0;
    vk::GetImageMemoryRequirements(m_device->device(), image, &memory_reqs);
    // Allocate enough memory for image
    memory_info.allocationSize = memory_reqs.size;
    pass = m_device->phy().set_memory_type(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    err = vk::AllocateMemory(m_device->device(), &memory_info, NULL, &image_memory);
    ASSERT_VK_SUCCESS(err);
    err = vk::BindImageMemory(m_device->device(), image, image_memory, 0);
    ASSERT_VK_SUCCESS(err);

    VkImageViewCreateInfo image_view_create_info = LvlInitStruct<VkImageViewCreateInfo>();
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = tex_format;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageView view;
    err = vk::CreateImageView(m_device->device(), &image_view_create_info, NULL, &view);
    ASSERT_VK_SUCCESS(err);
    // Create Samplers
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    // Update descriptor with image and sampler
    descriptor_set.WriteDescriptorImageInfo(0, view, sampler);
    // Break memory binding and attempt update
    vk::FreeMemory(m_device->device(), image_memory, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-BoundResourceFreedMemoryAccess");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-BoundResourceFreedMemoryAccess");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    // Cleanup
    vk::DestroyImage(m_device->device(), image, NULL);
    vk::DestroySampler(m_device->device(), sampler, NULL);
    vk::DestroyImageView(m_device->device(), view, NULL);
}

TEST_F(VkLayerTest, InvalidDynamicOffsetCases) {
    // Create a descriptorSet w/ dynamic descriptor and then hit 3 offset error
    // cases:
    // 1. No dynamicOffset supplied
    // 2. Too many dynamicOffsets supplied
    // 3. Dynamic offset oversteps buffer being updated
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    // Create a buffer to update the descriptor with
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBufferObj dynamic_uniform_buffer;
    dynamic_uniform_buffer.init(*m_device, buffCI);

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer.handle(), 0, 1024,
                                             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, NULL);
    m_errorMonitor->VerifyFound();
    uint32_t pDynOff[2] = {0, 756};
    // Now cause error b/c too many dynOffsets in array for # of dyn descriptors
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-dynamicOffsetCount-00359");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 2, pDynOff);
    m_errorMonitor->VerifyFound();
    pDynOff[0] = 512;
    // Finally cause error due to dynamicOffset being too big
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // This update should succeed, but offset size of 512 will overstep buffer
    // /w range 1024 & size 1024
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 1, pDynOff);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DescriptorBufferUpdateNoMemoryBound) {
    TEST_DESCRIPTION("Attempt to update a descriptor with a non-sparse buffer that doesn't have memory bound");
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00329");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00329");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    // Create a buffer to update the descriptor with
    uint32_t qfi = 0;
    VkBufferCreateInfo buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffCI.queueFamilyIndexCount = 1;
    buffCI.pQueueFamilyIndices = &qfi;

    VkBuffer dynamic_uniform_buffer;
    err = vk::CreateBuffer(m_device->device(), &buffCI, NULL, &dynamic_uniform_buffer);
    ASSERT_VK_SUCCESS(err);

    // Attempt to update descriptor without binding memory to it
    descriptor_set.WriteDescriptorBufferInfo(0, dynamic_uniform_buffer, 0, 1024, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
    vk::DestroyBuffer(m_device->device(), dynamic_uniform_buffer, NULL);
}

TEST_F(VkLayerTest, InvalidDynamicDescriptorSet) {
    ASSERT_NO_FATAL_FAILURE(Init());

    const VkDeviceSize partial_size = m_device->props.limits.minUniformBufferOffsetAlignment;
    const VkDeviceSize buffer_size = partial_size * 10;  // make sure way more then alignment multiple

    // Create a buffer to update the descriptor with
    uint32_t qfi = 0;
    VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = buffer_size;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_ci.queueFamilyIndexCount = 1;
    buffer_ci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    // test various uses of offsets and size
    // The non-dynamic binds are there to make sure pDynamicOffsets are matched correctly at bind time
    // clang-format off
    OneOffDescriptorSet descriptor_set_0(m_device, {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_ALL, nullptr},
        // Gap to ensure looping for binding index is correct
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr}  // pDynamicOffsets[0]
    });
    OneOffDescriptorSet descriptor_set_1(m_device, {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_ALL, nullptr},
        // This dynamic type has a descriptorCount of 0 which will be skipped
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0, VK_SHADER_STAGE_ALL, nullptr},
    });
    OneOffDescriptorSet descriptor_set_2(m_device, {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},  // pDynamicOffsets[1]
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1, VK_SHADER_STAGE_ALL, nullptr},
        // [2] and [3] are same, but tests descriptor arrays
        {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2, VK_SHADER_STAGE_ALL, nullptr},  // pDynamicOffsets[2]/[3]
        {3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr}   // pDynamicOffsets[4]
    });
    // clang-format on
    const VkPipelineLayoutObj pipeline_layout(m_device,
                                              {&descriptor_set_0.layout_, &descriptor_set_1.layout_, &descriptor_set_2.layout_});
    const VkPipelineLayout layout = pipeline_layout.handle();

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    descriptor_set_0.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE);  // non-dynamic
    descriptor_set_1.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE);  // non-dynamic
    descriptor_set_2.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE);  // non-dynamic

    // buffer[0, max]
    descriptor_set_0.WriteDescriptorBufferInfo(2, buffer.handle(), 0, VK_WHOLE_SIZE,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);  // pDynamicOffsets[0]
    // buffer[alignment, max]
    descriptor_set_2.WriteDescriptorBufferInfo(0, buffer.handle(), partial_size, VK_WHOLE_SIZE,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);  // pDynamicOffsets[1]
    // buffer[0, max - alignment]
    descriptor_set_2.WriteDescriptorBufferInfo(2, buffer.handle(), 0, buffer_size - partial_size,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 0, 1);  // pDynamicOffsets[2]
    // buffer[0, max - alignment]
    descriptor_set_2.WriteDescriptorBufferInfo(2, buffer.handle(), 0, buffer_size - partial_size,
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, 1);  // pDynamicOffsets[3]
    // buffer[alignment, max - alignment]
    descriptor_set_2.WriteDescriptorBufferInfo(3, buffer.handle(), partial_size, buffer_size - (partial_size * 2),
                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);  // pDynamicOffsets[4]

    descriptor_set_0.UpdateDescriptorSets();
    descriptor_set_1.UpdateDescriptorSets();
    descriptor_set_2.UpdateDescriptorSets();

    m_commandBuffer->begin();

    VkDescriptorSet descriptorSets[3] = {descriptor_set_0.set_, descriptor_set_1.set_, descriptor_set_2.set_};
    uint32_t offsets[5] = {0, 0, 0, 0, 0};

    if (partial_size > 1) {
        // non multiple of alignment
        offsets[4] = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDynamicOffsets-01971");
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5,
                                  offsets);
        m_errorMonitor->VerifyFound();
        offsets[4] = 0;
    }

    // Larger than buffer
    offsets[0] = static_cast<uint32_t>(partial_size);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06715");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[0] = 0;

    // Larger than buffer
    offsets[1] = static_cast<uint32_t>(partial_size);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-06715");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[1] = 0;

    // Makes the range the same size of buffer which is valid
    offsets[2] = static_cast<uint32_t>(partial_size);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);

    // Now an extra increment larger than buffer
    offsets[2] = static_cast<uint32_t>(partial_size * 2);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[2] = 0;

    // Same thing but with [3] to test descriptor arrays
    offsets[3] = static_cast<uint32_t>(partial_size);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);

    offsets[3] = static_cast<uint32_t>(partial_size * 2);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();
    offsets[3] = 0;

    // range should be at end of buffer (same size)
    offsets[4] = static_cast<uint32_t>(partial_size);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);

    // Now an extra increment larger than buffer
    // tests (offset + range + dynamic_offset)
    offsets[4] = static_cast<uint32_t>(partial_size * 2);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-pDescriptorSets-01979");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, descriptorSets, 5, offsets);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, DynamicOffsetWithNullBuffer) {
    TEST_DESCRIPTION("Create a descriptorSet w/ dynamic descriptors where 1 binding is inactive, but all have null buffers");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    // Update descriptors
    const uint32_t BINDING_COUNT = 3;
    VkDescriptorBufferInfo buff_info[BINDING_COUNT] = {};
    buff_info[0].buffer = VK_NULL_HANDLE;
    buff_info[0].offset = 0;
    buff_info[0].range = 256;
    buff_info[1].buffer = VK_NULL_HANDLE;
    buff_info[1].offset = 256;
    buff_info[1].range = 512;
    buff_info[2].buffer = VK_NULL_HANDLE;
    buff_info[2].offset = 0;
    buff_info[2].range = 512;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = BINDING_COUNT;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptor_write.pBufferInfo = buff_info;

    // all 3 descriptors produce this error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferInfo-buffer-02998");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferInfo-buffer-02998");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferInfo-buffer-02998");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Create PSO to be used for draw-time errors below
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 x;
        layout(set=0) layout(binding=0) uniform foo1 { int x; int y; } bar1;
        layout(set=0) layout(binding=2) uniform foo2 { int x; int y; } bar2;
        void main(){
           x = vec4(bar1.y) + vec4(bar2.y);
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.pipeline_layout_ = VkPipelineLayoutObj(m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    uint32_t dyn_off[BINDING_COUNT] = {0, 1024, 256};
    // The 2 active descriptors produce this error
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02699");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02699");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set.set_, BINDING_COUNT, dyn_off);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, UpdateDescriptorSetMismatchType) {
    ASSERT_NO_FATAL_FAILURE(Init());

    uint32_t qfi = 0;
    VkBufferCreateInfo buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.size = m_device->props.limits.minUniformBufferOffsetAlignment;
    buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_ci.queueFamilyIndexCount = 1;
    buffer_ci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    buffer.init(*m_device, buffer_ci);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr}});

    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    // wrong type
    descriptor_set.WriteDescriptorBufferInfo(1, buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00319");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DescriptorSetCompatibility) {
    // Test various desriptorSet errors with bad binding combinations
    using std::vector;
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const uint32_t NUM_DESCRIPTOR_TYPES = 5;
    VkDescriptorPoolSize ds_type_count[NUM_DESCRIPTOR_TYPES] = {};
    ds_type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ds_type_count[0].descriptorCount = 10;
    ds_type_count[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    ds_type_count[1].descriptorCount = 2;
    ds_type_count[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    ds_type_count[2].descriptorCount = 2;
    ds_type_count[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count[3].descriptorCount = 5;
    // TODO : LunarG ILO driver currently asserts in desc.c w/ INPUT_ATTACHMENT
    // type
    // ds_type_count[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    ds_type_count[4].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    ds_type_count[4].descriptorCount = 2;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 5;
    ds_pool_ci.poolSizeCount = NUM_DESCRIPTOR_TYPES;
    ds_pool_ci.pPoolSizes = ds_type_count;

    VkDescriptorPool ds_pool;
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    ASSERT_VK_SUCCESS(err);

    static const uint32_t MAX_DS_TYPES_IN_LAYOUT = 2;
    VkDescriptorSetLayoutBinding dsl_binding[MAX_DS_TYPES_IN_LAYOUT] = {};
    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[0].descriptorCount = 5;
    dsl_binding[0].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding[0].pImmutableSamplers = NULL;

    // Create layout identical to set0 layout but w/ different stageFlags
    VkDescriptorSetLayoutBinding dsl_fs_stage_only = {};
    dsl_fs_stage_only.binding = 0;
    dsl_fs_stage_only.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_fs_stage_only.descriptorCount = 5;
    dsl_fs_stage_only.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  // Different stageFlags to cause error at
                                                                  // bind time
    dsl_fs_stage_only.pImmutableSamplers = NULL;

    vector<VkDescriptorSetLayoutObj> ds_layouts;
    // Create 4 unique layouts for full pipelineLayout, and 1 special fs-only
    // layout for error case
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    const VkDescriptorSetLayoutObj ds_layout_fs_only(m_device, {dsl_fs_stage_only});

    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    dsl_binding[0].descriptorCount = 2;
    dsl_binding[1].binding = 1;
    dsl_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    dsl_binding[1].descriptorCount = 2;
    dsl_binding[1].stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding[1].pImmutableSamplers = NULL;
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>({dsl_binding[0], dsl_binding[1]}));

    dsl_binding[0].binding = 0;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding[0].descriptorCount = 5;
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    dsl_binding[0].descriptorCount = 2;
    ds_layouts.emplace_back(m_device, std::vector<VkDescriptorSetLayoutBinding>(1, dsl_binding[0]));

    const auto &ds_vk_layouts = MakeVkHandles<VkDescriptorSetLayout>(ds_layouts);

    static const uint32_t NUM_SETS = 4;
    VkDescriptorSet descriptorSet[NUM_SETS] = {};
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorPool = ds_pool;
    alloc_info.descriptorSetCount = ds_vk_layouts.size();
    alloc_info.pSetLayouts = ds_vk_layouts.data();
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, descriptorSet);
    ASSERT_VK_SUCCESS(err);
    VkDescriptorSet ds0_fs_only = {};
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &ds_layout_fs_only.handle();
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &ds0_fs_only);
    ASSERT_VK_SUCCESS(err);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&ds_layouts[0], &ds_layouts[1]});
    // Create pipelineLayout with only one setLayout
    const VkPipelineLayoutObj single_pipe_layout(m_device, {&ds_layouts[0]});
    // Create pipelineLayout with 2 descriptor setLayout at index 0
    const VkPipelineLayoutObj pipe_layout_one_desc(m_device, {&ds_layouts[3]});
    // Create pipelineLayout with 5 SAMPLER descriptor setLayout at index 0
    const VkPipelineLayoutObj pipe_layout_five_samp(m_device, {&ds_layouts[2]});
    // Create pipelineLayout with UB type, but stageFlags for FS only
    VkPipelineLayoutObj pipe_layout_fs_only(m_device, {&ds_layout_fs_only});
    // Create pipelineLayout w/ incompatible set0 layout, but set1 is fine
    const VkPipelineLayoutObj pipe_layout_bad_set0(m_device, {&ds_layout_fs_only, &ds_layouts[1]});

    // Add buffer binding for UBO
    uint32_t qfi = 0;
    VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 8;
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    VkBufferObj buffer;
    buffer.init(*m_device, bci);
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = buffer.handle();
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptorSet[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Create PSO to be used for draw-time errors below
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragUniformShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    pipe.CreateVKPipeline(pipe_layout_fs_only.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    // TODO : Want to cause various binding incompatibility issues here to test
    // DrawState
    //  First cause various verify_layout_compatibility() fails
    //  Second disturb early and late sets and verify INFO msgs
    // VerifySetLayoutCompatibility fail cases:
    // 1. invalid VkPipelineLayout (layout) passed into vk::CmdBindDescriptorSets
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindDescriptorSets-layout-parameter");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                              CastToHandle<VkPipelineLayout, uintptr_t>(0xbaadb1be), 0, 1, &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 2. layoutIndex exceeds # of layouts in layout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " attempting to bind set to index 1");
    m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindDescriptorSets-firstSet-00360");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, single_pipe_layout.handle(), 0, 2,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 3. Pipeline setLayout[0] has 2 descriptors, but set being bound has 5
    // descriptors
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " has 2 total descriptors, but ");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_one_desc.handle(), 0, 1,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 4. same # of descriptors but mismatch in type
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " is type 'VK_DESCRIPTOR_TYPE_SAMPLER' but binding ");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_five_samp.handle(), 0, 1,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // 5. same # of descriptors but mismatch in stageFlags
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " has stageFlags VK_SHADER_STAGE_FRAGMENT_BIT but binding 0 for ");
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_fs_only.handle(), 0, 1,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->VerifyFound();

    // Now that we're done actively using the pipelineLayout that gfx pipeline
    //  was created with, we should be able to delete it. Do that now to verify
    //  that validation obeys pipelineLayout lifetime
    pipe_layout_fs_only.Reset();

    // Cause draw-time errors due to PSO incompatibilities
    // 1. Error due to not binding required set (we actually use same code as
    // above to disturb set0)
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 2,
                              &descriptorSet[0], 0, NULL);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout_bad_set0.handle(), 1, 1,
                              &descriptorSet[1], 0, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " uses set #0 but that set is not bound.");

    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissor);

    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // 2. Error due to bound set not being compatible with PSO's
    // VkPipelineLayout (diff stageFlags in this case)
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 2,
                              &descriptorSet[0], 0, NULL);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " bound as set #0 is not compatible with ");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02697");
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // Remaining clean-up
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroyDescriptorPool(m_device->device(), ds_pool, NULL);
}

TEST_F(VkLayerTest, NullRenderPass) {
    // Bind a NULL RenderPass
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "vkCmdBeginRenderPass: required parameter pRenderPassBegin specified as NULL");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    // Don't care about RenderPass handle b/c error should be flagged before
    // that
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), NULL, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, EndCommandBufferWithinRenderPass) {
    TEST_DESCRIPTION("End a command buffer with an active render pass");

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkEndCommandBuffer-commandBuffer-00060");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::EndCommandBuffer(m_commandBuffer->handle());

    m_errorMonitor->VerifyFound();

    // End command buffer properly to avoid driver issues. This is safe -- the
    // previous vk::EndCommandBuffer should not have reached the driver.
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // TODO: Add test for VK_COMMAND_BUFFER_LEVEL_SECONDARY
    // TODO: Add test for VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
}

TEST_F(VkLayerTest, DSUsageBitsErrors) {
    TEST_DESCRIPTION("Attempt to update descriptor sets for images and buffers that do not have correct usage bits sets.");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkFormat buffer_format = VK_FORMAT_R8_UNORM;
    VkFormatProperties format_properties;
    vk::GetPhysicalDeviceFormatProperties(gpu(), buffer_format, &format_properties);
    if (!(format_properties.bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT)) {
        printf("%s Device does not support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT for this format; skipped.\n", kSkipPrefix);
        return;
    }

    constexpr uint32_t kLocalDescriptorTypeRangeSize = (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT - VK_DESCRIPTOR_TYPE_SAMPLER + 1);

    std::array<VkDescriptorPoolSize, kLocalDescriptorTypeRangeSize> ds_type_count;
    for (uint32_t i = 0; i < ds_type_count.size(); ++i) {
        ds_type_count[i].type = VkDescriptorType(i);
        ds_type_count[i].descriptorCount = 1;
    }

    vk_testing::DescriptorPool ds_pool;
    ds_pool.init(*m_device, vk_testing::DescriptorPool::create_info(0, kLocalDescriptorTypeRangeSize, ds_type_count));
    ASSERT_TRUE(ds_pool.initialized());

    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings(1);
    dsl_bindings[0].binding = 0;
    dsl_bindings[0].descriptorType = VkDescriptorType(0);
    dsl_bindings[0].descriptorCount = 1;
    dsl_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_bindings[0].pImmutableSamplers = NULL;

    // Create arrays of layout and descriptor objects
    using UpDescriptorSet = std::unique_ptr<vk_testing::DescriptorSet>;
    std::vector<UpDescriptorSet> descriptor_sets;
    using UpDescriptorSetLayout = std::unique_ptr<VkDescriptorSetLayoutObj>;
    std::vector<UpDescriptorSetLayout> ds_layouts;
    descriptor_sets.reserve(kLocalDescriptorTypeRangeSize);
    ds_layouts.reserve(kLocalDescriptorTypeRangeSize);
    for (uint32_t i = 0; i < kLocalDescriptorTypeRangeSize; ++i) {
        dsl_bindings[0].descriptorType = VkDescriptorType(i);
        ds_layouts.push_back(UpDescriptorSetLayout(new VkDescriptorSetLayoutObj(m_device, dsl_bindings)));
        descriptor_sets.push_back(UpDescriptorSet(ds_pool.alloc_sets(*m_device, *ds_layouts.back())));
        ASSERT_TRUE(descriptor_sets.back()->initialized());
    }

    // Create a buffer & bufferView to be used for invalid updates
    const VkDeviceSize buffer_size = 256;
    uint8_t data[buffer_size];
    VkConstantBufferObj buffer(m_device, buffer_size, data, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT);
    VkConstantBufferObj storage_texel_buffer(m_device, buffer_size, data, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    ASSERT_TRUE(buffer.initialized() && storage_texel_buffer.initialized());

    auto buff_view_ci = vk_testing::BufferView::createInfo(buffer.handle(), VK_FORMAT_R8_UNORM);
    vk_testing::BufferView buffer_view_obj, storage_texel_buffer_view_obj;
    buffer_view_obj.init(*m_device, buff_view_ci);
    buff_view_ci.buffer = storage_texel_buffer.handle();
    storage_texel_buffer_view_obj.init(*m_device, buff_view_ci);
    ASSERT_TRUE(buffer_view_obj.initialized() && storage_texel_buffer_view_obj.initialized());
    VkBufferView buffer_view = buffer_view_obj.handle();
    VkBufferView storage_texel_buffer_view = storage_texel_buffer_view_obj.handle();

    // Create an image to be used for invalid updates
    VkImageObj image_obj(m_device);
    image_obj.InitNoLayout(64, 64, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image_obj.initialized());
    VkImageView image_view = image_obj.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer.handle();
    VkDescriptorImageInfo img_info = {};
    img_info.imageView = image_view;
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = &buffer_view;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = &img_info;

    // These error messages align with VkDescriptorType struct
    std::string error_codes[] = {
        "UNASSIGNED-CoreValidation-DrawState-InvalidImageView",  // placeholder, no error for SAMPLER descriptor
        "VUID-VkWriteDescriptorSet-descriptorType-00337",        // COMBINED_IMAGE_SAMPLER
        "VUID-VkWriteDescriptorSet-descriptorType-00337",        // SAMPLED_IMAGE
        "VUID-VkWriteDescriptorSet-descriptorType-00339",        // STORAGE_IMAGE
        "VUID-VkWriteDescriptorSet-descriptorType-00334",        // UNIFORM_TEXEL_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00335",        // STORAGE_TEXEL_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00330",        // UNIFORM_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00331",        // STORAGE_BUFFER
        "VUID-VkWriteDescriptorSet-descriptorType-00330",        // UNIFORM_BUFFER_DYNAMIC
        "VUID-VkWriteDescriptorSet-descriptorType-00331",        // STORAGE_BUFFER_DYNAMIC
        "VUID-VkWriteDescriptorSet-descriptorType-00338"         // INPUT_ATTACHMENT
    };
    // Start loop at 1 as SAMPLER desc type has no usage bit error
    for (uint32_t i = 1; i < kLocalDescriptorTypeRangeSize; ++i) {
        if (VkDescriptorType(i) == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            // Now check for UNIFORM_TEXEL_BUFFER using storage_texel_buffer_view
            descriptor_write.pTexelBufferView = &storage_texel_buffer_view;
        }
        descriptor_write.descriptorType = VkDescriptorType(i);
        descriptor_write.dstSet = descriptor_sets[i]->handle();
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, error_codes[i]);

        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

        m_errorMonitor->VerifyFound();
        if (VkDescriptorType(i) == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            descriptor_write.pTexelBufferView = &buffer_view;
        }
    }
}

TEST_F(VkLayerTest, DSBufferInfoErrors) {
    TEST_DESCRIPTION(
        "Attempt to update buffer descriptor set that has incorrect parameters in VkDescriptorBufferInfo struct. This includes:\n"
        "1. offset value greater than or equal to buffer size\n"
        "2. range value of 0\n"
        "3. range value greater than buffer (size - offset)");

    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    const bool update_template_support =
        IsExtensionsEnabled(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME) && !IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY);
    if (!update_template_support) {
        printf("%s Descriptor Update Template Extensions not supported, template cases skipped.\n", kSkipPrefix);
    }

    // Note: Includes workaround for some implementations which incorrectly return 0 maxPushDescriptors
    bool push_descriptor_support = IsExtensionsEnabled(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) &&
                                   (GetPushDescriptorProperties(instance(), gpu()).maxPushDescriptors > 0);
    if (!push_descriptor_support) {
        printf("%s Push Descriptor Extension not supported, push descriptor cases skipped.\n", kSkipPrefix);
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    std::vector<VkDescriptorSetLayoutBinding> ds_bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    OneOffDescriptorSet descriptor_set(m_device, ds_bindings);

    // Create a buffer to be used for invalid updates
    VkBufferCreateInfo buff_ci = LvlInitStruct<VkBufferCreateInfo>();
    buff_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buff_ci.size = m_device->props.limits.minUniformBufferOffsetAlignment;
    buff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkBufferObj buffer;
    buffer.init(*m_device, buff_ci);

    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer.handle();
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pTexelBufferView = nullptr;
    descriptor_write.pBufferInfo = &buff_info;
    descriptor_write.pImageInfo = nullptr;

    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.dstSet = descriptor_set.set_;

    // Relying on the "return nullptr for non-enabled extensions
    auto vkCreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateDescriptorUpdateTemplateKHR");
    auto vkDestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkDestroyDescriptorUpdateTemplateKHR");
    auto vkUpdateDescriptorSetWithTemplateKHR =
        (PFN_vkUpdateDescriptorSetWithTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkUpdateDescriptorSetWithTemplateKHR");

    if (update_template_support) {
        ASSERT_NE(vkCreateDescriptorUpdateTemplateKHR, nullptr);
        ASSERT_NE(vkDestroyDescriptorUpdateTemplateKHR, nullptr);
        ASSERT_NE(vkUpdateDescriptorSetWithTemplateKHR, nullptr);
    }

    // Setup for update w/ template tests
    // Create a template of descriptor set updates
    struct SimpleTemplateData {
        uint8_t padding[7];
        VkDescriptorBufferInfo buff_info;
        uint32_t other_padding[4];
    };
    SimpleTemplateData update_template_data = {};

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = offsetof(SimpleTemplateData, buff_info);
    update_template_entry.stride = sizeof(SimpleTemplateData);

    auto update_template_ci = LvlInitStruct<VkDescriptorUpdateTemplateCreateInfoKHR>();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = descriptor_set.layout_.handle();

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    if (update_template_support) {
        auto result = vkCreateDescriptorUpdateTemplateKHR(m_device->device(), &update_template_ci, nullptr, &update_template);
        ASSERT_VK_SUCCESS(result);
    }

    // VK_KHR_push_descriptor support
    auto vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");
    auto vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vk::GetDeviceProcAddr(
        m_device->device(), "vkCmdPushDescriptorSetWithTemplateKHR");

    std::unique_ptr<VkDescriptorSetLayoutObj> push_dsl = nullptr;
    std::unique_ptr<VkPipelineLayoutObj> pipeline_layout = nullptr;
    VkDescriptorUpdateTemplate push_template = VK_NULL_HANDLE;
    if (push_descriptor_support) {
        ASSERT_NE(vkCmdPushDescriptorSetKHR, nullptr);
        push_dsl.reset(
            new VkDescriptorSetLayoutObj(m_device, ds_bindings, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR));
        pipeline_layout.reset(new VkPipelineLayoutObj(m_device, {push_dsl.get()}));
        ASSERT_TRUE(push_dsl->initialized());

        if (update_template_support) {
            ASSERT_NE(vkCmdPushDescriptorSetWithTemplateKHR, nullptr);
            auto push_template_ci = LvlInitStruct<VkDescriptorUpdateTemplateCreateInfoKHR>();
            push_template_ci.descriptorUpdateEntryCount = 1;
            push_template_ci.pDescriptorUpdateEntries = &update_template_entry;
            push_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
            push_template_ci.descriptorSetLayout = VK_NULL_HANDLE;
            push_template_ci.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            push_template_ci.pipelineLayout = pipeline_layout->handle();
            push_template_ci.set = 0;
            auto result = vkCreateDescriptorUpdateTemplateKHR(m_device->device(), &push_template_ci, nullptr, &push_template);
            ASSERT_VK_SUCCESS(result);
        }
    }

    auto do_test = [&](const char *desired_failure) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, desired_failure);
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
        m_errorMonitor->VerifyFound();

        if (push_descriptor_support) {
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, desired_failure);
            m_commandBuffer->begin();
            vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->handle(), 0, 1,
                                      &descriptor_write);
            m_commandBuffer->end();
            m_errorMonitor->VerifyFound();
        }

        if (update_template_support) {
            update_template_data.buff_info = buff_info;  // copy the test case information into our "pData"
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, desired_failure);
            vkUpdateDescriptorSetWithTemplateKHR(m_device->device(), descriptor_set.set_, update_template, &update_template_data);
            m_errorMonitor->VerifyFound();
            if (push_descriptor_support) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, desired_failure);
                m_commandBuffer->begin();
                vkCmdPushDescriptorSetWithTemplateKHR(m_commandBuffer->handle(), push_template, pipeline_layout->handle(), 0,
                                                      &update_template_data);
                m_commandBuffer->end();
                m_errorMonitor->VerifyFound();
            }
        }
    };

    // Cause error due to offset out of range
    buff_info.offset = buff_ci.size;
    buff_info.range = VK_WHOLE_SIZE;
    do_test("VUID-VkDescriptorBufferInfo-offset-00340");

    // Now cause error due to range of 0
    buff_info.offset = 0;
    buff_info.range = 0;
    do_test("VUID-VkDescriptorBufferInfo-range-00341");

    // Now cause error due to range exceeding buffer size - offset
    buff_info.offset = 0;
    buff_info.range = buff_ci.size + 1;
    do_test("VUID-VkDescriptorBufferInfo-range-00342");

    if (update_template_support) {
        vkDestroyDescriptorUpdateTemplateKHR(m_device->device(), update_template, nullptr);
        if (push_descriptor_support) {
            vkDestroyDescriptorUpdateTemplateKHR(m_device->device(), push_template, nullptr);
        }
    }
}

TEST_F(VkLayerTest, DSBufferLimitErrors) {
    TEST_DESCRIPTION(
        "Attempt to update buffer descriptor set that has VkDescriptorBufferInfo values that violate device limits.\n"
        "Test cases include:\n"
        "1. range of uniform buffer update exceeds maxUniformBufferRange\n"
        "2. offset of uniform buffer update is not multiple of minUniformBufferOffsetAlignment\n"
        "3. using VK_WHOLE_SIZE with uniform buffer size exceeding maxUniformBufferRange\n"
        "4. range of storage buffer update exceeds maxStorageBufferRange\n"
        "5. offset of storage buffer update is not multiple of minStorageBufferOffsetAlignment\n"
        "6. using VK_WHOLE_SIZE with storage buffer size exceeding maxStorageBufferRange");

    ASSERT_NO_FATAL_FAILURE(Init());

    struct TestCase {
        VkDescriptorType descriptor_type;
        VkBufferUsageFlagBits buffer_usage;
        VkDeviceSize max_range;
        std::string max_range_vu;
        VkDeviceSize min_align;
        std::string min_align_vu;
    };

    for (const auto &test_case : {
             TestCase({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                       m_device->props.limits.maxUniformBufferRange, "VUID-VkWriteDescriptorSet-descriptorType-00332",
                       m_device->props.limits.minUniformBufferOffsetAlignment, "VUID-VkWriteDescriptorSet-descriptorType-00327"}),
             TestCase({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                       m_device->props.limits.maxStorageBufferRange, "VUID-VkWriteDescriptorSet-descriptorType-00333",
                       m_device->props.limits.minStorageBufferOffsetAlignment, "VUID-VkWriteDescriptorSet-descriptorType-00328"}),
         }) {
        // Create layout with single buffer
        OneOffDescriptorSet descriptor_set(m_device, {
                                                         {0, test_case.descriptor_type, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     });

        // Create a buffer to be used for invalid updates
        VkBufferCreateInfo bci = LvlInitStruct<VkBufferCreateInfo>();
        bci.usage = test_case.buffer_usage;
        bci.size = test_case.max_range + test_case.min_align;  // Make buffer bigger than range limit
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vk_testing::Buffer buffer;
        buffer.init_no_mem(*m_device, bci);
        if (buffer.handle() == VK_NULL_HANDLE) {
            std::string msg = std::string{kSkipPrefix} + " Failed to allocate buffer of size " + std::to_string(bci.size) +
                              " in DSBufferLimitErrors; skipped";
            printf("%s\n", msg.c_str());
            continue;
        }

        // Have to bind memory to buffer before descriptor update
        VkMemoryRequirements mem_reqs;
        vk::GetBufferMemoryRequirements(m_device->device(), buffer.handle(), &mem_reqs);

        VkMemoryAllocateInfo mem_alloc = LvlInitStruct<VkMemoryAllocateInfo>();
        mem_alloc.allocationSize = mem_reqs.size;
        bool pass = m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc, 0);
        if (!pass) {
            printf("%s Failed to allocate memory in DSBufferLimitErrors; skipped.\n", kSkipPrefix);
            continue;
        }

        vk_testing::DeviceMemory mem(*m_device, mem_alloc);
        if (mem.handle() == VK_NULL_HANDLE) {
            printf("%s Failed to allocate memory in DSBufferLimitErrors; skipped.\n", kSkipPrefix);
            continue;
        }
        VkResult err = vk::BindBufferMemory(m_device->device(), buffer.handle(), mem.handle(), 0);
        ASSERT_VK_SUCCESS(err);

        VkDescriptorBufferInfo buff_info = {};
        buff_info.buffer = buffer.handle();
        VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_write.dstBinding = 0;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pTexelBufferView = nullptr;
        descriptor_write.pBufferInfo = &buff_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.descriptorType = test_case.descriptor_type;
        descriptor_write.dstSet = descriptor_set.set_;

        // Exceed range limit
        if (test_case.max_range != UINT32_MAX) {
            buff_info.range = test_case.max_range + 1;
            buff_info.offset = 0;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.max_range_vu);
            vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
            m_errorMonitor->VerifyFound();
        }

        // Reduce size of range to acceptable limit and cause offset error
        if (test_case.min_align > 1) {
            buff_info.range = test_case.max_range;
            buff_info.offset = test_case.min_align - 1;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.min_align_vu);
            vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
            m_errorMonitor->VerifyFound();
        }

        // Exceed effective range limit by using VK_WHOLE_SIZE
        buff_info.range = VK_WHOLE_SIZE;
        buff_info.offset = 0;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, test_case.max_range_vu);
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DSAspectBitsErrors) {
    TEST_DESCRIPTION("Attempt to update descriptor sets for images that do not have correct aspect bits sets.");
    VkResult err;

    // Enable KHR multiplane req'd extensions
    AddOptionalExtensions(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    // Enable Sampler YCbCr Conversion
    auto ycbcr_features = LvlInitStruct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>();
    if (IsExtensionsEnabled(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME)) {
        auto vkGetPhysicalDeviceFeatures2FunctionKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
            vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR"));
        ASSERT_TRUE(vkGetPhysicalDeviceFeatures2FunctionKHR != nullptr);
        auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&ycbcr_features);
        vkGetPhysicalDeviceFeatures2FunctionKHR(gpu(), &features2);
        ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    } else {
        ASSERT_NO_FATAL_FAILURE(InitState());
    }

    auto depth_format = FindSupportedDepthStencilFormat(gpu());
    if (!depth_format) {
        printf("%s No Depth + Stencil format found. Skipped.\n", kSkipPrefix);
    } else {
        OneOffDescriptorSet descriptor_set(m_device,
                                           {
                                               {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           });

        if (descriptor_set.set_ == VK_NULL_HANDLE) {
            GTEST_SKIP() << "Couldn't create descriptor set";
        }

        // Create an image to be used for invalid updates
        VkImageObj image_obj(m_device);
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), depth_format, &format_props);
        if (!image_obj.IsCompatible(VK_IMAGE_USAGE_SAMPLED_BIT, format_props.optimalTilingFeatures)) {
            printf("%s Depth + Stencil format cannot be sampled with optimalTiling. Skipped.\n", kSkipPrefix);
        } else {
            image_obj.Init(64, 64, 1, depth_format, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL);
            ASSERT_TRUE(image_obj.initialized());
            VkImage image = image_obj.image();

            // Now create view for image
            VkImageViewCreateInfo image_view_ci = LvlInitStruct<VkImageViewCreateInfo>();
            image_view_ci.image = image;
            image_view_ci.format = depth_format;
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_ci.subresourceRange.layerCount = 1;
            image_view_ci.subresourceRange.baseArrayLayer = 0;
            image_view_ci.subresourceRange.levelCount = 1;
            // Setting both depth & stencil aspect bits is illegal for an imageView used
            // to populate a descriptor set.
            image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

            VkImageView image_view;
            err = vk::CreateImageView(m_device->device(), &image_view_ci, NULL, &image_view);
            ASSERT_VK_SUCCESS(err);
            descriptor_set.WriteDescriptorImageInfo(0, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

            const char *error_msg = "VUID-VkDescriptorImageInfo-imageView-01976";
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, error_msg);
            descriptor_set.UpdateDescriptorSets();
            m_errorMonitor->VerifyFound();
            vk::DestroyImageView(m_device->device(), image_view, NULL);
        }
    }

    if (!ycbcr_features.samplerYcbcrConversion) {
        printf("%s test requires KHR multiplane extensions, not available.  Skipping.\n", kSkipPrefix);
    } else {
        VkFormat mp_format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;  // commonly supported multi-planar format
        VkImageObj image_obj(m_device);
        VkFormatProperties format_props;
        vk::GetPhysicalDeviceFormatProperties(m_device->phy().handle(), mp_format, &format_props);
        if (!image_obj.IsCompatible(VK_IMAGE_USAGE_SAMPLED_BIT, format_props.optimalTilingFeatures)) {
            printf("%s multi-planar format cannot be sampled for optimalTiling. Skipped.\n", kSkipPrefix);
        } else {
            VkImageCreateInfo image_ci = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                          nullptr,
                                          VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,  // need for multi-planar
                                          VK_IMAGE_TYPE_2D,
                                          mp_format,
                                          {64, 64, 1},
                                          1,
                                          1,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_SAMPLED_BIT,
                                          VK_SHARING_MODE_EXCLUSIVE,
                                          0,
                                          nullptr,
                                          VK_IMAGE_LAYOUT_UNDEFINED};
            image_obj.init(&image_ci);
            ASSERT_TRUE(image_obj.initialized());

            auto vkCreateSamplerYcbcrConversionKHR = reinterpret_cast<PFN_vkCreateSamplerYcbcrConversionKHR>(
                vk::GetDeviceProcAddr(m_device->handle(), "vkCreateSamplerYcbcrConversionKHR"));
            auto vkDestroySamplerYcbcrConversionKHR = reinterpret_cast<PFN_vkDestroySamplerYcbcrConversionKHR>(
                vk::GetDeviceProcAddr(m_device->handle(), "vkDestroySamplerYcbcrConversionKHR"));
            ASSERT_NE(vkCreateSamplerYcbcrConversionKHR, nullptr);
            ASSERT_NE(vkDestroySamplerYcbcrConversionKHR, nullptr);

            auto ycbcr_create_info = LvlInitStruct<VkSamplerYcbcrConversionCreateInfo>();
            ycbcr_create_info.format = mp_format;
            ycbcr_create_info.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
            ycbcr_create_info.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
            ycbcr_create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
            ycbcr_create_info.xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
            ycbcr_create_info.yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
            ycbcr_create_info.chromaFilter = VK_FILTER_NEAREST;
            ycbcr_create_info.forceExplicitReconstruction = false;

            vk_testing::SamplerYcbcrConversion conversion(*m_device, ycbcr_create_info, true);

            VkSamplerYcbcrConversionInfo ycbcr_info = LvlInitStruct<VkSamplerYcbcrConversionInfo>();
            ycbcr_info.conversion = conversion.handle();

            auto image_view_ci = image_obj.TargetViewCI(mp_format);
            image_view_ci.pNext = &ycbcr_info;
            auto image_view = image_obj.targetView(image_view_ci);

            VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
            sampler_ci.pNext = &ycbcr_info;
            vk_testing::Sampler sampler(*m_device, sampler_ci);
            ASSERT_TRUE(sampler.initialized());

            OneOffDescriptorSet descriptor_set(
                m_device, {
                              {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, &sampler.handle()},
                          });

            if (descriptor_set.set_ == VK_NULL_HANDLE) {
                GTEST_SKIP() << "Couldn't create descriptor set";
            }

            // 01564 appears to be impossible to hit due to the following check in descriptor_validation.cpp:
            // if (sampler && !desc->IsImmutableSampler() && FormatIsMultiplane(image_state->createInfo.format)) ...
            //   - !desc->IsImmutableSampler() will cause 02738; IOW, multi-plane conversion _requires_ an immutable sampler
            //   - !desc->IsImmutableSampler() must be removed for 01564 to get hit, but it's not clear whether or not this is
            //   correct based on the comments in the code

            // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-sampler-01564");
            descriptor_set.WriteDescriptorImageInfo(0, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            descriptor_set.UpdateDescriptorSets();
            // m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, DSTypeMismatch) {
    // Create DS w/ layout of one type and attempt Update w/ mis-matched type
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, " binding #0 with type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER but update type is VK_DESCRIPTOR_TYPE_SAMPLER");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();

    m_errorMonitor->VerifyFound();

    vk::DestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, DSUpdateOutOfBounds) {
    // For overlapping Update, have arrayIndex exceed that of layout
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkBufferTest buffer_test(m_device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    if (!buffer_test.GetBufferCurrent()) {
        // Something prevented creation of buffer so abort
        printf("%s Buffer creation failed, skipping test\n", kSkipPrefix);
        return;
    }

    // Correctly update descriptor to avoid "NOT_UPDATED" error
    VkDescriptorBufferInfo buff_info = {};
    buff_info.buffer = buffer_test.GetBuffer();
    buff_info.offset = 0;
    buff_info.range = 1024;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 1; /* This index out of bounds for the update */
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buff_info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDSUpdateIndex) {
    // Create layout w/ count of 1 and attempt update to that layout w/ binding index 2
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstBinding-00315");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // This is the wrong type, but out of bounds will be flagged first
    descriptor_set.WriteDescriptorImageInfo(2, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vk::DestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, DSUpdateEmptyBinding) {
    // Create layout w/ empty binding and attempt to update it
    VkResult err;

    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 0 /* !! */, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // descriptor_write.descriptorCount = 1, Lie here to avoid parameter_validation error
    // This is the wrong type, but empty binding error will be flagged first
    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstBinding-00316");
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vk::DestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, InvalidDSUpdateStruct) {
    // Call UpdateDS w/ struct type other than valid VK_STRUCTUR_TYPE_UPDATE_*
    // types
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, ".sType must be VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET");

    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorImageInfo info = {};
    info.sampler = sampler;

    VkWriteDescriptorSet descriptor_write;
    memset(&descriptor_write, 0, sizeof(descriptor_write));
    descriptor_write.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; /* Intentionally broken struct type */
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.descriptorCount = 1;
    // This is the wrong type, but out of bounds will be flagged first
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptor_write.pImageInfo = &info;

    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    m_errorMonitor->VerifyFound();

    vk::DestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, SampleDescriptorUpdateError) {
    // Create a single Sampler descriptor and send it an invalid Sampler
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00325");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    VkSampler sampler = CastToHandle<VkSampler, uintptr_t>(0xbaadbeef);  // Sampler with invalid handle

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImageViewDescriptorUpdateError) {
    // Create a single combined Image/Sampler descriptor and send it an invalid
    // imageView
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02996");

    ASSERT_NO_FATAL_FAILURE(Init());
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       });

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    VkImageView view = CastToHandle<VkImageView, uintptr_t>(0xbaadbeef);  // invalid imageView object

    descriptor_set.WriteDescriptorImageInfo(0, view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    vk::DestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, CopyDescriptorUpdateErrors) {
    // Create DS w/ layout of 2 types, write update 1 and attempt to copy-update
    // into the other
    VkResult err;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " binding #1 with type VK_DESCRIPTOR_TYPE_SAMPLER. Types do not match.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler immutable_sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &immutable_sampler);
    ASSERT_VK_SUCCESS(err);

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    OneOffDescriptorSet descriptor_set_2(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, static_cast<VkSampler *>(&immutable_sampler)},
                  });

    VkSampler sampler;
    err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);

    // SAMPLER binding from layout above
    // This write update should succeed
    descriptor_set.WriteDescriptorImageInfo(1, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.UpdateDescriptorSets();
    // Now perform a copy update that fails due to type mismatch
    VkCopyDescriptorSet copy_ds_update = LvlInitStruct<VkCopyDescriptorSet>();
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;  // Copy from SAMPLER binding
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;       // ERROR : copy to UNIFORM binding
    copy_ds_update.descriptorCount = 1;  // copy 1 descriptor
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();
    // Now perform a copy update that fails due to binding out of bounds
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, " does not have copy update src binding of 3.");
    copy_ds_update = LvlInitStruct<VkCopyDescriptorSet>();
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 3;  // ERROR : Invalid binding for matching layout
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 1;  // Copy 1 descriptor
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();

    // Now perform a copy update that fails due to binding out of bounds
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         " binding#1 with offset index of 1 plus update array offset of 0 and update of 5 "
                                         "descriptors oversteps total number of descriptors in set: 2.");

    copy_ds_update = LvlInitStruct<VkCopyDescriptorSet>();
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;
    copy_ds_update.dstSet = descriptor_set.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 5;  // ERROR copy 5 descriptors (out of bounds for layout)
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);

    m_errorMonitor->VerifyFound();

    // Now perform a copy into an immutable sampler
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyDescriptorSet-dstBinding-02753");
    copy_ds_update.srcSet = descriptor_set.set_;
    copy_ds_update.srcBinding = 1;
    copy_ds_update.dstSet = descriptor_set_2.set_;
    copy_ds_update.dstBinding = 0;
    copy_ds_update.descriptorCount = 1;
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    vk::DestroySampler(m_device->device(), sampler, NULL);
    vk::DestroySampler(m_device->device(), immutable_sampler, NULL);
}

TEST_F(VkLayerTest, DrawWithPipelineIncompatibleWithRenderPass) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible cases. Initial case is drawing with an active renderpass that's not compatible with the bound "
        "pipeline state object's creation renderpass");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);  // We shouldn't need a fragment shader
    // but add it to be able to run on more devices
    // Create a renderpass that will be incompatible with default renderpass
    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    // Format incompatible with PSO RP color attach format B8G8R8A8_UNORM
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    VkRenderPass rp;
    vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {{0, 0}, {64, 64}};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);
    pipe.CreateVKPipeline(pipeline_layout.handle(), rp);

    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = rp;
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cbbi.pInheritanceInfo = &cbii;
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cbbi);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (the error should trigger on the attempt to draw).
    m_commandBuffer->Draw(3, 1, 0, 0);

    // Finalize recording of the command buffer
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();

    vk::DestroyRenderPass(m_device->device(), rp, NULL);
}

TEST_F(VkLayerTest, DrawWithPipelineIncompatibleWithRenderPassFragmentDensityMap) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible case: drawing with an active renderpass that's not compatible with the bound pipeline state "
        "object's creation renderpass since only the former uses a Fragment Density Map.");

    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) &&
        !IsExtensionsEnabled(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME << " AND " << VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME
                     << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);  // We shouldn't need a fragment shader
    // but add it to be able to run on more devices

    VkAttachmentDescription attach = {};
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkAttachmentReference ref = {};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

    VkRenderPassFragmentDensityMapCreateInfoEXT rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
    rpfdmi.fragmentDensityMapAttachment = ref;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpfdmi);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    // Create rp1 with FDM pNext and rp2 without FDM pNext
    vk_testing::RenderPass rp1(*m_device, rpci);
    ASSERT_TRUE(rp1.initialized());

    rpci.pNext = nullptr;
    rpci.attachmentCount = 1;
    vk_testing::RenderPass rp2(*m_device, rpci);
    ASSERT_TRUE(rp2.initialized());

    // Create image views
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageView iv = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    // Create a framebuffer with rp1
    VkFramebufferCreateInfo fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = rp1.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &iv;
    fbci.width = 128;
    fbci.height = 128;
    fbci.layers = 1;

    vk_testing::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    VkRenderPassBeginInfo rp_begin = LvlInitStruct<VkRenderPassBeginInfo>();
    rp_begin.renderPass = rp1.handle();
    rp_begin.framebuffer = fb.handle();
    rp_begin.renderArea = {{0, 0}, {128, 128}};

    // Create a graphics pipeline with rp2
    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {{0, 0}, {64, 64}};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);
    pipe.CreateVKPipeline(pipeline_layout.handle(), rp2.handle());

    // Begin renderpass and bind to pipeline
    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = rp1.handle();
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cbbi.pInheritanceInfo = &cbii;
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cbbi);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (the error should trigger on the attempt to draw).
    m_commandBuffer->Draw(3, 1, 0, 0);

    // Finalize recording of the command buffer
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DrawWithPipelineIncompatibleWithRenderPassMultiview) {
    TEST_DESCRIPTION(
        "Hit RenderPass incompatible cases: drawing with an active renderpass that's not compatible with the bound pipeline state "
        "object's creation renderpass since only the former uses Multiview.");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&multiview_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (!features2.features.multiViewport) {
        printf("%s multiViewport feature is not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    if (!multiview_features.multiview) {
        printf("%s multiview feature is not supported, skipping test.\n", kSkipPrefix);
        return;
    }

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        } else {
            printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);  // We shouldn't need a fragment shader
    // but add it to be able to run on more devices

    // Set up VkRenderPassCreateInfo struct used with VK_VERSION_1_0
    VkAttachmentReference color_att = {};
    color_att.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription attach = {};
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_att;

    uint32_t viewMasks[] = {0x3u};
    VkRenderPassMultiviewCreateInfo rpmvci = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    rpmvci.subpassCount = 1;
    rpmvci.pViewMasks = viewMasks;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpmvci);
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;

    // Set up VkRenderPassCreateInfo2 struct used with VK_VERSION_1_2
    VkAttachmentReference2 color_att2 = LvlInitStruct<VkAttachmentReference2>();
    color_att2.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentDescription2 attach2 = LvlInitStruct<VkAttachmentDescription2>();
    attach2.samples = VK_SAMPLE_COUNT_1_BIT;
    attach2.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach2.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach2.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass2 = LvlInitStruct<VkSubpassDescription2>();
    subpass2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass2.viewMask = 0x3u;
    subpass2.colorAttachmentCount = 1;
    subpass2.pColorAttachments = &color_att2;

    VkRenderPassCreateInfo2 rpci2 = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci2.attachmentCount = 1;
    rpci2.pAttachments = &attach2;
    rpci2.subpassCount = 1;
    rpci2.pSubpasses = &subpass2;

    // Create render passes with VK_VERSION_1_0 struct and vkCreateRenderPass call
    // Create rp[0] with Multiview pNext, rp[1] without Multiview pNext, rp[2] with Multiview pNext but another viewMask
    VkRenderPass rp[3];
    vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp[0]);
    rpci.pNext = nullptr;
    vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp[1]);
    uint32_t viewMasks2[] = {0x1u};
    rpmvci.pViewMasks = viewMasks2;
    rpci.pNext = &rpmvci;
    vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp[2]);

    // Create render passes with VK_VERSION_1_2 struct and vkCreateRenderPass2KHR call
    // Create rp2[0] with Multiview, rp2[1] without Multiview (zero viewMask), rp2[2] with Multiview but another viewMask
    VkRenderPass rp2[3];
    if (rp2Supported) {
        PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
            (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");
        vkCreateRenderPass2KHR(m_device->device(), &rpci2, nullptr, &rp2[0]);
        subpass2.viewMask = 0x0u;
        rpci2.pSubpasses = &subpass2;
        vkCreateRenderPass2KHR(m_device->device(), &rpci2, nullptr, &rp2[1]);
        subpass2.viewMask = 0x1u;
        rpci2.pSubpasses = &subpass2;
        vkCreateRenderPass2KHR(m_device->device(), &rpci2, nullptr, &rp2[2]);
    }

    // Create image view
    VkImageObj image(m_device);
    auto ici2d = image.ImageCreateInfo2D(128, 128, 1, 2, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                         VK_IMAGE_TILING_OPTIMAL, 0);
    image.Init(ici2d);
    ASSERT_TRUE(image.initialized());

    VkImageView iv;
    VkImageViewCreateInfo ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    ivci.format = VK_FORMAT_B8G8R8A8_UNORM;
    ivci.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY};
    ivci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2};
    vk::CreateImageView(m_device->device(), &ivci, NULL, &iv);

    // Create framebuffers for rp[0] and rp2[0]
    VkFramebufferCreateInfo fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = rp[0];
    fbci.attachmentCount = 1;
    fbci.pAttachments = &iv;
    fbci.width = 128;
    fbci.height = 128;
    fbci.layers = 1;

    VkFramebuffer fb = VK_NULL_HANDLE;
    VkFramebuffer fb2 = VK_NULL_HANDLE;
    vk::CreateFramebuffer(m_device->handle(), &fbci, nullptr, &fb);
    if (rp2Supported) {
        fbci.renderPass = rp2[0];
        vk::CreateFramebuffer(m_device->handle(), &fbci, nullptr, &fb2);
    }

    VkRenderPassBeginInfo rp_begin = LvlInitStruct<VkRenderPassBeginInfo>();
    rp_begin.renderPass = rp[0];
    rp_begin.framebuffer = fb;
    rp_begin.renderArea = {{0, 0}, {128, 128}};

    // Create a graphics pipeline with rp[1]
    VkPipelineObj pipe_1(m_device);
    pipe_1.AddShader(&vs);
    pipe_1.AddShader(&fs);
    pipe_1.AddDefaultColorAttachment();
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe_1.SetViewport(m_viewports);
    VkRect2D rect = {{0, 0}, {64, 64}};
    m_scissors.push_back(rect);
    pipe_1.SetScissor(m_scissors);
    pipe_1.CreateVKPipeline(pipeline_layout.handle(), rp[1]);

    // Create a graphics pipeline with rp[2]
    VkPipelineObj pipe_2(m_device);
    pipe_2.AddShader(&vs);
    pipe_2.AddShader(&fs);
    pipe_2.AddDefaultColorAttachment();
    m_viewports.push_back(viewport);
    pipe_2.SetViewport(m_viewports);
    m_scissors.push_back(rect);
    pipe_2.SetScissor(m_scissors);
    pipe_2.CreateVKPipeline(pipeline_layout.handle(), rp[2]);

    VkPipelineObj pipe2_1(m_device);
    VkPipelineObj pipe2_2(m_device);
    if (rp2Supported) {
        // Create a graphics pipeline with rp2[1]
        pipe2_1.AddShader(&vs);
        pipe2_1.AddShader(&fs);
        pipe2_1.AddDefaultColorAttachment();
        m_viewports.push_back(viewport);
        pipe2_1.SetViewport(m_viewports);
        m_scissors.push_back(rect);
        pipe2_1.SetScissor(m_scissors);
        pipe2_1.CreateVKPipeline(pipeline_layout.handle(), rp2[1]);

        // Create a graphics pipeline with rp2[2]
        pipe2_2.AddShader(&vs);
        pipe2_2.AddShader(&fs);
        pipe2_2.AddDefaultColorAttachment();
        m_viewports.push_back(viewport);
        pipe2_2.SetViewport(m_viewports);
        m_scissors.push_back(rect);
        pipe2_2.SetScissor(m_scissors);
        pipe2_2.CreateVKPipeline(pipeline_layout.handle(), rp2[2]);
    }

    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = rp[0];
    cbii.subpass = 0;
    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    cbbi.pInheritanceInfo = &cbii;

    // Begin rp[0] for VK_VERSION_1_0 test cases
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cbbi);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    // Bind rp[1]'s pipeline to command buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_1.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (error on Multiview usage should trigger on draw)
    m_commandBuffer->Draw(3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // Bind rp[2]'s pipeline to command buffer
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_2.handle());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
    // Render triangle (error on non-matching viewMasks for Multiview usage should trigger on draw)
    m_commandBuffer->Draw(3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    // End rp[0]
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Begin rp2[0] for VK_VERSION_1_2 test cases
    if (rp2Supported) {
        cbii.renderPass = rp2[0];
        rp_begin.renderPass = rp2[0];
        rp_begin.framebuffer = fb2;
        vk::BeginCommandBuffer(m_commandBuffer->handle(), &cbbi);
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

        // Bind rp2[1]'s pipeline to command buffer
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2_1.handle());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
        // Render triangle (error on Multiview usage should trigger on draw)
        m_commandBuffer->Draw(3, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // Bind rp2[2]'s pipeline to command buffer
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2_2.handle());

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-renderPass-02684");
        // Render triangle (error on non-matching viewMasks for Multiview usage should trigger on draw)
        m_commandBuffer->Draw(3, 1, 0, 0);
        m_errorMonitor->VerifyFound();

        // End rp2[0]
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    for (int i = 0; i < 3; i++) {
        vk::DestroyRenderPass(m_device->device(), rp[i], nullptr);
        if (rp2Supported) {
            vk::DestroyRenderPass(m_device->device(), rp2[i], nullptr);
        }
    }
    vk::DestroyFramebuffer(m_device->device(), fb, nullptr);
    if (rp2Supported) {
        vk::DestroyFramebuffer(m_device->device(), fb2, nullptr);
    }
    vk::DestroyImageView(m_device->device(), iv, nullptr);
}

TEST_F(VkLayerTest, Maint1BindingSliceOf3DImage) {
    TEST_DESCRIPTION(
        "Attempt to bind a slice of a 3D texture in a descriptor set. This is explicitly disallowed by KHR_maintenance1 to keep "
        "things simple for drivers.");
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    } else {
        printf("%s %s is not supported; skipping\n", kSkipPrefix, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkResult err;

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });

    VkImageCreateInfo ici = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                             nullptr,
                             VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR,
                             VK_IMAGE_TYPE_3D,
                             VK_FORMAT_R8G8B8A8_UNORM,
                             {32, 32, 32},
                             1,
                             1,
                             VK_SAMPLE_COUNT_1_BIT,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_SHARING_MODE_EXCLUSIVE,
                             0,
                             nullptr,
                             VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageObj image(m_device);
    image.init(&ici);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkImageView view;
    err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    // Meat of the test.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorImageInfo-imageView-06711");

    VkDescriptorImageInfo dii = {VK_NULL_HANDLE, view, VK_IMAGE_LAYOUT_GENERAL};
    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                  nullptr,
                                  descriptor_set.set_,
                                  0,
                                  0,
                                  1,
                                  VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                  &dii,
                                  nullptr,
                                  nullptr};
    vk::UpdateDescriptorSets(m_device->device(), 1, &write, 0, nullptr);

    m_errorMonitor->VerifyFound();

    vk::DestroyImageView(m_device->device(), view, nullptr);
}

TEST_F(VkLayerTest, UpdateDestroyDescriptorSetLayout) {
    TEST_DESCRIPTION("Attempt updates to descriptor sets with destroyed descriptor set layouts");
    // TODO: Update to match the descriptor set layout specific VUIDs/VALIDATION_ERROR_* when present
    const auto kWriteDestroyedLayout = "VUID-VkWriteDescriptorSet-dstSet-00320";
    const auto kCopyDstDestroyedLayout = "VUID-VkCopyDescriptorSet-dstSet-parameter";
    const auto kCopySrcDestroyedLayout = "VUID-VkCopyDescriptorSet-srcSet-parameter";

    ASSERT_NO_FATAL_FAILURE(Init());

    // Set up the descriptor (resource) and write/copy operations to use.
    float data[16] = {};
    VkConstantBufferObj buffer(m_device, sizeof(data), data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    ASSERT_TRUE(buffer.initialized());

    VkDescriptorBufferInfo info = {};
    info.buffer = buffer.handle();
    info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write_descriptor = LvlInitStruct<VkWriteDescriptorSet>();
    write_descriptor.dstSet = VK_NULL_HANDLE;  // must update this
    write_descriptor.dstBinding = 0;
    write_descriptor.descriptorCount = 1;
    write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor.pBufferInfo = &info;

    VkCopyDescriptorSet copy_descriptor = LvlInitStruct<VkCopyDescriptorSet>();
    copy_descriptor.srcSet = VK_NULL_HANDLE;  // must update
    copy_descriptor.srcBinding = 0;
    copy_descriptor.dstSet = VK_NULL_HANDLE;  // must update
    copy_descriptor.dstBinding = 0;
    copy_descriptor.descriptorCount = 1;

    // Create valid and invalid source and destination descriptor sets
    std::vector<VkDescriptorSetLayoutBinding> one_uniform_buffer = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    OneOffDescriptorSet good_dst(m_device, one_uniform_buffer);
    ASSERT_TRUE(good_dst.Initialized());

    OneOffDescriptorSet bad_dst(m_device, one_uniform_buffer);
    // Must assert before invalidating it below
    ASSERT_TRUE(bad_dst.Initialized());
    bad_dst.layout_ = VkDescriptorSetLayoutObj();

    OneOffDescriptorSet good_src(m_device, one_uniform_buffer);
    ASSERT_TRUE(good_src.Initialized());

    // Put valid data in the good and bad sources, simultaneously doing a positive test on write and copy operations
    write_descriptor.dstSet = good_src.set_;
    vk::UpdateDescriptorSets(m_device->device(), 1, &write_descriptor, 0, NULL);

    OneOffDescriptorSet bad_src(m_device, one_uniform_buffer);
    ASSERT_TRUE(bad_src.Initialized());

    // to complete our positive testing use copy, where above we used write.
    copy_descriptor.srcSet = good_src.set_;
    copy_descriptor.dstSet = bad_src.set_;
    vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_descriptor);
    bad_src.layout_ = VkDescriptorSetLayoutObj();

    // Trigger the three invalid use errors
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kWriteDestroyedLayout);
    write_descriptor.dstSet = bad_dst.set_;
    vk::UpdateDescriptorSets(m_device->device(), 1, &write_descriptor, 0, NULL);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kCopyDstDestroyedLayout);
    copy_descriptor.dstSet = bad_dst.set_;
    vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_descriptor);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, kCopySrcDestroyedLayout);
    copy_descriptor.srcSet = bad_src.set_;
    copy_descriptor.dstSet = good_dst.set_;
    vk::UpdateDescriptorSets(m_device->device(), 0, nullptr, 1, &copy_descriptor);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, FramebufferIncompatible) {
    TEST_DESCRIPTION(
        "Bind a secondary command buffer with a framebuffer that does not match the framebuffer for the active renderpass.");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // A renderpass with one color attachment.
    VkAttachmentDescription attachment = {0,
                                          VK_FORMAT_B8G8R8A8_UNORM,
                                          VK_SAMPLE_COUNT_1_BIT,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_STORE,
                                          VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                          VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                          VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &att_ref, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, &attachment, 1, &subpass, 0, nullptr};

    VkRenderPass rp;
    VkResult err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &rp);
    ASSERT_VK_SUCCESS(err);

    // A compatible framebuffer.
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    ASSERT_TRUE(image.initialized());

    VkImageViewCreateInfo ivci = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        image.handle(),
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_B8G8R8A8_UNORM,
        {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkImageView view;
    err = vk::CreateImageView(m_device->device(), &ivci, nullptr, &view);
    ASSERT_VK_SUCCESS(err);

    VkFramebufferCreateInfo fci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp, 1, &view, 32, 32, 1};
    VkFramebuffer fb;
    err = vk::CreateFramebuffer(m_device->device(), &fci, nullptr, &fb);
    ASSERT_VK_SUCCESS(err);

    VkCommandBufferAllocateInfo cbai = LvlInitStruct<VkCommandBufferAllocateInfo>();
    cbai.commandPool = m_commandPool->handle();
    cbai.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    cbai.commandBufferCount = 1;

    VkCommandBuffer sec_cb;
    err = vk::AllocateCommandBuffers(m_device->device(), &cbai, &sec_cb);
    ASSERT_VK_SUCCESS(err);
    VkCommandBufferBeginInfo cbbi = LvlInitStruct<VkCommandBufferBeginInfo>();
    VkCommandBufferInheritanceInfo cbii = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    cbii.renderPass = renderPass();
    cbii.framebuffer = fb;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    cbbi.pInheritanceInfo = &cbii;
    vk::BeginCommandBuffer(sec_cb, &cbbi);
    vk::EndCommandBuffer(sec_cb);

    VkCommandBufferBeginInfo cbbi2 = LvlInitStruct<VkCommandBufferBeginInfo>();
    vk::BeginCommandBuffer(m_commandBuffer->handle(), &cbbi2);
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-pCommandBuffers-00099");
    vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &sec_cb);
    m_errorMonitor->VerifyFound();
    // Cleanup

    vk::CmdEndRenderPass(m_commandBuffer->handle());
    vk::EndCommandBuffer(m_commandBuffer->handle());

    vk::DestroyImageView(m_device->device(), view, NULL);
    vk::DestroyRenderPass(m_device->device(), rp, NULL);
    vk::DestroyFramebuffer(m_device->device(), fb, NULL);
}

TEST_F(VkLayerTest, RenderPassMissingAttachment) {
    TEST_DESCRIPTION("Begin render pass with missing framebuffer attachment");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Create a renderPass with a single color attachment
    VkAttachmentReference attach = {};
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pColorAttachments = &attach;
    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    vk_testing::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    auto createView = LvlInitStruct<VkImageViewCreateInfo>();
    createView.image = m_renderTargets[0]->handle();
    createView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createView.format = VK_FORMAT_B8G8R8A8_UNORM;
    createView.components.r = VK_COMPONENT_SWIZZLE_R;
    createView.components.g = VK_COMPONENT_SWIZZLE_G;
    createView.components.b = VK_COMPONENT_SWIZZLE_B;
    createView.components.a = VK_COMPONENT_SWIZZLE_A;
    createView.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    createView.flags = 0;

    VkImageView iv;
    vk::CreateImageView(m_device->handle(), &createView, nullptr, &iv);

    auto fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &iv;
    fb_info.width = 100;
    fb_info.height = 100;
    fb_info.layers = 1;

    // Create the framebuffer then destory the view it uses.
    vk_testing::Framebuffer fb(*m_device, fb_info);
    ASSERT_TRUE(fb.initialized());
    vk::DestroyImageView(device(), iv, NULL);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-framebuffer-parameter");

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.renderPass = rp.handle();
    rpbi.framebuffer = fb.handle();
    rpbi.renderArea = {{0, 0}, {32, 32}};

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    // Don't call vk::CmdEndRenderPass; as the begin has been "skipped" based on the error condition
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, AttachmentDescriptionUndefinedFormat) {
    TEST_DESCRIPTION("Create a render pass with an attachment description format set to VK_FORMAT_UNDEFINED");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription-format-06698");

    VkAttachmentReference color_attach = {};
    color_attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attach.attachment = 0;
    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attach;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_UNDEFINED;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    rpci.pAttachments = &attach_desc;
    VkRenderPass rp;
    VkResult result = vk::CreateRenderPass(m_device->device(), &rpci, NULL, &rp);

    m_errorMonitor->VerifyFound();

    if (result == VK_SUCCESS) {
        vk::DestroyRenderPass(m_device->device(), rp, nullptr);
    }
}

TEST_F(VkLayerTest, InvalidCreateDescriptorPool) {
    TEST_DESCRIPTION("Attempt to create descriptor pool with invalid parameters");

    ASSERT_NO_FATAL_FAILURE(Init());

    const uint32_t default_descriptor_count = 1;
    const VkDescriptorPoolSize dp_size_template{VK_DESCRIPTOR_TYPE_SAMPLER, default_descriptor_count};

    const VkDescriptorPoolCreateInfo dp_ci_template{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                                                    nullptr,  // pNext
                                                    0,        // flags
                                                    1,        // maxSets
                                                    1,        // poolSizeCount
                                                    &dp_size_template};

    // try maxSets = 0
    {
        VkDescriptorPoolCreateInfo invalid_dp_ci = dp_ci_template;
        invalid_dp_ci.maxSets = 0;  // invalid maxSets value

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolCreateInfo-maxSets-00301");
        {
            VkDescriptorPool pool;
            vk::CreateDescriptorPool(m_device->device(), &invalid_dp_ci, nullptr, &pool);
        }
        m_errorMonitor->VerifyFound();
    }

    // try descriptorCount = 0
    {
        VkDescriptorPoolSize invalid_dp_size = dp_size_template;
        invalid_dp_size.descriptorCount = 0;  // invalid descriptorCount value

        VkDescriptorPoolCreateInfo dp_ci = dp_ci_template;
        dp_ci.pPoolSizes = &invalid_dp_size;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolSize-descriptorCount-00302");
        {
            VkDescriptorPool pool;
            vk::CreateDescriptorPool(m_device->device(), &dp_ci, nullptr, &pool);
        }
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, DuplicateDescriptorBinding) {
    TEST_DESCRIPTION("Create a descriptor set layout with a duplicate binding number.");

    ASSERT_NO_FATAL_FAILURE(Init());
    // Create layout where two binding #s are "1"
    static const uint32_t NUM_BINDINGS = 3;
    VkDescriptorSetLayoutBinding dsl_binding[NUM_BINDINGS] = {};
    dsl_binding[0].binding = 1;
    dsl_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[0].descriptorCount = 1;
    dsl_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[0].pImmutableSamplers = NULL;
    dsl_binding[1].binding = 0;
    dsl_binding[1].descriptorCount = 1;
    dsl_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[1].descriptorCount = 1;
    dsl_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[1].pImmutableSamplers = NULL;
    dsl_binding[2].binding = 1;  // Duplicate binding should cause error
    dsl_binding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding[2].descriptorCount = 1;
    dsl_binding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding[2].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.bindingCount = NUM_BINDINGS;
    ds_layout_ci.pBindings = dsl_binding;
    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-binding-00279");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidPushDescriptorSetLayout) {
    TEST_DESCRIPTION("Create a push descriptor set layout with invalid bindings.");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    // Get the push descriptor limits
    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    // Note that as binding is referenced in ds_layout_ci, it is effectively in the closure by reference as well.
    auto test_create_ds_layout = [&ds_layout_ci, this](std::string error) {
        VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, error);
        vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
        m_errorMonitor->VerifyFound();
        vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
    };

    // Starting with the initial VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC type set above..
    test_create_ds_layout("VUID-VkDescriptorSetLayoutCreateInfo-flags-00280");

    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    test_create_ds_layout(
        "VUID-VkDescriptorSetLayoutCreateInfo-flags-00280");  // This is the same VUID as above, just a second error condition.

    if (!(push_descriptor_prop.maxPushDescriptors == std::numeric_limits<uint32_t>::max())) {
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.descriptorCount = push_descriptor_prop.maxPushDescriptors + 1;
        test_create_ds_layout("VUID-VkDescriptorSetLayoutCreateInfo-flags-00281");
    } else {
        printf("%s maxPushDescriptors is set to maximum unit32_t value, skipping 'out of range test'.\n", kSkipPrefix);
    }
}

TEST_F(VkLayerTest, InvalidPushDescriptorImageLayout) {
    TEST_DESCRIPTION("Use a push descriptor with a mismatched image layout.");

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout(m_device, {dsl_binding}, VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    auto pipeline_layout = VkPipelineLayoutObj(m_device, {&ds_layout});

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D tex;
        layout(location=0) out vec4 color;
        void main(){
           color = textureLod(tex, vec2(0.5, 0.5), 0.0);
        }
    )glsl";
    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.MakeDynamic(VK_DYNAMIC_STATE_VIEWPORT);
    pipe.MakeDynamic(VK_DYNAMIC_STATE_SCISSOR);
    pipe.CreateVKPipeline(pipeline_layout.handle(), m_renderPass);
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;

    auto err = vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    ASSERT_VK_SUCCESS(err);
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    VkImageView image_view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);
    image.SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);

    VkDescriptorImageInfo img_info = {};
    img_info.sampler = sampler;
    img_info.imageView = image_view;
    img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &img_info;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.dstBinding = 0;

    auto vkCmdPushDescriptorSetKHR =
        (PFN_vkCmdPushDescriptorSetKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCmdPushDescriptorSetKHR");

    for (uint32_t i = 0; i < 2; i++) {
        m_commandBuffer->begin();
        if (i == 1) {
            // Test path where image layout in command buffer is known at draw time
            image.ImageMemoryBarrier(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_SHADER_WRITE_BIT,
                                     VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        }
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vkCmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                                  &descriptor_write);
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &m_viewports[0]);
        vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &m_scissors[0]);

        if (i == 1) {
            // Test path where image layout in command buffer is known at draw time
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "Image layout specified at vkUpdateDescriptorSet* or vkCmdPushDescriptorSet* "
                                                 "time doesn't match actual image layout at time descriptor is used");
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "with specific layout VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL that doesn't match "
                                                 "the previous known layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL");
            m_commandBuffer->Draw(1, 1, 0, 0);
            m_errorMonitor->VerifyFound();
            break;
        }
        m_errorMonitor->SetDesiredFailureMsg(
            kErrorBit,
            "to be in layout VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL--instead, current layout is VK_IMAGE_LAYOUT_GENERAL.");
        m_commandBuffer->Draw(1, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        m_commandBuffer->QueueCommandBuffer(false);
        m_errorMonitor->VerifyFound();
    }

    vk::DestroySampler(m_device->device(), sampler, NULL);
}

TEST_F(VkLayerTest, PushDescriptorSetLayoutWithoutExtension) {
    TEST_DESCRIPTION("Create a push descriptor set layout without loading the needed extension.");
    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    std::string error = "Attempted to use VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR in ";
    error = error + "VkDescriptorSetLayoutCreateInfo::flags but its required extension ";
    error = error + VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME;
    error = error + " has not been enabled.";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, error.c_str());
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-00281");
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
}

TEST_F(VkLayerTest, DescriptorIndexingSetLayoutWithoutExtension) {
    TEST_DESCRIPTION("Create an update_after_bind set layout without loading the needed extension.");
    ASSERT_NO_FATAL_FAILURE(Init());

    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

    std::string error = "Attemped to use VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT in ";
    error = error + "VkDescriptorSetLayoutCreateInfo::flags but its required extension ";
    error = error + VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME;
    error = error + " has not been enabled.";

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, error.c_str());
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
}

TEST_F(VkLayerTest, DescriptorIndexingSetLayout) {
    TEST_DESCRIPTION("Exercise various create/allocate-time errors related to VK_EXT_descriptor_indexing.");

    if (!(CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names, m_device_extension_names, NULL,
                                                         m_errorMonitor))) {
        printf("%s Descriptor indexing or one of its dependencies not supported, skipping tests.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables all supported indexing features except descriptorBindingUniformBufferUpdateAfterBind
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    std::array<VkDescriptorBindingFlagsEXT, 2> flags = {
        {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT}};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = (uint32_t)flags.size();
    flags_create_info.pBindingFlags = flags.data();

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;

    // VU for VkDescriptorSetLayoutBindingFlagsCreateInfoEXT::bindingCount
    flags_create_info.bindingCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-bindingCount-03002");
    VkResult err = vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);

    flags_create_info.bindingCount = 1;

    // set is missing UPDATE_AFTER_BIND_POOL flag.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-03000");
    // binding uses a feature we disabled
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingUniformBufferUpdateAfterBind-03005");
    err = vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);

    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    ds_layout_ci.bindingCount = 0;
    flags_create_info.bindingCount = 0;
    err = vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
    auto dspci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_size;
    dspci.maxSets = 1;
    VkDescriptorPool pool;
    err = vk::CreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);

    auto ds_alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout;

    VkDescriptorSet ds = VK_NULL_HANDLE;
    // mismatch between descriptor set and pool
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-03044");
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    m_errorMonitor->VerifyFound();

    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
    vk::DestroyDescriptorPool(m_device->handle(), pool, nullptr);

    if (indexing_features.descriptorBindingVariableDescriptorCount) {
        ds_layout_ci.flags = 0;
        ds_layout_ci.bindingCount = 1;
        flags_create_info.bindingCount = 1;
        flags[0] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
        err = vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
        ASSERT_VK_SUCCESS(err);

        pool_size = {binding.descriptorType, 3};
        dspci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
        dspci.poolSizeCount = 1;
        dspci.pPoolSizes = &pool_size;
        dspci.maxSets = 2;
        err = vk::CreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
        ASSERT_VK_SUCCESS(err);

        auto count_alloc_info = LvlInitStruct<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT>();
        count_alloc_info.descriptorSetCount = 1;
        // Set variable count larger than what was in the descriptor binding
        uint32_t variable_count = 2;
        count_alloc_info.pDescriptorCounts = &variable_count;

        ds_alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>(&count_alloc_info);
        ds_alloc_info.descriptorPool = pool;
        ds_alloc_info.descriptorSetCount = 1;
        ds_alloc_info.pSetLayouts = &ds_layout;

        ds = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkDescriptorSetVariableDescriptorCountAllocateInfo-pSetLayouts-03046");
        vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
        m_errorMonitor->VerifyFound();

        vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);

        // Now update descriptor set with a size that falls within the descriptor set layout size but that is more than the
        // descriptor set size
        binding.descriptorCount = 3;
        err = vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
        ASSERT_VK_SUCCESS(err);
        vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
        VkBufferObj buffer;
        VkMemoryPropertyFlags reqs = 0;
        buffer.init_as_dst(*m_device, 128 * 128, reqs);
        VkDescriptorBufferInfo buffer_info[3] = {};
        for (int i = 0; i < 3; i++) {
            buffer_info[i].buffer = buffer.handle();
            buffer_info[i].offset = 0;
            buffer_info[i].range = 128 * 128;
        }
        VkWriteDescriptorSet descriptor_writes[1] = {};
        descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
        descriptor_writes[0].dstSet = ds;
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].descriptorCount = 3;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].pBufferInfo = buffer_info;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
        vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);
        m_errorMonitor->VerifyFound();
        vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
        vk::DestroyDescriptorPool(m_device->handle(), pool, nullptr);
    }
}

TEST_F(VkLayerTest, DescriptorIndexingUpdateAfterBind) {
    TEST_DESCRIPTION("Exercise errors for updating a descriptor set after it is bound.");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    // Create a device that enables all supported indexing features except descriptorBindingUniformBufferUpdateAfterBind
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    indexing_features.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;

    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        printf("%s Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind, skipping\n", kSkipPrefix);
        return;
    }
    if (VK_FALSE == features2.features.fragmentStoresAndAtomics) {
        printf("%s Test requires (unsupported) fragmentStoresAndAtomics, skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorBindingFlagsEXT flags[3] = {0, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                                            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 3;
    flags_create_info.pBindingFlags = &flags[0];

    // Descriptor set has two bindings - only the second is update_after_bind
    VkDescriptorSetLayoutBinding binding[3] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    ds_layout_ci.bindingCount = 3;
    ds_layout_ci.pBindings = &binding[0];
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;

    VkResult err = vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);

    VkDescriptorPoolSize pool_sizes[3] = {
        {binding[0].descriptorType, binding[0].descriptorCount},
        {binding[1].descriptorType, binding[1].descriptorCount},
        {binding[2].descriptorType, binding[2].descriptorCount},
    };
    auto dspci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    dspci.poolSizeCount = 3;
    dspci.pPoolSizes = &pool_sizes[0];
    dspci.maxSets = 1;
    VkDescriptorPool pool;
    err = vk::CreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);

    auto ds_alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout;

    VkDescriptorSet ds = VK_NULL_HANDLE;
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    ASSERT_VK_SUCCESS(err);

    VkBufferCreateInfo buffCI = LvlInitStruct<VkBufferCreateInfo>();
    buffCI.size = 1024;
    buffCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBuffer dynamic_uniform_buffer;
    err = vk::CreateBuffer(m_device->device(), &buffCI, NULL, &dynamic_uniform_buffer);
    ASSERT_VK_SUCCESS(err);

    VkDeviceMemory mem;
    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(m_device->device(), dynamic_uniform_buffer, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc_info = LvlInitStruct<VkMemoryAllocateInfo>();
    mem_alloc_info.allocationSize = mem_reqs.size;
    m_device->phy().set_memory_type(mem_reqs.memoryTypeBits, &mem_alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    err = vk::AllocateMemory(m_device->device(), &mem_alloc_info, NULL, &mem);
    ASSERT_VK_SUCCESS(err);

    err = vk::BindBufferMemory(m_device->device(), dynamic_uniform_buffer, mem, 0);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorBufferInfo buffInfo[2] = {};
    buffInfo[0].buffer = dynamic_uniform_buffer;
    buffInfo[0].offset = 0;
    buffInfo[0].range = 1024;

    VkWriteDescriptorSet descriptor_write[2] = {};
    descriptor_write[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write[0].dstSet = ds;
    descriptor_write[0].dstBinding = 0;
    descriptor_write[0].descriptorCount = 1;
    descriptor_write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write[0].pBufferInfo = buffInfo;
    descriptor_write[1] = descriptor_write[0];
    descriptor_write[1].dstBinding = 1;
    descriptor_write[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;

    vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);

    // Create a dummy pipeline, since VL inspects which bindings are actually used at draw time
    char const *fsSource = R"glsl(
        #version 450
        layout(location=0) out vec4 color;
        layout(set=0, binding=0) uniform foo0 { float x0; } bar0;
        layout(set=0, binding=1) buffer  foo1 { float x1; } bar1;
        layout(set=0, binding=2) buffer  foo2 { float x2; } bar2;
        void main(){
           color = vec4(bar0.x0 + bar1.x1 + bar2.x2);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe(m_device);
    pipe.SetViewport(m_viewports);
    pipe.SetScissor(m_scissors);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.CreateVKPipeline(pipeline_layout, m_renderPass);

    // Make both bindings valid before binding to the command buffer
    vk::UpdateDescriptorSets(m_device->device(), 2, &descriptor_write[0], 0, NULL);

    VkSubmitInfo submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    // Two subtests. First only updates the update_after_bind binding and expects
    // no error. Second updates the other binding and expects an error when the
    // command buffer is ended.
    for (uint32_t i = 0; i < 2; ++i) {
        m_commandBuffer->begin();

        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &ds, 0, NULL);

        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
        vk::CmdDraw(m_commandBuffer->handle(), 0, 0, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());

        // Valid to update binding 1 after being bound
        vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write[1], 0, NULL);

        if (i == 0) {
            // expect no errors
            m_commandBuffer->end();
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02699");
            vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
            m_errorMonitor->VerifyFound();
            vk::QueueWaitIdle(m_device->m_queue);
        } else {
            // Invalid to update binding 0 after being bound. But the error is actually
            // generated during vk::EndCommandBuffer
            vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write[0], 0, NULL);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer-VkDescriptorSet");

            vk::EndCommandBuffer(m_commandBuffer->handle());
            m_errorMonitor->VerifyFound();
        }
    }

    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
    vk::DestroyDescriptorPool(m_device->handle(), pool, nullptr);
    vk::DestroyBuffer(m_device->handle(), dynamic_uniform_buffer, NULL);
    vk::FreeMemory(m_device->handle(), mem, NULL);
    vk::DestroyPipelineLayout(m_device->handle(), pipeline_layout, NULL);
}

TEST_F(VkLayerTest, DescriptorIndexingSetNonIdenticalWrite) {
    TEST_DESCRIPTION("VkWriteDescriptorSet must have identical VkDescriptorBindingFlagBits");

    if (!(CheckDescriptorIndexingSupportAndInitFramework(this, m_instance_extension_names, m_device_extension_names, NULL,
                                                         m_errorMonitor))) {
        printf("%s Descriptor indexing or one of its dependencies not supported, skipping tests.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (VK_FALSE == indexing_features.descriptorBindingStorageBufferUpdateAfterBind) {
        printf("%s Test requires (unsupported) descriptorBindingStorageBufferUpdateAfterBind, skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // not all identical VkDescriptorBindingFlags flags
    VkDescriptorBindingFlags flags[3] = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT, 0,
                                         VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfo>();
    flags_create_info.bindingCount = 3;
    flags_create_info.pBindingFlags = &flags[0];

    VkDescriptorSetLayoutBinding binding[3] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
    };
    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    ds_layout_ci.bindingCount = 3;
    ds_layout_ci.pBindings = &binding[0];
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    ASSERT_VK_SUCCESS(vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout));

    VkDescriptorPoolSize pool_sizes = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3};
    auto dspci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    dspci.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_sizes;
    dspci.maxSets = 3;
    VkDescriptorPool pool;
    ASSERT_VK_SUCCESS(vk::CreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool));

    auto ds_alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout;
    VkDescriptorSet ds = VK_NULL_HANDLE;
    ASSERT_VK_SUCCESS(vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds));

    auto buff_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buff_create_info.size = 1024;
    buff_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkBufferObj buffer;
    buffer.init(*m_device, buff_create_info);

    VkDescriptorBufferInfo bufferInfo[3] = {};
    bufferInfo[0].buffer = buffer.handle();
    bufferInfo[0].offset = 0;
    bufferInfo[0].range = 1024;
    bufferInfo[1] = bufferInfo[0];
    bufferInfo[2] = bufferInfo[0];

    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = ds;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = bufferInfo;
    // If the dstBinding has fewer than descriptorCount, remainder will be used to update the subsequent binding
    descriptor_write.descriptorCount = 3;

    // binding 1 has a different VkDescriptorBindingFlags
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
    vk::DestroyDescriptorPool(m_device->handle(), pool, nullptr);
}

TEST_F(VkLayerTest, AllocatePushDescriptorSet) {
    TEST_DESCRIPTION("Attempt to allocate a push descriptor set.");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto push_descriptor_prop = GetPushDescriptorProperties(instance(), gpu());
    if (push_descriptor_prop.maxPushDescriptors < 1) {
        // Some implementations report an invalid maxPushDescriptors of 0
        printf("%s maxPushDescriptors is zero, skipping tests\n", kSkipPrefix);
        return;
    }

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;
    VkDescriptorSetLayout ds_layout = VK_NULL_HANDLE;
    VkResult err = vk::CreateDescriptorSetLayout(m_device->handle(), &ds_layout_ci, nullptr, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    VkDescriptorPoolSize pool_size = {binding.descriptorType, binding.descriptorCount};
    auto dspci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    dspci.poolSizeCount = 1;
    dspci.pPoolSizes = &pool_size;
    dspci.maxSets = 1;
    VkDescriptorPool pool;
    err = vk::CreateDescriptorPool(m_device->handle(), &dspci, nullptr, &pool);
    ASSERT_VK_SUCCESS(err);

    auto ds_alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    ds_alloc_info.descriptorPool = pool;
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout;

    VkDescriptorSet ds = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-00308");
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
    m_errorMonitor->VerifyFound();

    vk::DestroyDescriptorPool(m_device->handle(), pool, nullptr);
    vk::DestroyDescriptorSetLayout(m_device->handle(), ds_layout, nullptr);
}

TEST_F(VkLayerTest, CreateDescriptorUpdateTemplate) {
    TEST_DESCRIPTION("Verify error messages for invalid vkCreateDescriptorUpdateTemplate calls.");

#ifdef __ANDROID__
    GTEST_SKIP() << "Skipped on Android pending further investigation.";
#endif

    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_ub(m_device, {dsl_binding});
    const VkDescriptorSetLayoutObj ds_layout_ub1(m_device, {dsl_binding});
    const VkDescriptorSetLayoutObj ds_layout_ub_push(m_device, {dsl_binding},
                                                     VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    const VkPipelineLayoutObj pipeline_layout(m_device, {{&ds_layout_ub, &ds_layout_ub1, &ds_layout_ub_push}});
    PFN_vkCreateDescriptorUpdateTemplateKHR vkCreateDescriptorUpdateTemplateKHR =
        (PFN_vkCreateDescriptorUpdateTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateDescriptorUpdateTemplateKHR");
    ASSERT_NE(vkCreateDescriptorUpdateTemplateKHR, nullptr);
    PFN_vkDestroyDescriptorUpdateTemplateKHR vkDestroyDescriptorUpdateTemplateKHR =
        (PFN_vkDestroyDescriptorUpdateTemplateKHR)vk::GetDeviceProcAddr(m_device->device(), "vkDestroyDescriptorUpdateTemplateKHR");
    ASSERT_NE(vkDestroyDescriptorUpdateTemplateKHR, nullptr);

    uint64_t badhandle = 0xcadecade;
    VkDescriptorUpdateTemplateEntry entries = {0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, sizeof(VkBuffer)};
    VkDescriptorUpdateTemplateCreateInfo create_info = LvlInitStruct<VkDescriptorUpdateTemplateCreateInfo>();
    create_info.flags = 0;
    create_info.descriptorUpdateEntryCount = 1;
    create_info.pDescriptorUpdateEntries = &entries;

    auto do_test = [&](std::string err) {
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, err);
        if (VK_SUCCESS == vkCreateDescriptorUpdateTemplateKHR(m_device->handle(), &create_info, nullptr, &dut)) {
            vkDestroyDescriptorUpdateTemplateKHR(m_device->handle(), dut, nullptr);
        }
        m_errorMonitor->VerifyFound();
    };

    // Descriptor set type template
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    // descriptorSetLayout is NULL
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350");

    // Bad pipelineLayout handle, to be ignored if templatType is DESCRIPTOR_SET
    {
        create_info.pipelineLayout = reinterpret_cast<VkPipelineLayout &>(badhandle);
        create_info.descriptorSetLayout = ds_layout_ub.handle();
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        if (VK_SUCCESS == vkCreateDescriptorUpdateTemplateKHR(m_device->handle(), &create_info, nullptr, &dut)) {
            vkDestroyDescriptorUpdateTemplateKHR(m_device->handle(), dut, nullptr);
        }
    }

    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
    // Bad pipelineLayout handle
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352");

    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    create_info.pipelineLayout = pipeline_layout.handle();
    create_info.set = 2;

    // Bad bindpoint -- force fuzz the bind point
    memset(&create_info.pipelineBindPoint, 0xFE, sizeof(create_info.pipelineBindPoint));
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00351");
    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

    // Bad pipeline layout
    create_info.pipelineLayout = VK_NULL_HANDLE;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00352");
    create_info.pipelineLayout = pipeline_layout.handle();

    // Wrong set #
    create_info.set = 0;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");

    // Invalid set #
    create_info.set = 42;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00353");

    // Bad descriptorSetLayout handle, to be ignored if templateType is PUSH_DESCRIPTORS
    // NOTE: AMD's Windows proprietary driver doesn't seem to ignore this handle
    create_info.set = 2;
    create_info.descriptorSetLayout = reinterpret_cast<VkDescriptorSetLayout &>(badhandle);
    if (!IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        VkDescriptorUpdateTemplateKHR dut = VK_NULL_HANDLE;
        if (VK_SUCCESS == vkCreateDescriptorUpdateTemplateKHR(m_device->handle(), &create_info, nullptr, &dut)) {
            vkDestroyDescriptorUpdateTemplateKHR(m_device->handle(), dut, nullptr);
        }
    }
    // Bad descriptorSetLayout handle
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    do_test("VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-00350");
}

TEST_F(VkLayerTest, InlineUniformBlockEXT) {
    TEST_DESCRIPTION("Test VK_EXT_inline_uniform_block.");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    std::array<const char *, 2> required_device_extensions = {
        {VK_KHR_MAINTENANCE_1_EXTENSION_NAME, VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    // Enable descriptor indexing if supported, but don't require it.
    bool supportsDescriptorIndexing = true;
    required_device_extensions = {{VK_KHR_MAINTENANCE_3_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            supportsDescriptorIndexing = false;
            return;
        }
    }

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);

    auto descriptor_indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    void *pNext = supportsDescriptorIndexing ? &descriptor_indexing_features : nullptr;
    // Create a device that enables inline_uniform_block
    auto inline_uniform_block_features = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>(pNext);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&inline_uniform_block_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    assert(vkGetPhysicalDeviceProperties2KHR != nullptr);

    // Get the inline uniform block limits
    auto inline_uniform_props = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockPropertiesEXT>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&inline_uniform_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorSetLayoutBinding dslb = {};
    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    VkDescriptorSetLayout ds_layout = {};

    // Test too many bindings
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    dslb.descriptorCount = 4;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    if (inline_uniform_props.maxInlineUniformBlockSize < dslb.descriptorCount) {
        printf("%sDescriptorCount exceeds InlineUniformBlockSize limit, skipping tests\n", kSkipPrefix);
        return;
    }

    uint32_t maxBlocks = std::max(inline_uniform_props.maxPerStageDescriptorInlineUniformBlocks,
                                  inline_uniform_props.maxDescriptorSetInlineUniformBlocks);
    if (maxBlocks > 4096) {
        printf("Too large of a maximum number of inline uniform blocks, skipping tests\n");
        return;
    }

    for (uint32_t i = 0; i < 1 + maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();
    VkResult err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    ASSERT_VK_SUCCESS(err);

    const char *max_inline_vuid = (supportsDescriptorIndexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-02214"
                                                               : "VUID-VkPipelineLayoutCreateInfo-descriptorType-02212";
    const char *max_all_inline_vuid = (supportsDescriptorIndexing) ? "VUID-VkPipelineLayoutCreateInfo-descriptorType-02216"
                                                                   : "VUID-VkPipelineLayoutCreateInfo-descriptorType-02213";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_inline_vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, max_all_inline_vuid);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02215");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02217");

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    err = vk::CreatePipelineLayout(m_device->device(), &pipeline_layout_ci, NULL, &pipeline_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyPipelineLayout(m_device->device(), pipeline_layout, NULL);
    pipeline_layout = VK_NULL_HANDLE;
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, nullptr);
    ds_layout = VK_NULL_HANDLE;

    // Single binding that's too large and is not a multiple of 4
    dslb.binding = 0;
    dslb.descriptorCount = inline_uniform_props.maxInlineUniformBlockSize + 1;

    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dslb;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-descriptorType-02209");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-descriptorType-02210");
    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, nullptr);
    ds_layout = VK_NULL_HANDLE;

    // Pool size must be a multiple of 4
    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    ds_type_count.descriptorCount = 33;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 2;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool ds_pool = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolSize-type-02218");
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);
    m_errorMonitor->VerifyFound();
    if (ds_pool) {
        vk::DestroyDescriptorPool(m_device->handle(), ds_pool, nullptr);
        ds_pool = VK_NULL_HANDLE;
    }

    // Create a valid pool
    ds_type_count.descriptorCount = 32;
    err = vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &ds_pool);

    // Create two valid sets with 8 bytes each
    dslb_vec.clear();
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    dslb.descriptorCount = 8;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dslb_vec.push_back(dslb);
    dslb.binding = 1;
    dslb_vec.push_back(dslb);

    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = &dslb_vec[0];

    err = vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);

    VkDescriptorSet descriptor_sets[2];
    VkDescriptorSetLayout set_layouts[2] = {ds_layout, ds_layout};
    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 2;
    alloc_info.descriptorPool = ds_pool;
    alloc_info.pSetLayouts = set_layouts;
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets);

    // Test invalid VkWriteDescriptorSet parameters (array element and size must be multiple of 4)
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_sets[0];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 3;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;

    uint32_t dummyData[8] = {};
    VkWriteDescriptorSetInlineUniformBlockEXT write_inline_uniform = LvlInitStruct<VkWriteDescriptorSetInlineUniformBlockEXT>();
    write_inline_uniform.dataSize = 3;
    write_inline_uniform.pData = &dummyData[0];
    descriptor_write.pNext = &write_inline_uniform;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02220");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.dstArrayElement = 1;
    descriptor_write.descriptorCount = 4;
    write_inline_uniform.dataSize = 4;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02219");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.pNext = nullptr;
    descriptor_write.dstArrayElement = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02221");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);
    m_errorMonitor->VerifyFound();

    descriptor_write.pNext = &write_inline_uniform;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    // Test invalid VkCopyDescriptorSet parameters (array element and size must be multiple of 4)
    VkCopyDescriptorSet copy_ds_update = LvlInitStruct<VkCopyDescriptorSet>();
    copy_ds_update.srcSet = descriptor_sets[0];
    copy_ds_update.srcBinding = 0;
    copy_ds_update.srcArrayElement = 0;
    copy_ds_update.dstSet = descriptor_sets[1];
    copy_ds_update.dstBinding = 0;
    copy_ds_update.dstArrayElement = 0;
    copy_ds_update.descriptorCount = 4;

    copy_ds_update.srcArrayElement = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyDescriptorSet-srcBinding-02223");
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.srcArrayElement = 0;
    copy_ds_update.dstArrayElement = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyDescriptorSet-dstBinding-02224");
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.dstArrayElement = 0;
    copy_ds_update.descriptorCount = 5;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkCopyDescriptorSet-srcBinding-02225");
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);
    m_errorMonitor->VerifyFound();

    copy_ds_update.descriptorCount = 4;
    vk::UpdateDescriptorSets(m_device->device(), 0, NULL, 1, &copy_ds_update);

    vk::DestroyDescriptorPool(m_device->handle(), ds_pool, nullptr);
    vk::DestroyDescriptorSetLayout(m_device->device(), ds_layout, nullptr);
}

TEST_F(VkLayerTest, InlineUniformBlockEXTFeature) {
    TEST_DESCRIPTION("Test VK_EXT_inline_uniform_block features.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    // Don't enable any features
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorSetLayoutBinding dslb = {};
    dslb.binding = 0;
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = 0;
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dslb;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-descriptorType-04604");
    VkDescriptorSetLayout ds_layout = {};
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, WrongdstArrayElement) {
    ASSERT_NO_FATAL_FAILURE(Init());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView view = image.targetView(VK_FORMAT_B8G8R8A8_UNORM);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = view;
    image_info.sampler = VK_NULL_HANDLE;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptor_set.image_infos.emplace_back(image_info);

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    descriptor_write.pImageInfo = descriptor_set.image_infos.data();
    descriptor_write.pBufferInfo = nullptr;
    descriptor_write.pTexelBufferView = nullptr;

    // sum of 3 pointing into array of 2 bindings
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    descriptor_write.dstArrayElement = 2;
    descriptor_set.descriptor_writes.emplace_back(descriptor_write);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    OneOffDescriptorSet descriptor_set2(m_device,
                                        {
                                            {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                            {1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                        });

    descriptor_set2.image_infos.emplace_back(image_info);
    descriptor_set2.image_infos.emplace_back(image_info);

    descriptor_write.dstSet = descriptor_set2.set_;
    descriptor_write.descriptorCount = 2;
    descriptor_write.pImageInfo = descriptor_set2.image_infos.data();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstArrayElement-00321");
    descriptor_write.dstArrayElement = 3;
    descriptor_set2.descriptor_writes.clear();
    descriptor_set2.descriptor_writes.emplace_back(descriptor_write);
    descriptor_set2.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DescriptorSetLayoutMisc) {
    TEST_DESCRIPTION("Various invalid ways to create a VkDescriptorSetLayout.");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 1;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;

    // Should succeed with shader stage of 0 or fragment
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);
    dsl_binding.stageFlags = 0;
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    vk::DestroyDescriptorSetLayout(device(), ds_layout, nullptr);

    dsl_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutBinding-descriptorType-01510");
    vk::CreateDescriptorSetLayout(device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NullDescriptorsDisabled) {
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr));

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02997");
    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.descriptor_writes.clear();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferInfo-buffer-02998");
    descriptor_set.WriteDescriptorBufferInfo(1, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.descriptor_writes.clear();
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-02995");
    VkBufferView buffer_view = VK_NULL_HANDLE;
    descriptor_set.WriteDescriptorBufferView(2, buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.descriptor_writes.clear();
    m_errorMonitor->VerifyFound();

    m_commandBuffer->begin();
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers-pBuffers-04001");
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &buffer, &offset);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, NullDescriptorsEnabled) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto robustness2_features = LvlInitStruct<VkPhysicalDeviceRobustness2FeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&robustness2_features);

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (!robustness2_features.nullDescriptor) {
        printf("%s nullDescriptor feature not supported, skipping test\n", kSkipPrefix);
        return;
    }

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device, {
                                                     {0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                     {2, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                 });

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorBufferInfo(1, VK_NULL_HANDLE, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    VkBufferView buffer_view = VK_NULL_HANDLE;
    descriptor_set.WriteDescriptorBufferView(2, buffer_view, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    descriptor_set.descriptor_writes.clear();

    m_commandBuffer->begin();
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &buffer, &offset);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorBufferInfo-buffer-02999");
    descriptor_set.WriteDescriptorBufferInfo(1, VK_NULL_HANDLE, 0, 16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    m_errorMonitor->VerifyFound();

    offset = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindVertexBuffers-pBuffers-04002");
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &buffer, &offset);
    m_commandBuffer->end();
    m_errorMonitor->VerifyFound();

    // Make sure sampler with NULL image view doesn't cause a crash or errors
    OneOffDescriptorSet sampler_descriptor_set(m_device,
                                               {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vk::CreateSampler(m_device->device(), &sampler_ci, NULL, &sampler);
    sampler_descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    sampler_descriptor_set.UpdateDescriptorSets();
    const VkPipelineLayoutObj pipeline_layout(m_device, {&sampler_descriptor_set.layout_});
    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=0) uniform sampler2D tex;
        layout(location=0) out vec4 x;
        void main(){
           x = texture(tex, vec2(1));
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
                              &sampler_descriptor_set.set_, 0, nullptr);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &m_viewports[0]);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &m_scissors[0]);
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    vk::DestroySampler(m_device->handle(), sampler, nullptr);
}

TEST_F(VkLayerTest, RenderPassCreatePotentialFormatFeatures) {
    TEST_DESCRIPTION("Validate PotentialFormatFeatures in renderpass create");

    // Check for VK_KHR_get_physical_device_properties2
    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }

    if (!OverrideDevsimForDeviceProfileLayer()) {
        GTEST_SKIP() << "Failed to override devsim for device profile layer.";
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    PFN_vkSetPhysicalDeviceFormatPropertiesEXT fpvkSetPhysicalDeviceFormatPropertiesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFormatPropertiesEXT fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFormatPropertiesEXT, fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    // Set format features from being found
    const VkFormat validColorFormat = VK_FORMAT_R8G8B8A8_UNORM;  // guaranteed to be valid everywhere
    const VkFormat invalidColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    const VkFormat depthFormat = VK_FORMAT_D16_UNORM;
    VkFormatProperties formatProps;
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), invalidColorFormat, &formatProps);
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), invalidColorFormat, formatProps);
    fpvkGetOriginalPhysicalDeviceFormatPropertiesEXT(gpu(), depthFormat, &formatProps);
    formatProps.linearTilingFeatures = 0;
    formatProps.optimalTilingFeatures = 0;
    fpvkSetPhysicalDeviceFormatPropertiesEXT(gpu(), depthFormat, formatProps);

    VkAttachmentDescription attachments[4] = {
        {0, validColorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        {0, invalidColorFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        {0, validColorFormat, VK_SAMPLE_COUNT_4_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL},
        {0, depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL}};

    VkAttachmentReference references[4] = {
        {0, VK_IMAGE_LAYOUT_GENERAL},  // valid color
        {1, VK_IMAGE_LAYOUT_GENERAL},  // invalid color
        {2, VK_IMAGE_LAYOUT_GENERAL},  // valid color multisample
        {3, VK_IMAGE_LAYOUT_GENERAL}   // invalid depth stencil
    };

    VkSubpassDescription subpass = {};
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &references[0];  // valid
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    VkSubpassDescription originalSubpass = subpass;

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 4, attachments, 1, &subpass, 0, nullptr};

    // Color attachment
    subpass.pColorAttachments = &references[1];
    TestRenderPassCreate(m_errorMonitor, device(), &rpci, rp2Supported, "VUID-VkSubpassDescription-pColorAttachments-02648",
                         "VUID-VkSubpassDescription2-pColorAttachments-02898");
    subpass = originalSubpass;

    // Input attachment
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &references[1];
    TestRenderPassCreate(m_errorMonitor, device(), &rpci, rp2Supported, "VUID-VkSubpassDescription-pInputAttachments-02647",
                         "VUID-VkSubpassDescription2-pInputAttachments-02897");
    subpass = originalSubpass;

    // Depth Stencil attachment
    subpass.pDepthStencilAttachment = &references[3];
    TestRenderPassCreate(m_errorMonitor, device(), &rpci, rp2Supported, "VUID-VkSubpassDescription-pDepthStencilAttachment-02650",
                         "VUID-VkSubpassDescription2-pDepthStencilAttachment-02900");
    subpass = originalSubpass;

    // Resolve attachment
    subpass.pResolveAttachments = &references[1];
    subpass.pColorAttachments = &references[2];  // valid
    // Can't use helper function due to need to set unexpected errors
    {
        VkRenderPass render_pass = VK_NULL_HANDLE;

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription-pResolveAttachments-02649");
        m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription-pResolveAttachments-00850");
        vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
        m_errorMonitor->VerifyFound();

        if (rp2Supported) {
            PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
                (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(device(), "vkCreateRenderPass2KHR");
            safe_VkRenderPassCreateInfo2 create_info2;
            ConvertVkRenderPassCreateInfoToV2KHR(rpci, &create_info2);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription2-pResolveAttachments-02899");
            m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pResolveAttachments-03068");
            vkCreateRenderPass2KHR(device(), create_info2.ptr(), nullptr, &render_pass);
            m_errorMonitor->VerifyFound();

            // For api version >= 1.2, try core entrypoint
            PFN_vkCreateRenderPass2 vkCreateRenderPass2 =
                (PFN_vkCreateRenderPass2)vk::GetDeviceProcAddr(device(), "vkCreateRenderPass2");
            if (vkCreateRenderPass2) {
                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription2-pResolveAttachments-02899");
                m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pResolveAttachments-03068");
                vkCreateRenderPass2(device(), create_info2.ptr(), nullptr, &render_pass);
                m_errorMonitor->VerifyFound();
            }
        }
    }
}

TEST_F(VkLayerTest, SubpassInputNotBoundDescriptorSet) {
    TEST_DESCRIPTION("Validate subpass input isn't bound to fragment shader or descriptor set");

    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageUsageFlags usage_input =
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_input(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(64, 64, 1, 1, format, usage_input, VK_IMAGE_TILING_OPTIMAL);
    image_input.Init(image_ci);
    VkImageView view_input = image_input.targetView(format);
    VkImageView attachments[] = {view_input, view_input};

    const VkAttachmentDescription inputAttachment = {
        0u,
        format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
    };
    std::vector<VkAttachmentDescription> attachmentDescs;
    attachmentDescs.push_back(inputAttachment);

    VkAttachmentReference inputRef = {
        0,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    std::vector<VkAttachmentReference> inputAttachments;
    inputAttachments.push_back(inputRef);

    const VkSubpassDescription subpass = {
        0u,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        static_cast<uint32_t>(inputAttachments.size()),
        inputAttachments.data(),
        0,
        nullptr,
        0u,
        nullptr,
        0u,
        nullptr,
    };
    const std::vector<VkSubpassDescription> subpasses(1u, subpass);

    const VkRenderPassCreateInfo rpci = {
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
    vk_testing::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = rp.handle();
    fbci.attachmentCount = 1u;
    fbci.pAttachments = attachments;
    fbci.width = 64;
    fbci.height = 64;
    fbci.layers = 1u;
    vk_testing::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler(*m_device, sampler_info);
    ASSERT_TRUE(sampler.initialized());

    // input index is wrong, it doesn't exist in supbass input attachments and the set and binding is undefined
    // It causes desired failures.
    char const *fsSource_fail = R"glsl(
        #version 450
        layout(input_attachment_index=1, set=0, binding=1) uniform subpassInput x;
        void main() {
           vec4 color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs_fail(this, fsSource_fail, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs_fail.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.gp_ci_.renderPass = rp.handle();
    g_pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "UNASSIGNED-CoreValidation-Shader-MissingInputAttachment");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkGraphicsPipelineCreateInfo-layout-00756");
    g_pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImageSubresourceOverlapBetweenAttachmentsAndDescriptorSets) {
    TEST_DESCRIPTION("Validate if attachments and descriptor set use the same image subresources");

    ASSERT_NO_FATAL_FAILURE(Init());
    if (IsPlatform(kNexusPlayer)) {
        printf("%s This test should not run on Nexus Player\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkFormat depth_format = FindSupportedDepthOnlyFormat(gpu());
    if (depth_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth only format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }
    VkDepthStencilObj depth_image(m_device);
    depth_image.Init(m_device, 64, 64, depth_format,
                  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_IMAGE_ASPECT_DEPTH_BIT);
    ASSERT_TRUE(depth_image.initialized());
    VkImageView depth_view = *depth_image.BindInfo();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(64, 64, 1, 2, format, usage, VK_IMAGE_TILING_OPTIMAL);
    image.Init(image_ci);
    VkImageView view_input = image.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1);
    VkImageView attachments[] = {view_input, depth_view};

    VkImageViewCreateInfo createView = LvlInitStruct<VkImageViewCreateInfo>();
    createView.image = image.handle();
    createView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createView.format = format;
    createView.components.r = VK_COMPONENT_SWIZZLE_R;
    createView.components.g = VK_COMPONENT_SWIZZLE_G;
    createView.components.b = VK_COMPONENT_SWIZZLE_B;
    createView.components.a = VK_COMPONENT_SWIZZLE_A;
    createView.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 1, 1};
    createView.flags = 0;
    vk_testing::ImageView view_sampler_overlap;
    view_sampler_overlap.init(*m_device, createView);

    createView.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vk_testing::ImageView view_sampler_not_overlap;
    view_sampler_not_overlap.init(*m_device, createView);

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
    std::vector<VkAttachmentDescription> attachmentDescs;
    attachmentDescs.push_back(inputAttachment);

    VkAttachmentReference inputRef = {
        0,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    std::vector<VkAttachmentReference> inputAttachments;
    inputAttachments.push_back(inputRef);

    const VkAttachmentDescription depthStencilAttachment = {0,
                                                            depth_format,
                                                            VK_SAMPLE_COUNT_1_BIT,
                                                            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                            VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                            VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                            VK_IMAGE_LAYOUT_GENERAL,
                                                            VK_IMAGE_LAYOUT_GENERAL};
    attachmentDescs.push_back(depthStencilAttachment);

    VkAttachmentReference depthStencilRef = {1, VK_IMAGE_LAYOUT_GENERAL};

    const VkSubpassDescription subpass = {
        0u,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        static_cast<uint32_t>(inputAttachments.size()),
        inputAttachments.data(),
        0,
        nullptr,
        nullptr,
        &depthStencilRef,
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
    vk_testing::RenderPass rp;
    rp.init(*m_device, renderPassInfo);

    const VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, 0, 0u, rp.handle(), 2u, attachments, 64, 64, 1u,
    };
    vk_testing::Framebuffer fb;
    fb.init(*m_device, fbci);

    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_info);

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput ia0;
        layout(set=0, binding=1) uniform sampler2D ci1;
        layout(set=0, binding=2) uniform sampler2D ci2;
        layout(set=0, binding=3) uniform sampler2D ci3;
        layout(set=0, binding=4) uniform sampler2D ci4;
        void main() {
           vec4 color = subpassLoad(ia0);
           color = texture(ci1, vec2(0));
           color = texture(ci2, vec2(0));
           color = texture(ci3, vec2(0));
           color = texture(ci4, vec2(0));
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.InitInfo();
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

    VkPipelineDepthStencilStateCreateInfo pipe_ds_state_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe_ds_state_ci.depthTestEnable = VK_TRUE;
    pipe_ds_state_ci.stencilTestEnable = VK_FALSE;

    g_pipe.gp_ci_.pDepthStencilState = &pipe_ds_state_ci;
    g_pipe.gp_ci_.renderPass = rp.handle();
    g_pipe.InitState();
    ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_input, sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    // input attachment and combined image sampler use the same view to cause DesiredFailure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(1, view_input, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // image subresource of input attachment and combined image sampler overlap to cause DesiredFailure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, view_sampler_overlap.handle(), sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // image subresource of input attachment and combined image sampler don't overlap. It should not cause failure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, view_sampler_not_overlap.handle(), sampler.handle(),
                                                     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // Both image subresource and depth stencil attachment are read only. It should not cause failure.
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(4, depth_view, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_renderPassBeginInfo.renderArea = {{0, 0}, {64, 64}};
    m_renderPassBeginInfo.renderPass = rp.handle();
    m_renderPassBeginInfo.framebuffer = fb.handle();

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06538");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, InvalidFragmentShadingRateFramebufferUsage) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment without the correct usage");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    GetPhysicalDeviceProperties2(properties2);

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    GetPhysicalDeviceFeatures2(features2);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentReference2KHR attach = LvlInitStruct<VkAttachmentReference2KHR>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2KHR subpass = LvlInitStruct<VkSubpassDescription2KHR>(&fsr_attachment);

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2KHR rpci = LvlInitStruct<VkRenderPassCreateInfo2KHR>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass rp(*m_device, rpci, true);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(1, 1, 1, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8_UINT);

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04548");
    vk_testing::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidFragmentShadingRateFramebufferDimensions) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment with too small dimensions");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    AddRequiredExtensions(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "This test is crashing on some AMD + Windows platforms without any validation errors getting hit; requires "
                        "investigation.";
    }

    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    GetPhysicalDeviceProperties2(properties2);

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeaturesKHR>();
    if (IsExtensionsEnabled(VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
        fsr_features.pNext = &multiview_features;
    }
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    GetPhysicalDeviceFeatures2(features2);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate not supported.";
    }

    if (fsr_properties.layeredShadingRateAttachments != VK_TRUE) {
        GTEST_SKIP() << "VkPhysicalDeviceFragmentShadingRatePropertiesKHR::layeredShadingRateAttachments not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentReference2 attach = LvlInitStruct<VkAttachmentReference2>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment);

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass rp(*m_device, rpci, true);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    VkImageCreateInfo ici = VkImageObj::ImageCreateInfo2D(
        1, 1, 1, 2, VK_FORMAT_R8_UINT, VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, VK_IMAGE_TILING_OPTIMAL, 0);
    image.InitNoLayout(ici);
    auto image_view_ci = image.TargetViewCI(VK_FORMAT_R8_UINT);
    image_view_ci.subresourceRange.layerCount = 2;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    const auto imageView = image.targetView(image_view_ci);

    VkFramebufferCreateInfo fb_info = LvlInitStruct<VkFramebufferCreateInfo>();
    fb_info.renderPass = rp.handle();
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = &imageView;
    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width * 2;
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;
    fb_info.layers = 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04539");
    vk_testing::Framebuffer fb(*m_device, fb_info);
    m_errorMonitor->VerifyFound();

    fb_info.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width;

    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height * 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04540");
    fb.init(*m_device, fb_info);
    m_errorMonitor->VerifyFound();
    fb_info.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height;

    VkImageView imageViewLayered = image.targetView(VK_FORMAT_R8_UINT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 2);
    fb_info.pAttachments = &imageViewLayered;
    fb_info.layers = 3;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04538");
    fb.init(*m_device, fb_info);
    m_errorMonitor->VerifyFound();

    if (multiview_features.multiview) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04538");
        fb.init(*m_device, fb_info);
        m_errorMonitor->VerifyFound();

        subpass.viewMask = 0x4;
        vk_testing::RenderPass rp2(*m_device, rpci, true);
        ASSERT_TRUE(rp2.initialized());
        subpass.viewMask = 0;

        fb_info.renderPass = rp2.handle();
        fb_info.layers = 1;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-flags-04537");
        fb.init(*m_device, fb_info);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, InvalidFragmentShadingRateAttachments) {
    TEST_DESCRIPTION("Specify a fragment shading rate attachment with too small dimensions");

    // Enable KHR_fragment_shading_rate and all of its required extensions
    bool fsr_extensions = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (fsr_extensions) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    fsr_extensions = fsr_extensions && DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    if (fsr_extensions) {
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_1_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        m_device_extension_names.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);
    } else {
        printf("%s requires VK_KHR_fragment_shading_rate.\n", kSkipPrefix);
        return;
    }

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceProperties2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fsr_properties =
        LvlInitStruct<VkPhysicalDeviceFragmentShadingRatePropertiesKHR>();
    VkPhysicalDeviceProperties2KHR properties2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&fsr_properties);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &properties2);

    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceFeatures2KHR");
    ASSERT_TRUE(vkGetPhysicalDeviceFeatures2KHR != nullptr);
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fsr_features = LvlInitStruct<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&fsr_features);
    vkGetPhysicalDeviceFeatures2KHR(gpu(), &features2);

    if (fsr_features.attachmentFragmentShadingRate != VK_TRUE) {
        printf("%s requires attachmentFragmentShadingRate feature.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentReference2 attach = LvlInitStruct<VkAttachmentReference2>();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;
    attach.attachment = 0;

    VkFragmentShadingRateAttachmentInfoKHR fsr_attachment = LvlInitStruct<VkFragmentShadingRateAttachmentInfoKHR>();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    fsr_attachment.pFragmentShadingRateAttachment = &attach;

    // Create a renderPass with a single fsr attachment
    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>(&fsr_attachment);

    VkAttachmentDescription2 attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    VkRenderPass rp;

    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");

    rpci.flags = VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-flags-04521");
    vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    rpci.flags = 0;
    attach_desc.format =
        FindFormatWithoutFeatures(gpu(), VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR);
    if (attach_desc.format) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-pAttachments-04586");
        vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
    }
    attach_desc.format = VK_FORMAT_R8_UINT;

    attach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04524");
    vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    attach.layout = VK_IMAGE_LAYOUT_GENERAL;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width + 1;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height + 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04525");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04528");
    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width ==
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.width) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04527");
    }
    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height ==
        fsr_properties.minFragmentShadingRateAttachmentTexelSize.height) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04530");
    }
    vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.minFragmentShadingRateAttachmentTexelSize.width / 2;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.minFragmentShadingRateAttachmentTexelSize.height / 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04526");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04529");
    vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width * 2;
    fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height * 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04527");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04530");
    vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
    m_errorMonitor->VerifyFound();
    fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;

    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width /
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.height >
        fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
        fsr_attachment.shadingRateAttachmentTexelSize.width = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.width;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04531");
        vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
        fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    }

    if (fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height /
            fsr_properties.minFragmentShadingRateAttachmentTexelSize.width >
        fsr_properties.maxFragmentShadingRateAttachmentTexelSizeAspectRatio) {
        fsr_attachment.shadingRateAttachmentTexelSize.height = fsr_properties.maxFragmentShadingRateAttachmentTexelSize.height;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                             "VUID-VkFragmentShadingRateAttachmentInfoKHR-pFragmentShadingRateAttachment-04532");
        vkCreateRenderPass2KHR(m_device->device(), &rpci, NULL, &rp);
        m_errorMonitor->VerifyFound();
        fsr_attachment.shadingRateAttachmentTexelSize = fsr_properties.minFragmentShadingRateAttachmentTexelSize;
    }
}

TEST_F(VkLayerTest, FramebufferDepthStencilResolveAttachmentTests) {
    TEST_DESCRIPTION("Create a framebuffer against a render pass using depth stencil resolve, with mismatched information");

    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    uint32_t attachmentWidth = 512;
    uint32_t attachmentHeight = 512;
    VkFormat attachmentFormat = FindSupportedDepthStencilFormat(gpu());
    if (attachmentFormat == VK_FORMAT_UNDEFINED) {
        printf("%s Did not find a supported depth stencil format; skipped.\n", kSkipPrefix);
        return;
    }

    VkAttachmentDescription2KHR attachmentDescriptions[2] = {};
    // Depth/stencil attachment
    attachmentDescriptions[0] = LvlInitStruct<VkAttachmentDescription2>();
    attachmentDescriptions[0].format = attachmentFormat;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    // Depth/stencil resolve attachment
    attachmentDescriptions[1] = LvlInitStruct<VkAttachmentDescription2>();
    attachmentDescriptions[1].format = attachmentFormat;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference2KHR depthStencilAttachmentReference = LvlInitStruct<VkAttachmentReference2>();
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 0;
    VkAttachmentReference2KHR depthStencilResolveAttachmentReference = LvlInitStruct<VkAttachmentReference2>();
    depthStencilResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilResolveAttachmentReference.attachment = 1;
    VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDepthStencilResolve =
        LvlInitStruct<VkSubpassDescriptionDepthStencilResolveKHR>();
    subpassDescriptionDepthStencilResolve.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentReference;
    subpassDescriptionDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    subpassDescriptionDepthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT_KHR;
    VkSubpassDescription2KHR subpassDescription = LvlInitStruct<VkSubpassDescription2>(&subpassDescriptionDepthStencilResolve);
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.attachmentCount = 2;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpassDescription;
    rpci.pAttachments = attachmentDescriptions;
    vk_testing::RenderPass rp(*m_device, rpci, true);
    ASSERT_TRUE(rp.initialized());

    // Depth resolve attachment, mismatched image usage
    // Try creating Framebuffer with images, but with invalid image create usage flags
    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = attachmentFormat;
    image_create_info.extent.width = attachmentWidth;
    image_create_info.extent.height = attachmentHeight;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.flags = 0;
    VkImageObj ds_image(m_device);
    ds_image.init(&image_create_info);
    ASSERT_TRUE(ds_image.initialized());

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageObj ds_resolve_image(m_device);
    ds_resolve_image.init(&image_create_info);
    ASSERT_TRUE(ds_resolve_image.initialized());

    VkImageView image_views[2];
    image_views[0] = ds_image.targetView(attachmentFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    image_views[1] = ds_resolve_image.targetView(image_create_info.format, VK_IMAGE_ASPECT_COLOR_BIT);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.width = attachmentWidth;
    fbci.height = attachmentHeight;
    fbci.layers = 1;
    fbci.renderPass = rp.handle();
    fbci.attachmentCount = 2;
    fbci.pAttachments = image_views;
    vk_testing::Framebuffer framebuffer;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-00880");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-pAttachments-02634");
    framebuffer.init(*m_device, fbci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DepthStencilResolveMode) {
    TEST_DESCRIPTION("Test valid usage of the VkResolveModeFlagBits");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    // DepthStencilResolve only can be done with RenderPass2
    if (!CheckCreateRenderPass2Support(this, m_device_extension_names)) {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        return;
    }

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)) {
        m_device_extension_names.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    } else {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR =
        (PFN_vkGetPhysicalDeviceProperties2KHR)vk::GetInstanceProcAddr(instance(), "vkGetPhysicalDeviceProperties2KHR");
    assert(vkGetPhysicalDeviceProperties2KHR != nullptr);
    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");
    assert(vkCreateRenderPass2KHR != nullptr);

    VkFormat depthFormat = FindSupportedDepthOnlyFormat(gpu());
    VkFormat stencilFormat = FindSupportedStencilOnlyFormat(gpu());
    VkFormat depthStencilFormat = FindSupportedDepthStencilFormat(gpu());
    if ((depthFormat == VK_FORMAT_UNDEFINED) || (stencilFormat == VK_FORMAT_UNDEFINED) ||
        (depthStencilFormat == VK_FORMAT_UNDEFINED)) {
        printf("%s Did not find a supported depth stencil formats; skipped.\n", kSkipPrefix);
        return;
    }

    VkPhysicalDeviceDepthStencilResolveProperties ds_resolve_props = LvlInitStruct<VkPhysicalDeviceDepthStencilResolveProperties>();
    VkPhysicalDeviceProperties2KHR prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&ds_resolve_props);
    vkGetPhysicalDeviceProperties2KHR(gpu(), &prop2);

    VkRenderPass renderPass;

    VkAttachmentDescription2KHR attachmentDescriptions[2] = {};
    // Depth/stencil attachment
    attachmentDescriptions[0] = LvlInitStruct<VkAttachmentDescription2KHR>();
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Depth/stencil resolve attachment
    attachmentDescriptions[1] = LvlInitStruct<VkAttachmentDescription2KHR>();
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    VkAttachmentReference2KHR depthStencilAttachmentReference = LvlInitStruct<VkAttachmentReference2KHR>();
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 0;
    VkAttachmentReference2KHR depthStencilResolveAttachmentReference = LvlInitStruct<VkAttachmentReference2KHR>();
    depthStencilResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilResolveAttachmentReference.attachment = 1;
    VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDSR = LvlInitStruct<VkSubpassDescriptionDepthStencilResolveKHR>();
    subpassDescriptionDSR.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentReference;
    VkSubpassDescription2KHR subpassDescription = LvlInitStruct<VkSubpassDescription2KHR>(&subpassDescriptionDSR);
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;

    VkRenderPassCreateInfo2KHR renderPassCreateInfo = LvlInitStruct<VkRenderPassCreateInfo2KHR>();
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;

    // Both modes can't be none
    attachmentDescriptions[0].format = depthStencilFormat;
    attachmentDescriptions[1].format = depthStencilFormat;
    subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_NONE;
    subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_NONE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03178");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();

    // Stencil is used but resolve is set to none, depthResolveMode should be ignored
    attachmentDescriptions[0].format = stencilFormat;
    attachmentDescriptions[1].format = stencilFormat;
    subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_NONE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03178");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;

    // Invalid use of UNUSED
    depthStencilAttachmentReference.attachment = VK_ATTACHMENT_UNUSED;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03177");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    depthStencilAttachmentReference.attachment = 0;

    // attachmentCount == 2
    depthStencilResolveAttachmentReference.attachment = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-pSubpasses-06473");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    depthStencilResolveAttachmentReference.attachment = 1;

    // test invalid sample counts
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03179");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_4_BIT;

    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_4_BIT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03180");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;

    // test resolve and non-resolve formats are not same types
    attachmentDescriptions[0].format = stencilFormat;
    attachmentDescriptions[1].format = depthFormat;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03181");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();

    attachmentDescriptions[0].format = depthFormat;
    attachmentDescriptions[1].format = stencilFormat;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03182");
    vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
    m_errorMonitor->VerifyFound();

    // test when independentResolve and independentResolve are false
    attachmentDescriptions[0].format = depthStencilFormat;
    attachmentDescriptions[1].format = depthStencilFormat;
    if (ds_resolve_props.independentResolve == VK_FALSE) {
        if (ds_resolve_props.independentResolveNone == VK_FALSE) {
            subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_NONE;
            subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            m_errorMonitor->SetDesiredFailureMsg(
                kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03185");
            vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
            m_errorMonitor->VerifyFound();
        } else {
            if ((ds_resolve_props.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0) {
                subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
                m_errorMonitor->SetDesiredFailureMsg(
                    kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03186");
                vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
                m_errorMonitor->VerifyFound();
            }
            if ((ds_resolve_props.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) != 0) {
                subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
                subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
                m_errorMonitor->SetDesiredFailureMsg(
                    kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-03186");
                vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
                m_errorMonitor->VerifyFound();
            }
        }
    } else {
        // test using unsupported resolve mode, which currently can only be AVERAGE
        // Need independentResolve to make easier to test
        if ((ds_resolve_props.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) == 0) {
            subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-depthResolveMode-03183");
            vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
            m_errorMonitor->VerifyFound();
        }
        if ((ds_resolve_props.supportedStencilResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT) == 0) {
            subpassDescriptionDSR.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
            subpassDescriptionDSR.stencilResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                                 "VUID-VkSubpassDescriptionDepthStencilResolve-stencilResolveMode-03184");
            vkCreateRenderPass2KHR(m_device->device(), &renderPassCreateInfo, nullptr, &renderPass);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(VkLayerTest, RenderPassCreateInvalidInputAttachmentLayout) {
    TEST_DESCRIPTION("Create renderpass where an input attachment is also uses as another type");

    bool rp2_supported = InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    if (rp2_supported) m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    rp2_supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkAttachmentDescription attach0 = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    const VkAttachmentDescription attach1 = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    const VkAttachmentReference ref0 = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference ref1 = {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    const VkAttachmentReference inRef0 = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    const VkAttachmentReference inRef1 = {1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    // First subpass draws to attachment 0
    const VkSubpassDescription subpass0 = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref0, nullptr, nullptr, 0, nullptr};
    // Second subpass reads attachment 0 as input-attachment, writes to attachment 1
    const VkSubpassDescription subpass1 = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &inRef0, 1, &ref1, nullptr, nullptr, 0, nullptr};
    // Seconnd subpass reads attachment 1 as input-attachment, writes to attachment 0
    const VkSubpassDescription subpass2 = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &inRef1, 1, &ref0, nullptr, nullptr, 0, nullptr};

    // Subpass 0 writes attachment 0 as output, subpass 1 reads as input (RAW)
    VkSubpassDependency dep0 = {0,
                               1,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    // Subpass 1 writes attachment 1 as output, subpass 2 reads as input while (RAW)
    VkSubpassDependency dep1 = {1,
                               2,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};
    // Subpass 1 reads attachment 0 as input, subpass 2 writes output (WAR)
    VkSubpassDependency dep2 = {1,
                               2,
                               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_DEPENDENCY_BY_REGION_BIT};

    std::vector<VkAttachmentDescription> attachs = {attach0, attach1};
    std::vector<VkSubpassDescription> subpasses = {subpass0, subpass1, subpass2};
    std::vector<VkSubpassDependency> deps = {dep0, dep1, dep2};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, (uint32_t)attachs.size(), attachs.data(), (uint32_t)subpasses.size(), subpasses.data(), (uint32_t)deps.size(), deps.data()};

    // Current setup should be OK -- no attachment is both input and output in same subpass
    PositiveTestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2_supported);

    // Check for invalid layout error when input-attachment is also used as color-attachment.
    // This should fail as attachment 0 is both input and output in subpass 2, but is not the correct layout
    // (Note that TestRenderPassCreate() filters out the VUID we want to test -- VUID-VkSubpassDescription-None-04437 -- so we can't use that here.)
    subpasses[2].pInputAttachments = &inRef0;
    const char *rp1_vuid = "VUID-VkSubpassDescription-None-04437";
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, rp1_vuid);
    {
        VkResult err;
        VkRenderPass render_pass = VK_NULL_HANDLE;
        err = vk::CreateRenderPass(m_device->device(), &rpci, nullptr, &render_pass);
        if (err == VK_SUCCESS) vk::DestroyRenderPass(m_device->device(), render_pass, nullptr);
    }
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, RenderPassMultiViewCreateInvalidViewMasks) {
    TEST_DESCRIPTION("Create a render pass with some view masks 0 and some not 0");

    // Check for VK_KHR_get_physical_device_properties2
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        } else {
            printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto render_pass_multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&render_pass_multiview_props);
    GetPhysicalDeviceProperties2(prop2);
    if (render_pass_multiview_props.maxMultiviewViewCount < 2) {
        printf("%s maxMultiviewViewCount lower than required, skipping test.\n", kSkipPrefix);
        return;
    }


    VkSubpassDescription subpasses[2];
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x3u, 0x0};
    uint32_t correlationMasks[] = {0x1u, 0x3u};
    VkRenderPassMultiviewCreateInfo rpmvci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 2, viewMasks, 0, nullptr, 2, correlationMasks};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 2, subpasses, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pNext-02513",
                         nullptr);
}

TEST_F(VkLayerTest, InvalidCreateDescriptorPoolFlags) {
    TEST_DESCRIPTION("Create descriptor pool with invalid flags.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
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
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool bad_pool;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolCreateInfo-flags-04607");
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &bad_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MissingMutableDescriptorTypeFeature) {
    TEST_DESCRIPTION("Create mutable descriptor pool with feature not enabled.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    VkDescriptorPool bad_pool;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolCreateInfo-flags-04609");
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &bad_pool);
    m_errorMonitor->VerifyFound();

    ds_type_count.type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    ds_pool_ci.flags = 0;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolCreateInfo-mutableDescriptorType-04608");
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, NULL, &bad_pool);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MutableDescriptorPoolsWithPartialOverlap) {
    TEST_DESCRIPTION("Create mutable descriptor pools with partial overlap.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
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

    VkDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    pool_sizes[1].descriptorCount = 1;

    VkDescriptorType first_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkDescriptorType second_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListVALVE lists[2] = {};
    lists[0].descriptorTypeCount = 2;
    lists[0].pDescriptorTypes = first_types;
    lists[1].descriptorTypeCount = 2;
    lists[1].pDescriptorTypes = second_types;

    VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mdtci.mutableDescriptorTypeListCount = 2;
    mdtci.pMutableDescriptorTypeLists = lists;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>(&mdtci);
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 2;
    ds_pool_ci.pPoolSizes = pool_sizes;

    VkDescriptorPool pool;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolCreateInfo-pPoolSizes-04787");
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, nullptr, &pool);
    m_errorMonitor->VerifyFound();

    lists[1].pDescriptorTypes = first_types;
    mdtci.mutableDescriptorTypeListCount = 1;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorPoolCreateInfo-pPoolSizes-04787");
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, nullptr, &pool);
    m_errorMonitor->VerifyFound();

    mdtci.mutableDescriptorTypeListCount = 2;
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, nullptr, &pool);
    vk::DestroyDescriptorPool(device(), pool, nullptr);

    second_types[0] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    lists[1].pDescriptorTypes = second_types;
    vk::CreateDescriptorPool(m_device->device(), &ds_pool_ci, nullptr, &pool);

    vk::DestroyDescriptorPool(device(), pool, nullptr);
}

TEST_F(VkLayerTest, InvalidCreateDescriptorPoolAllocateFlags) {
    TEST_DESCRIPTION("Create descriptor pool with invalid flags.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
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
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_SAMPLER;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.flags = 0;
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding_samp = {};
    dsl_binding_samp.binding = 0;
    dsl_binding_samp.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding_samp.descriptorCount = 1;
    dsl_binding_samp.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding_samp.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutObj ds_layout_samp(m_device, {dsl_binding_samp},
                                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE);

    VkDescriptorSetLayout set_layout = ds_layout_samp.handle();

    VkDescriptorSetAllocateInfo alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    alloc_info.descriptorSetCount = 1;
    alloc_info.descriptorPool = pool.handle();
    alloc_info.pSetLayouts = &set_layout;

    VkDescriptorSet descriptor_set;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetAllocateInfo-pSetLayouts-04610");
    vk::AllocateDescriptorSets(m_device->device(), &alloc_info, &descriptor_set);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidRenderArea) {
    TEST_DESCRIPTION("Begin render pass with render area that is not within the framebuffer.");

    AddOptionalExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    const bool device_group_supported = IsExtensionsEnabled(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);

    VkRenderPassBeginInfo rpbinfo = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbinfo.renderPass = m_renderPass;
    rpbinfo.framebuffer = m_framebuffer;
    rpbinfo.renderArea.extent.width = m_framebuffer_info.width;
    rpbinfo.renderArea.extent.height = m_framebuffer_info.height;
    rpbinfo.renderArea.offset.x = -32;
    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.clearValueCount = 1;
    rpbinfo.pClearValues = m_renderPassClearValues.data();

    m_commandBuffer->begin();

    if (device_group_supported) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-pNext-02850");
    } else {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderArea-02846");
    }
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    rpbinfo.renderArea.offset.x = 0;
    rpbinfo.renderArea.offset.y = -128;

    if (device_group_supported) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-pNext-02851");
    } else {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderArea-02847");
    }
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.renderArea.extent.width = m_framebuffer_info.width + 128;

    if (device_group_supported) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-pNext-02852");
    } else {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderArea-02848");
    }
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    rpbinfo.renderArea.extent.width = m_framebuffer_info.width;
    rpbinfo.renderArea.extent.height = m_framebuffer_info.height + 1;

    if (device_group_supported) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-pNext-02853");
    } else {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-renderArea-02849");
    }
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, SwapchainAcquireImageRetired) {
    TEST_DESCRIPTION("Test vkAcquireNextImageKHR with retired swapchain");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddSurfaceExtension();
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_TRUE(InitSwapchain());

    VkSwapchainCreateInfoKHR swapchain_create_info = LvlInitStruct<VkSwapchainCreateInfoKHR>();
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    swapchain_create_info.presentMode = m_surface_non_shared_present_mode;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = m_swapchain;

    VkSwapchainKHR swapchain;
    vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &swapchain);

    VkSemaphoreCreateInfo semaphore_create_info = LvlInitStruct<VkSemaphoreCreateInfo>();
    vk_testing::Semaphore semaphore(*m_device, semaphore_create_info);
    ASSERT_TRUE(semaphore.initialized());

    VkAcquireNextImageInfoKHR acquire_info = LvlInitStruct<VkAcquireNextImageInfoKHR>();
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = std::numeric_limits<uint64_t>::max();
    acquire_info.semaphore = semaphore.handle();
    acquire_info.fence = VK_NULL_HANDLE;
    acquire_info.deviceMask = 0x1;

    uint32_t dummy;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkAcquireNextImageKHR-swapchain-01285");
    vk::AcquireNextImageKHR(device(), m_swapchain, std::numeric_limits<uint64_t>::max(), semaphore.handle(), VK_NULL_HANDLE,
                            &dummy);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAcquireNextImageInfoKHR-swapchain-01675");
    vk::AcquireNextImage2KHR(device(), &acquire_info, &dummy);
    m_errorMonitor->VerifyFound();

    vk::DestroySwapchainKHR(m_device->device(), swapchain, nullptr);
}

TEST_F(VkLayerTest, InvalidDeviceGroupRenderArea) {
    TEST_DESCRIPTION("Begin render pass with device group render area that is not within the framebuffer.");

    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEVICE_GROUP_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkRect2D renderArea = {};
    renderArea.offset.x = -1;
    renderArea.offset.y = -1;
    renderArea.extent.width = 64;
    renderArea.extent.height = 64;

    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceMask = 0x1;
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &renderArea;

    VkRenderPassBeginInfo rpbinfo = LvlInitStruct<VkRenderPassBeginInfo>(&device_group_render_pass_begin_info);
    rpbinfo.renderPass = m_renderPass;
    rpbinfo.framebuffer = m_framebuffer;
    rpbinfo.renderArea.extent.width = m_framebuffer_info.width;
    rpbinfo.renderArea.extent.height = m_framebuffer_info.height;
    rpbinfo.renderArea.offset.x = -32;
    rpbinfo.renderArea.offset.y = 0;
    rpbinfo.clearValueCount = 1;
    rpbinfo.pClearValues = m_renderPassClearValues.data();

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06166");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06167");
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    renderArea.offset.x = 0;
    renderArea.offset.y = 1;
    renderArea.extent.width = m_framebuffer_info.width + 1;
    renderArea.extent.height = m_framebuffer_info.height;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-pNext-02856");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassBeginInfo-pNext-02857");
    m_commandBuffer->BeginRenderPass(rpbinfo);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, RenderPassBeginNullValues) {
    TEST_DESCRIPTION("Test invalid null entries for clear color");

    ASSERT_NO_FATAL_FAILURE(InitFramework());
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto rpbi = m_renderPassBeginInfo;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = nullptr;  // clearValueCount != 0, but pClearValues = null, leads to 04962
    TestRenderPassBegin(m_errorMonitor, m_device->device(), m_commandBuffer->handle(), &rpbi, false,
                        "VUID-VkRenderPassBeginInfo-clearValueCount-04962", nullptr);
}

TEST_F(VkLayerTest, DepthStencilResolveAttachmentInvalidFormat) {
    TEST_DESCRIPTION("Create subpass with VkSubpassDescriptionDepthStencilResolve that has an ");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    if (!rp2Supported) {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
        return;
    }

    if (!DeviceExtensionSupported(gpu(), nullptr, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)) {
        printf("%s Did not find required device extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
        return;
    }
    m_device_extension_names.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    VkAttachmentDescription2KHR attachmentDescriptions[2] = {};
    // Depth/stencil attachment
    attachmentDescriptions[0] = LvlInitStruct<VkAttachmentDescription2>();
    attachmentDescriptions[0].format = VK_FORMAT_R8_UNORM;
    attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Depth/stencil resolve attachment
    attachmentDescriptions[1] = LvlInitStruct<VkAttachmentDescription2>();
    attachmentDescriptions[1].format = ds_format;
    attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_4_BIT;
    attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    VkAttachmentReference2KHR depthStencilAttachmentReference = LvlInitStruct<VkAttachmentReference2KHR>();
    depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilAttachmentReference.attachment = 1;

    VkAttachmentReference2KHR depthStencilResolveAttachmentReference = LvlInitStruct<VkAttachmentReference2KHR>();
    depthStencilResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthStencilResolveAttachmentReference.attachment = 0;

    VkSubpassDescriptionDepthStencilResolveKHR subpassDescriptionDepthStencilResolve =
        LvlInitStruct<VkSubpassDescriptionDepthStencilResolveKHR>();
    subpassDescriptionDepthStencilResolve.pDepthStencilResolveAttachment = &depthStencilResolveAttachmentReference;
    subpassDescriptionDepthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
    subpassDescriptionDepthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;

    VkSubpassDescription2 subpassDescription = LvlInitStruct<VkSubpassDescription2>(&subpassDescriptionDepthStencilResolve);
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpassDescription;
    rpci.attachmentCount = 2;
    rpci.pAttachments = attachmentDescriptions;

    PFN_vkCreateRenderPass2KHR vkCreateRenderPass2KHR =
        (PFN_vkCreateRenderPass2KHR)vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR");

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit,
                                         "VUID-VkSubpassDescriptionDepthStencilResolve-pDepthStencilResolveAttachment-02651");
    vkCreateRenderPass2KHR(m_device->device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ValidateDescriptorBindingUpdateAfterBindWithAccelerationStructure) {
    TEST_DESCRIPTION("Validate acceleration structure descriptor writing.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_3_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    if (!InitFrameworkForRayTracingTest(this, false, false)) {
        GTEST_SKIP() << "unable to init ray tracing test";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_ALL;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 1;
    flags_create_info.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo create_info = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    VkDescriptorSetLayout setLayout;
    m_errorMonitor->SetDesiredFailureMsg(
        kErrorBit, "VUID-VkDescriptorSetLayoutBindingFlagsCreateInfo-descriptorBindingAccelerationStructureUpdateAfterBind-03570");
    vk::CreateDescriptorSetLayout(m_device->handle(), &create_info, nullptr, &setLayout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DescriptorUpdateOfMultipleBindingWithOneUpdateCall) {
    TEST_DESCRIPTION("Update a descriptor set containing multiple bindings with only one update");

    if (InstanceExtensionSupported(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        m_instance_extension_names.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    } else {
        printf("%s Did not find required instance extension %s; skipped.\n", kSkipPrefix,
               VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    std::array<const char *, 2> required_device_extensions = {
        {VK_KHR_MAINTENANCE_1_EXTENSION_NAME, VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME}};
    for (auto device_extension : required_device_extensions) {
        if (DeviceExtensionSupported(gpu(), nullptr, device_extension)) {
            m_device_extension_names.push_back(device_extension);
        } else {
            printf("%s %s Extension not supported, skipping tests\n", kSkipPrefix, device_extension);
            return;
        }
    }

    auto inlineUniformProps = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockPropertiesEXT>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&inlineUniformProps);
    GetPhysicalDeviceProperties2(prop2);

    VkPhysicalDeviceInlineUniformBlockFeaturesEXT extEnable = LvlInitStruct<VkPhysicalDeviceInlineUniformBlockFeaturesEXT>();
    extEnable.inlineUniformBlock = VK_TRUE;
    extEnable.descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE;

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &extEnable));
    VkResult res;

    float inline_data[] = {1.f, 2.f};

    vk_testing::DescriptorSetLayout descLayout;
    {
        VkDescriptorSetLayoutBinding layoutBinding[3] = {};
        uint32_t bindingCount[] = {sizeof(inline_data)/2, 0, sizeof(inline_data)/2};
        uint32_t bindingPoint[] = {0, 1, 2};
        VkDescriptorType descType[] = {VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
                                       VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT};
        for (size_t i = 0; i < 3; ++i) {
            layoutBinding[i].binding = bindingPoint[i];
            layoutBinding[i].descriptorCount = bindingCount[i];
            layoutBinding[i].descriptorType = descType[i];
            layoutBinding[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layoutCreate = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
        layoutCreate.bindingCount = 3;
        layoutCreate.pBindings = layoutBinding;

        if (inlineUniformProps.maxInlineUniformBlockSize < bindingCount[0]
            || inlineUniformProps.maxInlineUniformBlockSize < bindingCount[1]) {
            printf("%s DescriptorCount exceeds InlineUniformBlockSize limit, skipping tests\n", kSkipPrefix);
            return;
        }

        descLayout.init(*m_device, layoutCreate);

        ASSERT_TRUE(descLayout.initialized());
    }

    vk_testing::DescriptorPool descPool;
    {
        VkDescriptorPoolInlineUniformBlockCreateInfoEXT descPoolInlineInfo =
            LvlInitStruct<VkDescriptorPoolInlineUniformBlockCreateInfoEXT>();
        descPoolInlineInfo.maxInlineUniformBlockBindings = 2;

        VkDescriptorPoolSize poolSize[2];
        poolSize[0].descriptorCount = sizeof(inline_data);
        poolSize[0].type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
        poolSize[1].descriptorCount = 1;
        poolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

        VkDescriptorPoolCreateInfo poolCreate = LvlInitStruct<VkDescriptorPoolCreateInfo>(&descPoolInlineInfo);
        poolCreate.poolSizeCount = 2;
        poolCreate.pPoolSizes = poolSize;
        poolCreate.maxSets = 1;

        descPool.init(*m_device, poolCreate);
        ASSERT_TRUE(descPool.initialized());
    }

    VkDescriptorSet descSetHandle = VK_NULL_HANDLE;
    {
        VkDescriptorSetAllocateInfo allocInfo = LvlInitStruct<VkDescriptorSetAllocateInfo>();
        allocInfo.pSetLayouts = &descLayout.handle();
        allocInfo.descriptorSetCount = 1;
        allocInfo.descriptorPool = descPool.handle();

        // The Galaxy S10 device used in LunarG CI fails to allocate this descriptor set
        res = vk::AllocateDescriptorSets(m_device->device(), &allocInfo, &descSetHandle);
        if (res != VK_SUCCESS) {
            printf("%s vkAllocateDescriptorSets failed with error %d, skipping test.\n", kSkipPrefix, res);
            return;
        }
    }
    vk_testing::DescriptorSet descSet(*m_device, &descPool, descSetHandle);

    VkWriteDescriptorSetInlineUniformBlockEXT writeInlineUbDesc = LvlInitStruct<VkWriteDescriptorSetInlineUniformBlockEXT>();
    writeInlineUbDesc.dataSize = sizeof(inline_data);
    writeInlineUbDesc.pData = inline_data;

    VkWriteDescriptorSet writeDesc = LvlInitStruct<VkWriteDescriptorSet>(&writeInlineUbDesc);
    writeDesc.descriptorCount = sizeof(inline_data);
    writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    writeDesc.dstBinding = 0;
    writeDesc.dstArrayElement = 0;
    writeDesc.dstSet = descSet.handle();

    m_errorMonitor->Reset();
    vk::UpdateDescriptorSets(m_device->device(), 1, &writeDesc, 0, nullptr);
}

TEST_F(VkLayerTest, AccelerationStructureBindings) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR than allowed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto acc_structure_props = LvlInitStruct<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    GetPhysicalDeviceProperties2(acc_structure_props);

    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t maxBlocks = acc_structure_props.maxPerStageDescriptorUpdateAfterBindAccelerationStructures;
    if (maxBlocks > 4096) {
        GTEST_SKIP() << "Too large of a maximum number of per stage descriptor update after bind for acceleration structures, "
                        "skipping tests";
    }
    if (maxBlocks < 1) {
        GTEST_SKIP() << "Test requires maxPerStageDescriptorUpdateAfterBindAccelerationStructures >= 1";
    }

    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutBinding dslb = {};
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    for (uint32_t i = 0; i <= maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();

    vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-03572");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, AccelerationStructureBindingsNV) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV than allowed");

    AddRequiredExtensions(VK_NV_RAY_TRACING_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto ray_tracing_props = LvlInitStruct<VkPhysicalDeviceRayTracingPropertiesNV>();
    GetPhysicalDeviceProperties2(ray_tracing_props);

    ASSERT_NO_FATAL_FAILURE(InitState());

    uint32_t maxBlocks = ray_tracing_props.maxDescriptorSetAccelerationStructures;
    if (maxBlocks > 4096) {
        GTEST_SKIP() << "Too large of a maximum number of descriptor set acceleration structures, skipping tests";
    }
    if (maxBlocks < 1) {
        GTEST_SKIP() << "Test requires maxDescriptorSetAccelerationStructures >= 1";
    }

    std::vector<VkDescriptorSetLayoutBinding> dslb_vec = {};
    VkDescriptorSetLayoutBinding dslb = {};
    dslb.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    dslb.descriptorCount = 1;
    dslb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    for (uint32_t i = 0; i <= maxBlocks; ++i) {
        dslb.binding = i;
        dslb_vec.push_back(dslb);
    }

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.bindingCount = dslb_vec.size();
    ds_layout_ci.pBindings = dslb_vec.data();

    vk_testing::DescriptorSetLayout ds_layout(*m_device, ds_layout_ci);
    ASSERT_TRUE(ds_layout.initialized());

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &ds_layout.handle();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-descriptorType-02381");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidWriteMutableDescriptorSet) {
    TEST_DESCRIPTION("Write mutable descriptor set with invalid type.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto mutdesc_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
    auto features2 = GetPhysicalDeviceFeatures2(mutdesc_features);
    if (!mutdesc_features.mutableDescriptorType) {
        GTEST_SKIP() << "mutableDescriptorType feature not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool pool;
    pool.init(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListVALVE list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    vk_testing::DescriptorSetLayout ds_layout;
    ds_layout.init(*m_device, ds_layout_ci);
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkDescriptorSetAllocateInfo allocate_info = LvlInitStruct<VkDescriptorSetAllocateInfo>();
    allocate_info.descriptorPool = pool.handle();
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &ds_layout_handle;

    VkDescriptorSet descriptor_set;
    vk::AllocateDescriptorSets(device(), &allocate_info, &descriptor_set);

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
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-dstSet-04611");
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();

    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
}

TEST_F(VkLayerTest, MutableDescriptors) {
    TEST_DESCRIPTION("Test mutable descriptors");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mutable_descriptor_type_features);
    GetPhysicalDeviceFeatures2(features2);
    if (mutable_descriptor_type_features.mutableDescriptorType == VK_FALSE) {
        GTEST_SKIP() << "mutableDescriptorType feature is not supported, skipping test";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorType descriptor_types[] = {VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER};

    VkMutableDescriptorTypeListVALVE mutable_descriptor_type_list = {};
    mutable_descriptor_type_list.descriptorTypeCount = 1;
    mutable_descriptor_type_list.pDescriptorTypes = descriptor_types;

    VkMutableDescriptorTypeCreateInfoVALVE mutable_descriptor_type_ci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mutable_descriptor_type_ci.mutableDescriptorTypeListCount = 1;
    mutable_descriptor_type_ci.pMutableDescriptorTypeLists = &mutable_descriptor_type_list;

    VkDescriptorSetLayoutCreateInfo ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mutable_descriptor_type_ci);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    VkDescriptorSetLayout ds_layout;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMutableDescriptorTypeListVALVE-descriptorTypeCount-04599");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    mutable_descriptor_type_list.descriptorTypeCount = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMutableDescriptorTypeListVALVE-descriptorTypeCount-04597");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    mutable_descriptor_type_list.descriptorTypeCount = 2;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMutableDescriptorTypeListVALVE-pDescriptorTypes-04598");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMutableDescriptorTypeListVALVE-pDescriptorTypes-04600");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMutableDescriptorTypeListVALVE-pDescriptorTypes-04601");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMutableDescriptorTypeListVALVE-pDescriptorTypes-04602");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();

    descriptor_types[1] = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMutableDescriptorTypeListVALVE-pDescriptorTypes-04603");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, NULL, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, DescriptorUpdateTemplate) {
    TEST_DESCRIPTION("Use more bindings with a descriptorType of VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV than allowed");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    auto mutdesc_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
    auto features2 = GetPhysicalDeviceFeatures2(mutdesc_features);
    if (!mutdesc_features.mutableDescriptorType) {
        GTEST_SKIP() << "mutableDescriptorType feature not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    ds_type_count.descriptorCount = 1;

    VkDescriptorPoolCreateInfo ds_pool_ci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    ds_pool_ci.maxSets = 1;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vk_testing::DescriptorPool pool;
    pool.init(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_ALL;
    dsl_binding.pImmutableSamplers = nullptr;

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListVALVE list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&mdtci);
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &dsl_binding;

    vk_testing::DescriptorSetLayout ds_layout;
    ds_layout.init(*m_device, ds_layout_ci);
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    VkDescriptorUpdateTemplateEntry update_template_entry = {};
    update_template_entry.dstBinding = 0;
    update_template_entry.dstArrayElement = 0;
    update_template_entry.descriptorCount = 1;
    update_template_entry.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    update_template_entry.offset = 0;
    update_template_entry.stride = 16;

    auto update_template_ci = LvlInitStruct<VkDescriptorUpdateTemplateCreateInfo>();
    update_template_ci.descriptorUpdateEntryCount = 1;
    update_template_ci.pDescriptorUpdateEntries = &update_template_entry;
    update_template_ci.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET;
    update_template_ci.descriptorSetLayout = ds_layout_handle;

    VkDescriptorUpdateTemplate update_template = VK_NULL_HANDLE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorUpdateTemplateCreateInfo-templateType-04615");
    vk::CreateDescriptorUpdateTemplate(m_device->device(), &update_template_ci, nullptr, &update_template);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, MutableDescriptorSetLayout) {
    TEST_DESCRIPTION("Create mutable descriptor set layout.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    const bool push_descriptors = IsExtensionsEnabled(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
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
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_MUTABLE_VALVE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04593");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkMutableDescriptorTypeListVALVE list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    ds_layout_ci.pNext = &mdtci;

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vk::CreateSampler(device(), &sampler_ci, nullptr, &sampler);

    binding.pImmutableSamplers = &sampler;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04594");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    binding.pImmutableSamplers = nullptr;
    vk::DestroySampler(device(), sampler, nullptr);

    if (push_descriptors) {
        list.descriptorTypeCount = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        ds_layout_ci.flags =
            VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04590");
        vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, nullptr, &ds_layout);
        m_errorMonitor->VerifyFound();

        ds_layout_ci.flags =
            VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT | VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04592");
        vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, nullptr, &ds_layout);
        m_errorMonitor->VerifyFound();

        list.descriptorTypeCount = 2;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
        ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04591");
        vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, nullptr, &ds_layout);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(VkLayerTest, MutableDescriptorSetLayoutMissingFeature) {
    TEST_DESCRIPTION("Create mutable descriptor set layout without mutableDescriptorType feature enabled.");

    AddRequiredExtensions(VK_VALVE_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesVALVE>();
    mutable_descriptor_type_features.mutableDescriptorType = VK_FALSE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&mutable_descriptor_type_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding binding = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    ds_layout_ci.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE; // Invalid, feature is not enabled
    ds_layout_ci.bindingCount = 1;
    ds_layout_ci.pBindings = &binding;

    VkDescriptorSetLayout ds_layout;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-flags-04596");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();

    VkDescriptorType types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };

    VkMutableDescriptorTypeListVALVE list = {};
    list.descriptorTypeCount = 2;
    list.pDescriptorTypes = types;

    VkMutableDescriptorTypeCreateInfoVALVE mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoVALVE>();
    mdtci.mutableDescriptorTypeListCount = 1;
    mdtci.pMutableDescriptorTypeLists = &list;

    ds_layout_ci.pNext = &mdtci;
    ds_layout_ci.flags = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDescriptorSetLayoutCreateInfo-mutableDescriptorType-04595");
    vk::CreateDescriptorSetLayout(m_device->device(), &ds_layout_ci, nullptr, &ds_layout);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidDeviceGroupRenderAreaDynamicRendering) {
    TEST_DESCRIPTION("Begin rendering with invaid device group render area.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkRect2D renderArea = {};
    renderArea.offset.x = -1;
    renderArea.offset.y = -1;
    renderArea.extent.width = 64;
    renderArea.extent.height = 64;

    VkDeviceGroupRenderPassBeginInfo device_group_render_pass_begin_info = LvlInitStruct<VkDeviceGroupRenderPassBeginInfo>();
    device_group_render_pass_begin_info.deviceMask = 0x1;
    device_group_render_pass_begin_info.deviceRenderAreaCount = 1;
    device_group_render_pass_begin_info.pDeviceRenderAreas = &renderArea;

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&device_group_render_pass_begin_info);
    begin_rendering_info.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06166");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06167");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    renderArea.offset.x = 0;
    renderArea.offset.y = 0;
    renderArea.extent.width = m_device->props.limits.maxFramebufferWidth + 1;
    renderArea.extent.height = m_device->props.limits.maxFramebufferHeight + 1;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06168");
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkDeviceGroupRenderPassBeginInfo-offset-06169");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->end();
}

TEST_F(VkLayerTest, EndRenderingWithIncorrectlyStartedRenderpassInstance) {
    TEST_DESCRIPTION(
        "Test EndRendering without starting the instance with BeginRendering, in the same command buffer or in a different once");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        GTEST_SKIP() << "Test requires (unsupported) dynamicRendering , skipping.";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkAttachmentDescription attach[] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr},
    };
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, subpass, 0, nullptr};
    vk_testing::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo fbci = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &imageView, 32, 32, 1};
    vk_testing::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    m_commandBuffer->begin();

    VkRenderPassBeginInfo rpbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                                  nullptr,
                                  rp.handle(),
                                  fb.handle(),
                                  {{
                                       0,
                                       0,
                                   },
                                   {32, 32}},
                                  0,
                                  nullptr};

    m_commandBuffer->BeginRenderPass(rpbi, VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-None-06161");
    m_commandBuffer->EndRendering();
    m_errorMonitor->VerifyFound();

    VkFormat color_formats = {VK_FORMAT_R8G8B8A8_UNORM};

    VkCommandBufferInheritanceRenderingInfoKHR inheritance_rendering_info =
        LvlInitStruct<VkCommandBufferInheritanceRenderingInfoKHR>();
    inheritance_rendering_info.colorAttachmentCount = 1;
    inheritance_rendering_info.pColorAttachmentFormats = &color_formats;
    inheritance_rendering_info.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;

    VkCommandBufferObj secondary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    const VkCommandBufferInheritanceInfo cmdbuff_ii = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        &inheritance_rendering_info,  // pNext
        VK_NULL_HANDLE,
        0,  // subpass
        VK_NULL_HANDLE,
    };

    VkCommandBufferBeginInfo cmdbuff__bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                            nullptr,  // pNext
                                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, &cmdbuff_ii};
    cmdbuff__bi.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    secondary.begin(&cmdbuff__bi);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRendering-commandBuffer-06162");
    secondary.EndRendering();
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, EndRenderpassWithBeginRenderingRenderpassInstance) {
    TEST_DESCRIPTION(
        "Test EndRenderpass(2) starting the renderpass instance with BeginRendering");

    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework());

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&dynamic_rendering_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (!dynamic_rendering_features.dynamicRendering) {
        printf("%s Test requires (unsupported) dynamicRendering , skipping\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.imageView = imageView;

    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>();
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;
    begin_rendering_info.layerCount = 1;

    m_commandBuffer->begin();

    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass-None-06170");
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();

    auto vkCmdEndRenderPass2KHR =
        reinterpret_cast<PFN_vkCmdEndRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCmdEndRenderPass2KHR"));
    ASSERT_TRUE(vkCmdEndRenderPass2KHR != nullptr);

    VkSubpassEndInfoKHR subpassEndInfo = {VK_STRUCTURE_TYPE_SUBPASS_END_INFO_KHR, nullptr};

    VkCommandBufferObj primary(m_device, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    primary.begin();
    primary.BeginRendering(begin_rendering_info);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass2-None-06171");
    vkCmdEndRenderPass2KHR(primary.handle(), &subpassEndInfo);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImageSubresourceOverlapBetweenRenderPassAndDescriptorSets) {
    TEST_DESCRIPTION("Validate if attachments in render pass and descriptor set use the same image subresources");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass render_pass(*m_device, rpci);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 3;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.init(&image_create_info);

    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_testing::ImageView image_view;
    image_view.init(*m_device, ivci);
    VkImageView image_view_handle = image_view.handle();

    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.width = width;
    fbci.height = height;
    fbci.layers = 1;
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view_handle;

    vk_testing::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderPass = render_pass.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = m_renderPassClearValues.data();

    char const *fsSource = R"glsl(
            #version 450
            layout(location = 0) out vec4 x;
            layout(set = 0, binding = 0) writeonly uniform image2D image;
            void main(){
                x = vec4(1.0f);
                imageStore(image, ivec2(0), vec4(0.5f));
            }
        )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {layout_binding});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout});
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06537");

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestDescriptorReadFromWriteAttachment) {
    TEST_DESCRIPTION("Validate reading from a descriptor that uses same image view as framebuffer write attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass render_pass(*m_device, rpci);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 3;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.init(&image_create_info);

    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_testing::ImageView image_view;
    image_view.init(*m_device, ivci);
    VkImageView image_view_handle = image_view.handle();

    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.width = width;
    fbci.height = height;
    fbci.layers = 1;
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view_handle;

    vk_testing::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderPass = render_pass.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = m_renderPassClearValues.data();

    char const *fsSource = R"glsl(
            #version 450
            layout(location = 0) out vec4 color;
            layout(set = 0, binding = 0, rgba8) readonly uniform image2D image1;
            void main(){
                color = imageLoad(image1, ivec2(0));
            }
        )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {layout_binding});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout, &descriptor_set_layout});
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06538");

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestDescriptorWriteFromReadAttachment) {
    TEST_DESCRIPTION("Validate writting to a descriptor that uses same image view as framebuffer read attachment");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass render_pass(*m_device, rpci);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 3;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.init(&image_create_info);

    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vk_testing::ImageView image_view;
    image_view.init(*m_device, ivci);
    VkImageView image_view_handle = image_view.handle();

    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.width = width;
    fbci.height = height;
    fbci.layers = 1;
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view_handle;

    vk_testing::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderPass = render_pass.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = m_renderPassClearValues.data();

    char const *fsSource = R"glsl(
            #version 450
            layout(set = 0, binding = 0, rgba8) writeonly uniform image2D image1;
            layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput inputColor;
            void main(){
                vec4 color = subpassLoad(inputColor);
                imageStore(image1, ivec2(0), color);
            }
        )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);

    VkDescriptorSetLayoutBinding layout_binding1 = {};
    layout_binding1.binding = 0;
    layout_binding1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding1.descriptorCount = 1;
    layout_binding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding1.pImmutableSamplers = nullptr;
    VkDescriptorSetLayoutBinding layout_binding2 = {};
    layout_binding2.binding = 0;
    layout_binding2.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    layout_binding2.descriptorCount = 1;
    layout_binding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding2.pImmutableSamplers = nullptr;
    const VkDescriptorSetLayoutObj descriptor_set_layout1(m_device, {layout_binding1});
    const VkDescriptorSetLayoutObj descriptor_set_layout2(m_device, {layout_binding2});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout1, &descriptor_set_layout2});
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());

    OneOffDescriptorSet descriptor_set_storage_image(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });
    OneOffDescriptorSet descriptor_set_input_attachment(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                  });
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set_storage_image.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);
    descriptor_write.dstSet = descriptor_set_input_attachment.set_;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);


    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_storage_image.set_, 0, nullptr);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 1, 1,
                              &descriptor_set_input_attachment.set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-06539");
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, RenderPassMultiViewCreateInvalidViewOffsets) {
    TEST_DESCRIPTION("Create a render pass with invalid multiview pViewOffsets");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);

    if (!rp2Supported) {
        if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_MULTIVIEW_EXTENSION_NAME)) {
            m_device_extension_names.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        } else {
            printf("%s Extension %s is not supported.\n", kSkipPrefix, VK_KHR_MULTIVIEW_EXTENSION_NAME);
            return;
        }
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto render_pass_multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&render_pass_multiview_props);
    GetPhysicalDeviceProperties2(prop2);
    if (render_pass_multiview_props.maxMultiviewViewCount < 2) {
        printf("%s maxMultiviewViewCount lower than required, skipping test.\n", kSkipPrefix);
        return;
    }

    VkSubpassDescription subpasses[2];
    subpasses[0] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    subpasses[1] = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMasks[] = {0x1u, 0x2u};
    uint32_t correlationMasks[] = {0x1u, 0x2u};
    int32_t view_offset = 1;
    VkRenderPassMultiviewCreateInfo rpmvci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 2, viewMasks, 1, &view_offset, 2, correlationMasks};

    VkSubpassDependency dependency = {};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 2, subpasses, 1, &dependency};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, rp2Supported, "VUID-VkRenderPassCreateInfo-pNext-02512",
                         nullptr);
}

TEST_F(VkLayerTest, RenderPassMultiViewCreateInvalidViewMask) {
    TEST_DESCRIPTION("Create a render pass with invalid view mask");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    auto render_pass_multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&render_pass_multiview_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &prop2);

    if (render_pass_multiview_props.maxMultiviewViewCount >= 32) {
        printf("%s maxMultiviewViewCount too high, skipping test.\n", kSkipPrefix);
        return;
    }

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};
    uint32_t viewMask = 1 << render_pass_multiview_props.maxMultiviewViewCount;
    uint32_t correlationMask = 0x1u;
    VkRenderPassMultiviewCreateInfo rpmvci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO, nullptr, 1, &viewMask, 0, nullptr, 1, &correlationMask};

    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpmvci, 0, 0, nullptr, 1, &subpass, 0, nullptr};

    TestRenderPassCreate(m_errorMonitor, m_device->device(), &rpci, false, "VUID-VkRenderPassMultiviewCreateInfo-pViewMasks-06697",
                         nullptr);
}

TEST_F(VkLayerTest, InvalidRenderPassAttachmentFormat) {
    TEST_DESCRIPTION("Test creating render pass with attachment format VK_FORMAT_UNDEFINED");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_UNDEFINED;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr};

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription-format-06698");
    vk::CreateRenderPass(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    VkAttachmentDescription2 attach_desc_2 = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc_2.format = VK_FORMAT_UNDEFINED;
    attach_desc_2.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc_2.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc_2.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto subpass_2 = LvlInitStruct<VkSubpassDescription2>();

    auto render_pass_ci_2 = LvlInitStruct<VkRenderPassCreateInfo2>();
    render_pass_ci_2.attachmentCount = 1;
    render_pass_ci_2.pAttachments = &attach_desc_2;
    render_pass_ci_2.subpassCount = 1;
    render_pass_ci_2.pSubpasses = &subpass_2;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription2-format-06701");
    vk::CreateRenderPass2(device(), &render_pass_ci_2, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, SamplingFromReadOnlyDepthStencilAttachment) {
    TEST_DESCRIPTION("Use same image as depth stencil attachment in read only layer and as sampler");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t width = 32;
    const uint32_t height = 32;
    const VkFormat format = FindSupportedDepthStencilFormat(gpu());
    if (format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &attach_ref;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    vk_testing::RenderPass render_pass(*m_device, rpci);

    auto image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.flags = 0;

    VkImageObj image(m_device);
    image.init(&image_create_info);

    auto ivci = LvlInitStruct<VkImageViewCreateInfo>();
    ivci.image = image.handle();
    ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format = format;
    ivci.subresourceRange.layerCount = 1;
    ivci.subresourceRange.baseMipLevel = 0;
    ivci.subresourceRange.levelCount = 1;
    ivci.subresourceRange.baseArrayLayer = 0;
    ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vk_testing::ImageView image_view;
    image_view.init(*m_device, ivci);
    VkImageView image_view_handle = image_view.handle();

    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.width = width;
    fbci.height = height;
    fbci.layers = 1;
    fbci.renderPass = render_pass.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &image_view_handle;

    vk_testing::Framebuffer framebuffer(*m_device, fbci);

    VkRenderPassBeginInfo rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.framebuffer = framebuffer.handle();
    rpbi.renderPass = render_pass.handle();
    rpbi.renderArea.extent.width = width;
    rpbi.renderArea.extent.height = height;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = m_renderPassClearValues.data();

    char const *fsSource = R"glsl(
            #version 450
            layout(set = 0, binding = 0) uniform sampler2D depth;
            void main(){
                vec4 color = texture(depth, ivec2(0));
            }
        )glsl";

    VkShaderObj vs(this, bindStateVertShaderText, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineObj pipe(m_device);
    pipe.AddDefaultColorAttachment();
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    VkViewport viewport = {0.0f, 0.0f, 64.0f, 64.0f, 0.0f, 1.0f};
    m_viewports.push_back(viewport);
    pipe.SetViewport(m_viewports);
    VkRect2D rect = {};
    m_scissors.push_back(rect);
    pipe.SetScissor(m_scissors);
    auto pipe_ds_state_ci = LvlInitStruct<VkPipelineDepthStencilStateCreateInfo>();
    pipe_ds_state_ci.depthTestEnable = VK_TRUE;
    pipe_ds_state_ci.stencilTestEnable = VK_FALSE;
    pipe.SetDepthStencil(&pipe_ds_state_ci);

    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = nullptr;
    const VkDescriptorSetLayoutObj descriptor_set_layout(m_device, {layout_binding});

    const VkPipelineLayoutObj pipeline_layout(DeviceObj(), {&descriptor_set_layout, &descriptor_set_layout});
    pipe.CreateVKPipeline(pipeline_layout.handle(), render_pass.handle());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);

    m_commandBuffer->begin();
    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestColorAttachmentImageViewUsage) {
    TEST_DESCRIPTION("Create image view with missing usage bits.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL, 0);

    auto image_view_usage = LvlInitStruct<VkImageViewUsageCreateInfo>();
    image_view_usage.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto image_view_ci = LvlInitStruct<VkImageViewCreateInfo>(&image_view_usage);
    image_view_ci.image = image.handle();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_view_ci.subresourceRange.layerCount = 1;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vk_testing::ImageView image_view;
    image_view.init(*m_device, image_view_ci);

    vk_testing::Sampler sampler;
    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler.init(*m_device, sampler_ci);

    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler.handle();
    image_info.imageView = image_view.handle();
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.pImageInfo = &image_info;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkWriteDescriptorSet-descriptorType-00337");
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateRenderPassWithInvalidStencilLoadOp) {
    TEST_DESCRIPTION("Create render pass with invalid stencil load op.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    const VkFormat stencil_format = FindSupportedStencilOnlyFormat(gpu());
    if (stencil_format == VK_FORMAT_UNDEFINED) {
        printf("%s No stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    auto vkCreateRenderPass2KHR =
        reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR"));

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = stencil_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>();

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo2>();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription2-pNext-06704");
    vkCreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto attach_desc_stencil_layout = LvlInitStruct<VkAttachmentDescriptionStencilLayout>();
    attach_desc_stencil_layout.stencilInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc_stencil_layout.stencilFinalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.pNext = &attach_desc_stencil_layout;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription2-pNext-06705");
    vkCreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, CreateRenderPassWithViewMask) {
    TEST_DESCRIPTION("Create render pass with view mask, but multiview feature disabled.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(Init());

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto vkCreateRenderPass2KHR =
        reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR"));

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.viewMask = 0x1;

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo2>();
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription2-multiview-06558");
    vkCreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidSubpassDescriptionViewMask) {
    TEST_DESCRIPTION("Test creating render with invalid view mask bit");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&multiview_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (multiview_features.multiview == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }

    auto render_pass_multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    auto prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&render_pass_multiview_props);
    vk::GetPhysicalDeviceProperties2(gpu(), &prop2);

    if (render_pass_multiview_props.maxMultiviewViewCount >= 32) {
        printf("%s maxMultiviewViewCount too high, skipping test.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    auto subpass = LvlInitStruct<VkSubpassDescription2>();  //{0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr,
                                                            //nullptr, 0, nullptr};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.viewMask = 1 << render_pass_multiview_props.maxMultiviewViewCount;

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo2>();
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription2-viewMask-06706");
    vk::CreateRenderPass2(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, TestAllocatingVariableDescriptorSets) {
    TEST_DESCRIPTION("Test allocating large variable descriptor sets");

    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto indexing_features = LvlInitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&indexing_features);
    GetPhysicalDeviceFeatures2(features2);
    if (indexing_features.descriptorBindingVariableDescriptorCount == VK_FALSE) {
        GTEST_SKIP() << "descriptorBindingVariableDescriptorCount feature not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkDescriptorBindingFlagsEXT flags[2] = {0, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};
    auto flags_create_info = LvlInitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfoEXT>();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;

    VkDescriptorSetLayoutBinding bindings[2] = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, std::numeric_limits<uint32_t>::max() / 64, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    auto ds_layout_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>(&flags_create_info);
    ds_layout_ci.bindingCount = 2;
    ds_layout_ci.pBindings = bindings;
    vk_testing::DescriptorSetLayout ds_layout;
    ds_layout.init(*m_device, ds_layout_ci);
    VkDescriptorSetLayout ds_layout_handle = ds_layout.handle();

    auto count_alloc_info = LvlInitStruct<VkDescriptorSetVariableDescriptorCountAllocateInfoEXT>();
    count_alloc_info.descriptorSetCount = 1;
    uint32_t variable_count = 2;
    count_alloc_info.pDescriptorCounts = &variable_count;

    VkDescriptorPoolSize pool_sizes[2] = {{bindings[0].descriptorType, bindings[0].descriptorCount},
                                          {bindings[1].descriptorType, bindings[1].descriptorCount}};
    auto dspci = LvlInitStruct<VkDescriptorPoolCreateInfo>();
    dspci.poolSizeCount = 2;
    dspci.pPoolSizes = pool_sizes;
    dspci.maxSets = 1;
    vk_testing::DescriptorPool pool;
    pool.init(*m_device, dspci);

    VkDescriptorSetAllocateInfo ds_alloc_info = LvlInitStruct<VkDescriptorSetAllocateInfo>(&count_alloc_info);
    ds_alloc_info.descriptorPool = pool.handle();
    ds_alloc_info.descriptorSetCount = 1;
    ds_alloc_info.pSetLayouts = &ds_layout_handle;

    VkDescriptorSet ds;
    vk::AllocateDescriptorSets(m_device->handle(), &ds_alloc_info, &ds);
}

TEST_F(VkLayerTest, TestPipelineSubpassIndex) {
    TEST_DESCRIPTION("Test using pipeline with incompatible subpass index for current renderpass subpass");

    ASSERT_NO_FATAL_FAILURE(Init());

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference attach_ref = {};
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sci[2] = {};
    sci[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sci[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sci[1].colorAttachmentCount = 1;
    sci[1].pColorAttachments = &attach_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = 0;
    dependency.dstSubpass = 1;
    dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    VkRenderPassCreateInfo render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    render_pass_ci.subpassCount = 2;
    render_pass_ci.pSubpasses = sci;
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.pDependencies = &dependency;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, render_pass_ci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    VkFramebufferCreateInfo framebuffer_ci = LvlInitStruct<VkFramebufferCreateInfo>();
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = &imageView;
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, framebuffer_ci);

    CreatePipelineHelper pipe1(*this);
    pipe1.InitInfo();
    pipe1.gp_ci_.renderPass = render_pass.handle();
    pipe1.gp_ci_.subpass = 0;
    pipe1.InitState();
    pipe1.CreateGraphicsPipeline();

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    pipe2.gp_ci_.renderPass = render_pass.handle();
    pipe2.gp_ci_.subpass = 1;
    pipe2.InitState();
    pipe2.CreateGraphicsPipeline();

    VkClearValue clear_value = {};
    clear_value.color = {{0, 0, 0, 0}};

    VkRenderPassBeginInfo render_pass_bi = LvlInitStruct<VkRenderPassBeginInfo>();
    render_pass_bi.renderPass = render_pass.handle();
    render_pass_bi.framebuffer = framebuffer.handle();
    render_pass_bi.renderArea = {{0, 0}, {32, 32}};
    render_pass_bi.clearValueCount = 1;
    render_pass_bi.pClearValues = &clear_value;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_bi);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-subpass-02685");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-subpass-02685");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, TestAllViewMasksZero) {
    TEST_DESCRIPTION("Test VkRenderPassMultiviewCreateInfo with all view mask elements being 0.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    ASSERT_NO_FATAL_FAILURE(Init());
    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency dependency = {};
    dependency.dependencyFlags = VK_DEPENDENCY_VIEW_LOCAL_BIT;
    dependency.srcSubpass = 0;
    dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    auto render_pass_multiview_ci = LvlInitStruct<VkRenderPassMultiviewCreateInfo>();
    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>(&render_pass_multiview_ci);
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass_description;
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.pDependencies = &dependency;
    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo-pNext-02514");
    vk::CreateRenderPass(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();

    uint32_t correlation_mask = 0x1;
    render_pass_ci.dependencyCount = 0;
    render_pass_multiview_ci.correlationMaskCount = 1;
    render_pass_multiview_ci.pCorrelationMasks = &correlation_mask;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo-pNext-02515");
    vk::CreateRenderPass(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, FragmentDensityMappAttachmentCount) {
    TEST_DESCRIPTION("Test attachmentCount of VkRenderPassFragmentDensityMapCreateInfoEXT.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_FRAGMENT_DENSITY_MAP_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto fdm_features = LvlInitStruct<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&fdm_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);

    if (fdm_features.fragmentDensityMap != VK_TRUE) {
        printf("%s requires fragmentDensityMap feature.\n", kSkipPrefix);
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    auto vkCreateRenderPass2KHR =
        reinterpret_cast<PFN_vkCreateRenderPass2KHR>(vk::GetDeviceProcAddr(m_device->device(), "vkCreateRenderPass2KHR"));

    VkAttachmentDescription2 attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference ref = {};
    ref.attachment = 1;
    ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
    auto rpfdmi = LvlInitStruct<VkRenderPassFragmentDensityMapCreateInfoEXT>();
    rpfdmi.fragmentDensityMapAttachment = ref;

    // Create a renderPass with viewMask 0
    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.viewMask = 0;

    VkRenderPassCreateInfo2 render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo2>(&rpfdmi);
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &attach_desc;

    VkRenderPass render_pass;
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-fragmentDensityMapAttachment-06472");
    vkCreateRenderPass2KHR(device(), &render_pass_ci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, InvalidRenderPassAttachmentUndefinedLayout) {
    TEST_DESCRIPTION("Create render pass with invalid attachment undefined layout.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    bool rp2Supported = CheckCreateRenderPass2Support(this, m_device_extension_names);
    ASSERT_NO_FATAL_FAILURE(InitState());

    const VkFormat ds_format = FindSupportedDepthStencilFormat(gpu());
    if (ds_format == VK_FORMAT_UNDEFINED) {
        printf("%s No Depth + Stencil format found rest of tests skipped.\n", kSkipPrefix);
        return;
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkAttachmentDescription attach_desc = {};
    attach_desc.format = ds_format;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;

    TestRenderPassCreate(m_errorMonitor, device(), &rpci, rp2Supported, "VUID-VkAttachmentDescription-format-06699",
                         "VUID-VkAttachmentDescription2-format-06702");

    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkAttachmentDescription-format-06700");
    VkRenderPass render_pass;
    vk::CreateRenderPass(device(), &rpci, nullptr, &render_pass);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkLayerTest, ImagelessFramebufferWith3DImage) {
    TEST_DESCRIPTION("Create imageless framebuffer with image view from 3D image.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    auto imageless_framebuffer = LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeatures>();
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&imageless_framebuffer);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features2);
    if (imageless_framebuffer.imagelessFramebuffer == VK_FALSE) {
        printf("%s multiview feature not supported, skipping test.\n", kSkipPrefix);
        return;
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    VkSubpassDescription subpass = {};

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    VkAttachmentDescription attachment = {};
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkRenderPassCreateInfo rp_ci = LvlInitStruct<VkRenderPassCreateInfo>();
    rp_ci.subpassCount = 1;
    rp_ci.pSubpasses = &subpass;
    rp_ci.attachmentCount = 1;
    rp_ci.pAttachments = &attachment;

    vk_testing::RenderPass render_pass;
    render_pass.init(*m_device, rp_ci);

    auto image_ci = LvlInitStruct<VkImageCreateInfo>();
    image_ci.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_3D;
    image_ci.format = format;
    image_ci.extent.width = 32;
    image_ci.extent.height = 32;
    image_ci.extent.depth = 4;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView imageView = image.targetView(format, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 4, VK_IMAGE_VIEW_TYPE_2D_ARRAY);

    VkFramebufferAttachmentImageInfo framebuffer_attachment_image_info = LvlInitStruct<VkFramebufferAttachmentImageInfo>();
    framebuffer_attachment_image_info.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    framebuffer_attachment_image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    framebuffer_attachment_image_info.width = 32;
    framebuffer_attachment_image_info.height = 32;
    framebuffer_attachment_image_info.layerCount = 4;
    framebuffer_attachment_image_info.viewFormatCount = 1;
    framebuffer_attachment_image_info.pViewFormats = &format;

    VkFramebufferAttachmentsCreateInfo framebuffer_attachments = LvlInitStruct<VkFramebufferAttachmentsCreateInfo>();
    framebuffer_attachments.attachmentImageInfoCount = 1;
    framebuffer_attachments.pAttachmentImageInfos = &framebuffer_attachment_image_info;

    VkFramebufferCreateInfo framebuffer_ci = LvlInitStruct<VkFramebufferCreateInfo>(&framebuffer_attachments);
    framebuffer_ci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
    framebuffer_ci.renderPass = render_pass.handle();
    framebuffer_ci.attachmentCount = 1;
    framebuffer_ci.pAttachments = &imageView;
    framebuffer_ci.width = 32;
    framebuffer_ci.height = 32;
    framebuffer_ci.layers = 1;

    vk_testing::Framebuffer framebuffer;
    framebuffer.init(*m_device, framebuffer_ci);

    VkClearValue clear_value = {};
    clear_value.color = {{0u, 0u, 0u, 0u}};

    VkRenderPassAttachmentBeginInfo render_pass_attachment_bi = LvlInitStruct<VkRenderPassAttachmentBeginInfo>();
    render_pass_attachment_bi.attachmentCount = 1;
    render_pass_attachment_bi.pAttachments = &imageView;

    VkRenderPassBeginInfo render_pass_bi = LvlInitStruct<VkRenderPassBeginInfo>(&render_pass_attachment_bi);
    render_pass_bi.renderPass = render_pass.handle();
    render_pass_bi.framebuffer = framebuffer.handle();
    render_pass_bi.clearValueCount = 1;
    render_pass_bi.pClearValues = &clear_value;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(render_pass_bi);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkLayerTest, MultisampledRenderToSingleSampled) {
    TEST_DESCRIPTION("Test VK_EXT_multisampled_render_to_single_sampled");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    AddRequiredExtensions(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    auto dynamic_rendering_features = LvlInitStruct<VkPhysicalDeviceDynamicRenderingFeaturesKHR>();

    auto ms_render_to_single_sampled_features =
        LvlInitStruct<VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT>(&dynamic_rendering_features);
    auto imageless_features = LvlInitStruct<VkPhysicalDeviceImagelessFramebufferFeaturesKHR>(&ms_render_to_single_sampled_features);
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&imageless_features);
    InitFrameworkAndRetrieveFeatures(features2);

    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    bool imageless_fb_supported = IsExtensionsEnabled(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);

    auto vulkan_12_features = LvlInitStruct<VkPhysicalDeviceVulkan12Properties>();
    VkPhysicalDeviceProperties2KHR prop2 = LvlInitStruct<VkPhysicalDeviceProperties2KHR>(&vulkan_12_features);
    vk::GetPhysicalDeviceProperties2(gpu(), &prop2);

    ms_render_to_single_sampled_features.multisampledRenderToSingleSampled = true;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkAttachmentReference2 attachmentRef = LvlInitStruct<VkAttachmentReference2>();
    attachmentRef.layout = VK_IMAGE_LAYOUT_GENERAL;
    attachmentRef.attachment = 0;
    VkAttachmentReference2 depthRef = LvlInitStruct<VkAttachmentReference2>();
    depthRef.layout = VK_IMAGE_LAYOUT_GENERAL;
    depthRef.attachment = 1;

    auto ms_render_to_ss = LvlInitStruct<VkMultisampledRenderToSingleSampledInfoEXT>();
    ms_render_to_ss.multisampledRenderToSingleSampledEnable = VK_TRUE;
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;

    VkSubpassDescription2 subpass = LvlInitStruct<VkSubpassDescription2>(&ms_render_to_ss);
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachmentRef;

    VkRenderPassCreateInfo2 rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 2;

    VkAttachmentDescription2 attach_desc[2] = {};
    attach_desc[0] = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc[0].format = VK_FORMAT_B8G8R8A8_UNORM;
    attach_desc[0].samples = VK_SAMPLE_COUNT_4_BIT;
    attach_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
    attach_desc[1] = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc[1].format = VK_FORMAT_D32_SFLOAT;
    attach_desc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc[1].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    rpci.pAttachments = attach_desc;

    VkRenderPass rp;
    // attach_desc[0].samples != ms_state.rasterizationSamples
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription2-pNext-06870");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();

    attach_desc[0].samples = VK_SAMPLE_COUNT_2_BIT;
    subpass.pDepthStencilAttachment = &depthRef;
    // Depth VK_SAMPLE_COUNT_1_BIT, no VkSubpassDescriptionDepthStencilResolve in pNext
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescription2-pNext-06871");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();

    auto depth_stencil_resolve = LvlInitStruct<VkSubpassDescriptionDepthStencilResolve>();
    ms_render_to_ss.pNext = &depth_stencil_resolve;
    // VkSubpassDescriptionDepthStencilResolve depthResolveMode and stencilResolveMode both VK_RESOLVE_MODE_NONE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06873");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();

    VkResolveModeFlagBits unsupported_depth = VK_RESOLVE_MODE_NONE;
    VkResolveModeFlagBits supported_depth = VK_RESOLVE_MODE_NONE;
    VkResolveModeFlagBits unsupported_stencil = VK_RESOLVE_MODE_NONE;
    VkResolveModeFlagBits supported_stencil = VK_RESOLVE_MODE_NONE;
    for (VkResolveModeFlagBits i = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT; i <= VK_RESOLVE_MODE_MAX_BIT;
         i = VkResolveModeFlagBits(i << 1)) {
        if ((unsupported_depth == VK_RESOLVE_MODE_NONE) && !(i & vulkan_12_features.supportedDepthResolveModes)) {
            unsupported_depth = i;
        }
        if ((unsupported_stencil == VK_RESOLVE_MODE_NONE) && !(i & vulkan_12_features.supportedStencilResolveModes)) {
            unsupported_stencil = i;
        }
        if (supported_stencil == VK_RESOLVE_MODE_NONE) {
            if (i & vulkan_12_features.supportedDepthResolveModes) {
                supported_stencil = i;
            }
        } else if (supported_depth == VK_RESOLVE_MODE_NONE) {
            // Want supported depth different than supported stencil
            if (i & vulkan_12_features.supportedDepthResolveModes) {
                supported_depth = i;
            }
        }
    }
    if (unsupported_depth != VK_RESOLVE_MODE_NONE) {
        depth_stencil_resolve.depthResolveMode = unsupported_depth;
        // depthResolveMode unsupported
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06874");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
        depth_stencil_resolve.depthResolveMode = VK_RESOLVE_MODE_NONE;
    }

    if (unsupported_stencil != VK_RESOLVE_MODE_NONE) {
        attach_desc[1].format = VK_FORMAT_S8_UINT;
        depth_stencil_resolve.stencilResolveMode = unsupported_stencil;
        // stencilResolveMode unsupported
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06875");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
        depth_stencil_resolve.stencilResolveMode = VK_RESOLVE_MODE_NONE;
        attach_desc[1].format = VK_FORMAT_D32_SFLOAT;
    }

    if (!(vulkan_12_features.independentResolve) && !(vulkan_12_features.independentResolveNone) &&
        (supported_depth != VK_RESOLVE_MODE_NONE) && (supported_stencil != VK_RESOLVE_MODE_NONE)) {
        depth_stencil_resolve.depthResolveMode = supported_depth;
        depth_stencil_resolve.stencilResolveMode = supported_stencil;
        attach_desc[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        // Stencil and depth resolve modes must be the same
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06876");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
    }

    if (!(vulkan_12_features.independentResolve) && vulkan_12_features.independentResolveNone &&
        (supported_depth != VK_RESOLVE_MODE_NONE) && (supported_stencil != VK_RESOLVE_MODE_NONE)) {
        depth_stencil_resolve.depthResolveMode = supported_depth;
        depth_stencil_resolve.stencilResolveMode = supported_stencil;
        attach_desc[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        // Stencil and depth resolve modes must be the same or one of them must be VK_RESOLVE_MODE_NONE
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDescriptionDepthStencilResolve-pNext-06877");
        vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
        m_errorMonitor->VerifyFound();
    }

    ms_render_to_ss.pNext = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    // rasterizationSamples can't be VK_SAMPLE_COUNT_1_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMultisampledRenderToSingleSampledInfoEXT-rasterizationSamples-06878");
    vk::CreateRenderPass2(device(), &rpci, nullptr, &rp);
    m_errorMonitor->VerifyFound();
    attach_desc[0].samples = VK_SAMPLE_COUNT_2_BIT;

    ms_render_to_ss.pNext = nullptr;
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    subpass.pDepthStencilAttachment = nullptr;
    rpci.attachmentCount = 1;
    // Create a usable renderpass
    vk_testing::RenderPass test_rp(*m_device, rpci);

    VkPipelineMultisampleStateCreateInfo ms_state = LvlInitStruct<VkPipelineMultisampleStateCreateInfo>();
    ms_state.flags = 0;
    ms_state.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    ms_state.sampleShadingEnable = VK_FALSE;
    ms_state.minSampleShading = 0.0f;
    ms_state.pSampleMask = nullptr;
    ms_state.alphaToCoverageEnable = VK_FALSE;
    ms_state.alphaToOneEnable = VK_FALSE;

    CreatePipelineHelper pipe_helper(*this);
    pipe_helper.InitInfo();
    pipe_helper.InitState();
    pipe_helper.gp_ci_.renderPass = test_rp.handle();
    pipe_helper.pipe_ms_state_ci_ = ms_state;

    // ms_render_to_ss.rasterizationSamples != ms_state.rasterizationSamples
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06854");
    pipe_helper.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    // Actually create a usable pipeline
    pipe_helper.pipe_ms_state_ci_.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    pipe_helper.CreateGraphicsPipeline();

    VkRenderingAttachmentInfoKHR color_attachment = LvlInitStruct<VkRenderingAttachmentInfoKHR>();
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    VkRenderingInfoKHR begin_rendering_info = LvlInitStruct<VkRenderingInfoKHR>(&ms_render_to_ss);
    begin_rendering_info.layerCount = 1;
    begin_rendering_info.colorAttachmentCount = 1;
    begin_rendering_info.pColorAttachments = &color_attachment;

    m_commandBuffer->begin();
    m_commandBuffer->BeginRendering(begin_rendering_info);
    // ms_render_to_ss.rasterizationSamples != ms_state.rasterizationSamples
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-pipeline-06856");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_helper.pipeline_);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRendering();
    m_commandBuffer->end();

    VkImageFormatProperties2 image_format_prop = LvlInitStruct<VkImageFormatProperties2>();
    VkPhysicalDeviceImageFormatInfo2 image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>();
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.type = VK_IMAGE_TYPE_2D;
    image_format_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_format_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    VkResult result = vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);
    if ((result != VK_SUCCESS) || !(image_format_prop.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_2_BIT)) {
        GTEST_SKIP() << "Cannot create an image with format VK_FORMAT_B8G8R8A8_UNORM and sample count VK_SAMPLE_COUNT_2_BIT. Skipping remainder of the test";
    }

    VkImageCreateInfo image_create_info = LvlInitStruct<VkImageCreateInfo>();
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent = {64, 64, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageObj two_count_image(m_device);
    two_count_image.init(&image_create_info);

    vk_testing::ImageView two_count_image_view;
    auto image_view_ci = two_count_image.TargetViewCI(VK_FORMAT_B8G8R8A8_UNORM);
    image_view_ci.image = two_count_image.handle();
    two_count_image_view.init(*m_device, image_view_ci);

    color_attachment.imageView = two_count_image_view.handle();
    m_commandBuffer->begin();
    // Attachments must have a sample count that is either VK_SAMPLE_COUNT_1_BIT or
    // VkMultisampledRenderToSingleSampledInfoEXT::rasterizationSamples.
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06858");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageObj one_count_image(m_device);
    one_count_image.init(&image_create_info);
    vk_testing::ImageView one_count_image_view;
    auto one_count_image_view_ci = one_count_image.TargetViewCI(VK_FORMAT_B8G8R8A8_UNORM);
    one_count_image_view_ci.image = one_count_image.handle();
    one_count_image_view.init(*m_device, one_count_image_view_ci);
    color_attachment.imageView = one_count_image_view.handle();
    // Attachments with a sample count of VK_SAMPLE_COUNT_1_BIT must have been created with
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06859");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    VkImageObj good_one_count_image(m_device);
    image_create_info.flags = VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
    good_one_count_image.init(&image_create_info);
    vk_testing::ImageView good_one_count_image_view;
    auto good_one_count_image_view_ci = good_one_count_image.TargetViewCI(VK_FORMAT_B8G8R8A8_UNORM);
    good_one_count_image_view_ci.image = good_one_count_image.handle();
    good_one_count_image_view.init(*m_device, good_one_count_image_view_ci);
    color_attachment.imageView = good_one_count_image_view.handle();
    color_attachment.resolveImageView = good_one_count_image_view.handle();
    color_attachment.resolveImageLayout = VK_IMAGE_LAYOUT_GENERAL;
    begin_rendering_info.pNext = nullptr;
    color_attachment.imageView = good_one_count_image_view.handle();
    // If resolveMode is not VK_RESOLVE_MODE_NONE, imageView must not have a sample count of VK_SAMPLE_COUNT_1_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06861");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    color_attachment.imageView = two_count_image_view.handle();
    color_attachment.resolveImageView = VK_NULL_HANDLE;
    // Image samples can't be COUNT_1 or we get 06861, and has to be COUNT_1, or we get 06858, use COUNT_2 and expect 06858
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingInfo-imageView-06858");
    // If resolveMode is not VK_RESOLVE_MODE_NONE, resolveImageView must not be VK_NULL_HANDLE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06862");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    begin_rendering_info.pNext = &ms_render_to_ss;
    color_attachment.imageView = good_one_count_image_view.handle();
    color_attachment.resolveImageView = good_one_count_image_view.handle();
    // If imageView has a sample count of VK_SAMPLE_COUNT_1_BIT, resolveImageView must be VK_NULL_HANDLE
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderingAttachmentInfo-imageView-06863");
    m_commandBuffer->BeginRendering(begin_rendering_info);
    m_errorMonitor->VerifyFound();

    // Find an image format that can't be sampled
    image_format_prop = LvlInitStruct<VkImageFormatProperties2>();
    image_format_info = LvlInitStruct<VkPhysicalDeviceImageFormatInfo2>();
    image_format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_format_info.type = VK_IMAGE_TYPE_3D;
    image_format_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkFormat unsampleable_format = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits unsampleable_count = VK_SAMPLE_COUNT_1_BIT;
    for (VkFormat format = VK_FORMAT_UNDEFINED; format <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; format = VkFormat(format + 1)) {
        image_format_info.format = format;
        result = vk::GetPhysicalDeviceImageFormatProperties2(m_device->phy().handle(), &image_format_info, &image_format_prop);
        if (result == VK_SUCCESS) {
            if (image_format_prop.imageFormatProperties.sampleCounts != 0x7f) {
                unsampleable_format = format;
                for (VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT; samples <= VK_SAMPLE_COUNT_64_BIT;
                     samples = VkSampleCountFlagBits(samples << 1)) {
                    if (!(image_format_prop.imageFormatProperties.sampleCounts & samples)) {
                        unsampleable_count = samples;
                        break;
                    }
                }
                break;
            }
        }
    }

    if (unsampleable_format != VK_FORMAT_UNDEFINED) {
        image_create_info.imageType = VK_IMAGE_TYPE_3D;
        image_create_info.format = unsampleable_format;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;  // Can't use unsupported sample count or can't create image view
        image_create_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.flags =
            VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT | VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        VkImageObj unsampleable_image(m_device);
        unsampleable_image.init(&image_create_info);
        vk_testing::ImageView unsampleable_image_view;
        auto unsampleable_image_view_ci = unsampleable_image.TargetViewCI(unsampleable_format);
        unsampleable_image_view_ci.image = unsampleable_image.handle();
        unsampleable_image_view_ci.subresourceRange.baseMipLevel = 0;
        unsampleable_image_view_ci.subresourceRange.levelCount = 1;
        unsampleable_image_view.init(*m_device, unsampleable_image_view_ci);
        begin_rendering_info.pNext = &ms_render_to_ss;
        ms_render_to_ss.rasterizationSamples = unsampleable_count;
        color_attachment.resolveImageView = VK_NULL_HANDLE;
        color_attachment.imageView = unsampleable_image_view.handle();
        // Attachment must have a format that supports the sample count specified in rasterizationSamples
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMultisampledRenderToSingleSampledInfoEXT-pNext-06880");
        m_commandBuffer->BeginRendering(begin_rendering_info);
        m_errorMonitor->VerifyFound();
    }

    // Need a renderpass with a COUNT_1 attachment
    ms_render_to_ss.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
    attach_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    subpass.pDepthStencilAttachment = nullptr;
    rpci.attachmentCount = 1;
    // Create a usable renderpass
    vk_testing::RenderPass test_rp2(*m_device, rpci);
    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, test_rp2.handle(), 1, &one_count_image_view.handle(), 64, 64, 1};
    VkFramebuffer fb;
    // Framebuffer attachments with VK_SAMPLE_COUNT_1_BIT must have been created with
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-samples-06881");
    vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
    m_errorMonitor->VerifyFound();

    if (imageless_fb_supported) {
        VkFormat framebufferAttachmentFormats[1] = {VK_FORMAT_B8G8R8A8_UNORM};
        VkFramebufferAttachmentImageInfoKHR framebufferAttachmentImageInfo = LvlInitStruct<VkFramebufferAttachmentImageInfoKHR>();
        framebufferAttachmentImageInfo.flags = 0;
        framebufferAttachmentImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        framebufferAttachmentImageInfo.width = 64;
        framebufferAttachmentImageInfo.height = 64;
        framebufferAttachmentImageInfo.layerCount = 1;
        framebufferAttachmentImageInfo.viewFormatCount = 1;
        framebufferAttachmentImageInfo.pViewFormats = framebufferAttachmentFormats;
        VkFramebufferAttachmentsCreateInfoKHR framebufferAttachmentsCreateInfo =
            LvlInitStruct<VkFramebufferAttachmentsCreateInfoKHR>();
        framebufferAttachmentsCreateInfo.attachmentImageInfoCount = 1;
        framebufferAttachmentsCreateInfo.pAttachmentImageInfos = &framebufferAttachmentImageInfo;
        fbci.pNext = &framebufferAttachmentsCreateInfo;
        fbci.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkFramebufferCreateInfo-samples-06881");
        vk::CreateFramebuffer(m_device->device(), &fbci, nullptr, &fb);
        m_errorMonitor->VerifyFound();
    }

    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    VkImage bad_flag_image;
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT requires VK_SAMPLE_COUNT_1_BIT
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-flags-06883");
    vk::CreateImage(device(), &image_create_info, nullptr, &bad_flag_image);
    m_errorMonitor->VerifyFound();

    vk_testing::QueueCreateInfoArray queue_info(m_device->queue_props);
    VkDeviceCreateInfo device_create_info = LvlInitStruct<VkDeviceCreateInfo>();
    device_create_info.queueCreateInfoCount = queue_info.size();
    device_create_info.pQueueCreateInfos = queue_info.data();
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.ppEnabledExtensionNames = nullptr;

    VkDevice second_device;
    ASSERT_VK_SUCCESS(vk::CreateDevice(gpu(), &device_create_info, nullptr, &second_device));
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    bad_flag_image = VK_NULL_HANDLE;
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT requires multisampledRenderToSingleSampled feature
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkImageCreateInfo-multisampledRenderToSingleSampled-06882");
    vk::CreateImage(second_device, &image_create_info, nullptr, &bad_flag_image);
    m_errorMonitor->VerifyFound();
    vk::DestroyDevice(second_device, nullptr);
}
