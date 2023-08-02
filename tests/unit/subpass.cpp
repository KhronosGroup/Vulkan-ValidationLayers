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

TEST_F(NegativeSubpass, NonGraphicsPipeline) {
    TEST_DESCRIPTION("Create a subpass with the compute pipeline bind point");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_COMPUTE, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 0u, nullptr, 1u, subpasses, 0u, nullptr);

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2Supported, "VUID-VkSubpassDescription-pipelineBindPoint-04952",
                         "VUID-VkSubpassDescription2-pipelineBindPoint-04953");
}

TEST_F(NegativeSubpass, InputAttachmentParameters) {
    TEST_DESCRIPTION("Create a subpass with parameters in the input attachment ref which are invalid");

    // Check for VK_KHR_get_physical_device_properties2
    AddRequiredExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R32_UINT;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto reference = LvlInitStruct<VkAttachmentReference2>();
    reference.attachment = 0;
    reference.layout = VK_IMAGE_LAYOUT_GENERAL;
    reference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    auto subpass = LvlInitStruct<VkSubpassDescription2KHR>();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.viewMask = 0;
    subpass.inputAttachmentCount = 1;
    subpass.pInputAttachments = &reference;

    auto rpci2 = LvlInitStruct<VkRenderPassCreateInfo2KHR>(nullptr, 0u, 1u, &attach_desc, 1u, &subpass, 0u, nullptr, 0u, nullptr);

    // Valid
    PositiveTestRenderPass2KHRCreate(*m_device, rpci2);

    attach_desc.format = VK_FORMAT_UNDEFINED;

    reference.aspectMask = 0;
    // Test for aspect mask of 0
    m_errorMonitor->SetUnexpectedError("VUID-VkAttachmentDescription2-format-06698");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pInputAttachments-02897");
    TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, rpci2, {"VUID-VkSubpassDescription2-attachment-02800"});

    // Test for invalid aspect mask bits
    reference.aspectMask = 0x40000000;  // invalid VkImageAspectFlagBits value
    m_errorMonitor->SetUnexpectedError("VUID-VkAttachmentDescription2-format-06698");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pInputAttachments-02897");
    TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, rpci2, {"VUID-VkSubpassDescription2-attachment-02799"});

    // Test for invalid use of VK_IMAGE_ASPECT_METADATA_BIT
    reference.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetUnexpectedError("VUID-VkAttachmentDescription2-format-06698");
    m_errorMonitor->SetUnexpectedError("VUID-VkSubpassDescription2-pInputAttachments-02897");
    TestRenderPass2KHRCreate(*m_errorMonitor, *m_device, rpci2, {"VUID-VkSubpassDescription2-attachment-02801"});
}

TEST_F(NegativeSubpass, SubpassDependencies) {
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_MULTIVIEW_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    const bool multiview_supported =
        IsExtensionsEnabled(VK_KHR_MULTIVIEW_EXTENSION_NAME) || (DeviceValidationVersion() >= VK_API_VERSION_1_1);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
    }
    // Add a device features struct enabling NO features
    features2.features = {};
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));

    // Create two dummy subpasses
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
    };

    VkSubpassDependency dependency;
    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 0u, nullptr, 2u, subpasses, 1u, &dependency);

    // Non graphics stages in subpass dependency
    dependency = {0, 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00837",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00837",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00837",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00838",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03055");

    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00838",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03055");

    dependency = {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00837",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    dependency = {VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00838",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03055");

    dependency = {0, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-00837",
                         "VUID-VkRenderPassCreateInfo2-pDependencies-03054");

    // Geometry shaders not enabled source
    dependency = {0, 1, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcStageMask-04090",
                         "VUID-VkSubpassDependency2-srcStageMask-04090");

    // Geometry shaders not enabled destination
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-dstStageMask-04090",
                         "VUID-VkSubpassDependency2-dstStageMask-04090");

    // Tessellation not enabled source
    dependency = {0, 1, VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcStageMask-04091",
                         "VUID-VkSubpassDependency2-srcStageMask-04091");

    // Tessellation not enabled destination
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-dstStageMask-04091",
                         "VUID-VkSubpassDependency2-dstStageMask-04091");

    // Potential cyclical dependency
    dependency = {1, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00864",
                         "VUID-VkSubpassDependency2-srcSubpass-03084");

    // EXTERNAL to EXTERNAL dependency
    dependency = {
        VK_SUBPASS_EXTERNAL, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00865",
                         "VUID-VkSubpassDependency2-srcSubpass-03085");

    // srcStage contains framebuffer space, and dstStage contains non-framebuffer space
    dependency = {0,
                  0,
                  VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                  VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                  0,
                  0,
                  0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-06809",
                         "VUID-VkSubpassDependency2-srcSubpass-06810");

    // framebuffer space stages in self dependency with region bit
    dependency = {0, 0, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-02243",
                         "VUID-VkSubpassDependency2-srcSubpass-02245");

    // Same test but make sure the logical invalid order does not trip other VUID since both are framebuffer space stages
    dependency = {0, 0, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-02243",
                         "VUID-VkSubpassDependency2-srcSubpass-02245");

    // Source access mask mismatch with source stage mask
    dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_UNIFORM_READ_BIT, 0, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcAccessMask-00868",
                         "VUID-VkSubpassDependency2-srcAccessMask-03088");

    // Destination access mask mismatch with destination stage mask
    dependency = {
        0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0};

    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-dstAccessMask-00869",
                         "VUID-VkSubpassDependency2-dstAccessMask-03089");

    // srcSubpass larger than subpassCount
    dependency = {3, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-06866",
                         "VUID-VkRenderPassCreateInfo2-srcSubpass-02526");

    // dstSubpass larger than subpassCount
    dependency = {0, 3, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, 0};
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkRenderPassCreateInfo-pDependencies-06867",
                         "VUID-VkRenderPassCreateInfo2-dstSubpass-02527");

    if (multiview_supported) {
        // VIEW_LOCAL_BIT but multiview is not enabled
        dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, nullptr,
                             "VUID-VkRenderPassCreateInfo2-viewMask-03059");

        // Enable multiview
        uint32_t pViewMasks[2] = {0x3u, 0x3u};
        int32_t pViewOffsets[2] = {0, 0};
        auto rpmvci = LvlInitStruct<VkRenderPassMultiviewCreateInfo>(nullptr, 2u, pViewMasks, 0u, nullptr, 0u, nullptr);
        rpci.pNext = &rpmvci;

        // Excessive view offsets
        dependency = {0, 1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};
        rpmvci.pViewOffsets = pViewOffsets;
        rpmvci.dependencyCount = 2;

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01929", nullptr);

        rpmvci.dependencyCount = 0;

        // View offset with subpass self dependency
        dependency = {0, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      0, 0, VK_DEPENDENCY_VIEW_LOCAL_BIT};
        rpmvci.pViewOffsets = pViewOffsets;
        pViewOffsets[0] = 1;
        rpmvci.dependencyCount = 1;

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01930",
                             "VUID-VkSubpassDependency2-viewOffset-02530");

        rpmvci.dependencyCount = 0;

        // View offset with no view local bit
        if (rp2_supported) {
            dependency = {0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};
            rpmvci.pViewOffsets = pViewOffsets;
            pViewOffsets[0] = 1;
            rpmvci.dependencyCount = 1;

            TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, nullptr,
                                 "VUID-VkSubpassDependency2-dependencyFlags-03092");

            rpmvci.dependencyCount = 0;
        }

        // EXTERNAL subpass with VIEW_LOCAL_BIT - source subpass
        dependency = {VK_SUBPASS_EXTERNAL,         1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                      VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-dependencyFlags-02520",
                             "VUID-VkSubpassDependency2-dependencyFlags-03090");

        // EXTERNAL subpass with VIEW_LOCAL_BIT - destination subpass
        dependency = {0, VK_SUBPASS_EXTERNAL,         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
                      0, VK_DEPENDENCY_VIEW_LOCAL_BIT};

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-dependencyFlags-02521",
                             "VUID-VkSubpassDependency2-dependencyFlags-03091");

        // Multiple views but no view local bit in self-dependency
        dependency = {0, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, 0};

        TestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported, "VUID-VkSubpassDependency-srcSubpass-00872",
                             "VUID-VkRenderPassCreateInfo2-pDependencies-03060");
    }
}

TEST_F(NegativeSubpass, NextSubpassExcessive) {
    TEST_DESCRIPTION("Test that an error is produced when CmdNextSubpass is called too many times in a renderpass instance");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdNextSubpass-None-00909");
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        auto subpassBeginInfo = LvlInitStruct<VkSubpassBeginInfoKHR>(nullptr, VK_SUBPASS_CONTENTS_INLINE);
        auto subpassEndInfo = LvlInitStruct<VkSubpassEndInfoKHR>();

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdNextSubpass2-None-03102");

        vk::CmdNextSubpass2KHR(m_commandBuffer->handle(), &subpassBeginInfo, &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeSubpass, RenderPassEndBeforeFinalSubpass) {
    TEST_DESCRIPTION("Test that an error is produced when CmdEndRenderPass is called before the final subpass has been reached");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    const bool rp2Supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));

    VkSubpassDescription sd[2] = {{0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr},
                                  {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, nullptr}};

    auto rcpi = LvlInitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 0u, nullptr, 2u, sd, 0u, nullptr);

    vk_testing::RenderPass rp(*m_device, rcpi);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>(nullptr, 0u, rp.handle(), 0u, nullptr, 16u, 16u, 1u);

    vk_testing::Framebuffer fb(*m_device, fbci);

    m_commandBuffer->begin();

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {16u, 16u}}, 0u, nullptr);

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass-None-00910");
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_errorMonitor->VerifyFound();

    if (rp2Supported) {
        auto subpassEndInfo = LvlInitStruct<VkSubpassEndInfoKHR>();

        m_commandBuffer->reset();
        m_commandBuffer->begin();
        vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass2-None-03103");
        vk::CmdEndRenderPass2KHR(m_commandBuffer->handle(), &subpassEndInfo);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSubpass, SubpassIndices) {
    TEST_DESCRIPTION("Create render pass with valid stages");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
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

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
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
        safe_VkRenderPassCreateInfo2 create_info2 = ConvertVkRenderPassCreateInfoToV2KHR(rpci);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-srcSubpass-02526");
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkRenderPassCreateInfo2-dstSubpass-02527");
        vk::CreateRenderPass2KHR(m_device->device(), create_info2.ptr(), nullptr, &render_pass);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSubpass, DrawWithPipelineIncompatibleWithSubpass) {
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
    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 1u, attach, 2u, subpasses, 1u, &dep);
    vk_testing::RenderPass rp(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>(nullptr, 0u, rp.handle(), 1u, &imageView, 32u, 32u, 1u);
    vk_testing::Framebuffer fb(*m_device, fbci);

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT);
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
    pipe.CreateVKPipeline(pl.handle(), rp.handle());

    m_commandBuffer->begin();

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

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
}

TEST_F(NegativeSubpass, ImageBarrierSubpassConflict) {
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

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(nullptr, 0u, 2u, attach, 1u, subpasses, 1u, &dep);
    vk_testing::RenderPass rp(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    VkImageObj image2(m_device);
    image2.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView2 = image2.targetView(VK_FORMAT_R8G8B8A8_UNORM);
    // re-use imageView from start of test
    VkImageView iv_array[2] = {imageView, imageView2};

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>(nullptr, 0u, rp.handle(), 2u, iv_array, 32u, 32u, 1u);
    vk_testing::Framebuffer fb(*m_device, fbci);

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {32u, 32u}}, 0u, nullptr);

    auto img_barrier = LvlInitStruct<VkImageMemoryBarrier>();
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
}

TEST_F(NegativeSubpass, SubpassInputNotBoundDescriptorSet) {
    TEST_DESCRIPTION("Validate subpass input isn't bound to fragment shader or descriptor set");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkImageUsageFlags usage_input =
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageObj image_input(m_device);
    auto image_ci = VkImageObj::ImageCreateInfo2D(64, 64, 1, 1, format, usage_input, VK_IMAGE_TILING_OPTIMAL);
    image_input.Init(image_ci);
    image_input.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkImageView view_input = image_input.targetView(format);

    const VkAttachmentDescription inputAttachment = {
        0u,
        format,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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
        0u,      VK_PIPELINE_BIND_POINT_GRAPHICS, size32(inputAttachments), inputAttachments.data(), 0, nullptr, 0u, nullptr, 0u,
        nullptr,
    };
    const std::vector<VkSubpassDescription> subpasses(1u, subpass);

    const auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(nullptr, 0u, size32(attachmentDescs), attachmentDescs.data(),
                                                            size32(subpasses), subpasses.data(), 0u, nullptr);
    vk_testing::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = rp.handle();
    fbci.attachmentCount = 1u;
    fbci.pAttachments = &view_input;
    fbci.width = 64;
    fbci.height = 64;
    fbci.layers = 1u;
    vk_testing::Framebuffer fb(*m_device, fbci);
    ASSERT_TRUE(fb.initialized());

    VkSamplerCreateInfo sampler_info = SafeSaneSamplerCreateInfo();
    vk_testing::Sampler sampler(*m_device, sampler_info);
    ASSERT_TRUE(sampler.initialized());

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    {
        // input index is wrong, it doesn't exist in supbass input attachments and the set and binding is undefined
        // It causes desired failures.
        char const *fsSource_fail = R"glsl(
            #version 450
            layout(input_attachment_index=1, set=0, binding=1) uniform subpassInput x;
            void main() {
            vec4 color = subpassLoad(x);
            }
        )glsl";

        VkShaderObj fs_fail(this, fsSource_fail, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper g_pipe(*this);
        g_pipe.InitInfo();
        g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs_fail.GetStageCreateInfo()};
        g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        g_pipe.gp_ci_.renderPass = rp.handle();
        g_pipe.InitState();

        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06038");
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-VkGraphicsPipelineCreateInfo-layout-07988");
        g_pipe.CreateGraphicsPipeline();
        m_errorMonitor->VerifyFound();
    }

    {  // Binds input attachment
        char const *fsSource =
            "#version 450\n"
            "layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;\n"
            "void main() {\n"
            "   vec4 color = subpassLoad(x);\n"
            "}\n";
        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        CreatePipelineHelper g_pipe(*this);
        g_pipe.InitInfo();
        g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
        g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        g_pipe.gp_ci_.renderPass = rp.handle();
        g_pipe.InitState();
        ASSERT_VK_SUCCESS(g_pipe.CreateGraphicsPipeline());

        g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_input, sampler.handle(), VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        g_pipe.descriptor_set_->UpdateDescriptorSets();

        m_commandBuffer->begin();

        image_input.SetLayout(m_commandBuffer, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        m_renderPassBeginInfo.renderArea = {{0, 0}, {64, 64}};
        m_renderPassBeginInfo.renderPass = rp.handle();
        m_renderPassBeginInfo.framebuffer = fb.handle();

        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0,
                                  1, &g_pipe.descriptor_set_->set_, 0, nullptr);

        vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }
}

TEST_F(NegativeSubpass, SubpassDescriptionViewMask) {
    TEST_DESCRIPTION("Test creating render with invalid view mask bit");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    auto multiview_features = LvlInitStruct<VkPhysicalDeviceMultiviewFeatures>();
    auto features2 = GetPhysicalDeviceFeatures2(multiview_features);
    if (multiview_features.multiview == VK_FALSE) {
        GTEST_SKIP() << "multiview feature not supported";
    }

    auto render_pass_multiview_props = LvlInitStruct<VkPhysicalDeviceMultiviewProperties>();
    GetPhysicalDeviceProperties2(render_pass_multiview_props);

    if (render_pass_multiview_props.maxMultiviewViewCount >= 32) {
        GTEST_SKIP() << "maxMultiviewViewCount too high";
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
                                                            // nullptr, 0, nullptr};
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

TEST_F(NegativeSubpass, PipelineSubpassIndex) {
    TEST_DESCRIPTION("Test using pipeline with incompatible subpass index for current renderpass subpass");

    AddRequiredExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());

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

    auto render_pass_ci = LvlInitStruct<VkRenderPassCreateInfo>();
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

    auto framebuffer_ci = LvlInitStruct<VkFramebufferCreateInfo>();
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

    auto render_pass_bi = LvlInitStruct<VkRenderPassBeginInfo>();
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

TEST_F(NegativeSubpass, SubpassDependencyMasksSync2) {
    // Testing from the spec:
    // If a VkMemoryBarrier2 is included in the pNext chain,
    // srcStageMask, dstStageMask, srcAccessMask, and dstAccessMask parameters are ignored.
    // The synchronization and access scopes instead are defined by the parameters of VkMemoryBarrier2.
    SetTargetApiVersion(VK_API_VERSION_1_2);  // VK_KHR_create_renderpass2
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2FeaturesKHR>();
    GetPhysicalDeviceFeatures2(sync2_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &sync2_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto attach_ref = LvlInitStruct<VkAttachmentReference2>();
    attach_ref.attachment = 0;
    attach_ref.layout = VK_IMAGE_LAYOUT_GENERAL;
    auto subpass = LvlInitStruct<VkSubpassDescription2>();
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attach_ref;
    subpass.viewMask = 0;

    auto attach_desc = LvlInitStruct<VkAttachmentDescription2>();
    attach_desc.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attach_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach_desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    auto mem_barrier = LvlInitStruct<VkMemoryBarrier2>();
    mem_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    mem_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    mem_barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    auto dependency = LvlInitStruct<VkSubpassDependency2>();
    dependency.srcSubpass = 0;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = 0X8000000;  // not real value, VK_PIPELINE_STAGE_VIDEO_ENCODE_BIT_KHR doesn't exist
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependency.viewOffset = 0;

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo2>();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &attach_desc;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dependency;

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkSubpassDependency2-srcStageMask-parameter");
        vk_testing::RenderPass render_pass(*m_device, rpci);
        m_errorMonitor->VerifyFound();
    }

    dependency.pNext = &mem_barrier;  // srcStageMask should be ignored now
    { vk_testing::RenderPass render_pass(*m_device, rpci); }

    mem_barrier.srcStageMask = 0x8000000000000000ULL;  // not real value
    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkMemoryBarrier2-srcStageMask-parameter");
        vk_testing::RenderPass render_pass(*m_device, rpci);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSubpass, InputAttachmentReferences) {
    TEST_DESCRIPTION("Create a subpass with the meta data aspect mask set for an input attachment");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
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
    auto rpiaaci = LvlInitStruct<VkRenderPassInputAttachmentAspectCreateInfo>(nullptr, 1u, &iaar);

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(&rpiaaci, 0u, 1u, &attach, 1u, &subpass, 0u, nullptr);

    // Invalid aspect masks
    // Cannot/should not avoid getting the unxpected ones too
    iaar.aspectMask = VK_IMAGE_ASPECT_METADATA_BIT;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo-pNext-01963");
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964", nullptr);

    iaar.aspectMask = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo-pNext-01963");
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-02250", nullptr);

    // Aspect not present
    iaar.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01963",
                         "VUID-VkRenderPassCreateInfo2-attachment-02525");

    // Invalid subpass index
    iaar.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iaar.subpass = 1;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01926", nullptr);
    iaar.subpass = 0;

    // Invalid input attachment index
    iaar.inputAttachmentIndex = 1;
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkRenderPassCreateInfo-pNext-01927", nullptr);
}

TEST_F(NegativeSubpass, InputAttachmentLayout) {
    TEST_DESCRIPTION("Create renderpass where an input attachment is also uses as another type");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddOptionalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    const bool rp2_supported = IsExtensionsEnabled(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
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

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>(nullptr, 0u, size32(attachs), attachs.data(), size32(subpasses),
                                                      subpasses.data(), size32(deps), deps.data());

    // Current setup should be OK -- no attachment is both input and output in same subpass
    PositiveTestRenderPassCreate(m_errorMonitor, *m_device, rpci, rp2_supported);

    vk_testing::RenderPass render_pass(*m_device, rpci);
}

TEST_F(NegativeSubpass, InputAttachmentMissing) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming an input attachment which is not included in the subpass "
        "description");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput x;
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(x);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06038");
}

TEST_F(NegativeSubpass, InputAttachmentMissingArray) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a shader consuming an input attachment which is not included in the subpass "
        "description -- array case");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *fsSource = R"glsl(
        #version 450
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[1];
        layout(location=0) out vec4 color;
        void main() {
           color = subpassLoad(xs[0]);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06038");
}

TEST_F(NegativeSubpass, InputAttachmentSharingVariable) {
    TEST_DESCRIPTION("Make sure if 2 loads use same variable, both are tracked");

    ASSERT_NO_FATAL_FAILURE(Init());

    const VkAttachmentDescription inputAttachmentDescription = {0,
                                                                m_render_target_fmt,
                                                                VK_SAMPLE_COUNT_1_BIT,
                                                                VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                VK_ATTACHMENT_STORE_OP_STORE,
                                                                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                VK_IMAGE_LAYOUT_GENERAL,
                                                                VK_IMAGE_LAYOUT_GENERAL};

    // index 0 is unused
    // index 1 is is valid (for both color and input)
    const VkAttachmentReference inputAttachmentReferences[2] = {{VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_GENERAL},
                                                                {0, VK_IMAGE_LAYOUT_GENERAL}};

    const VkSubpassDescription subpassDescription = {(VkSubpassDescriptionFlags)0,
                                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                     2,
                                                     inputAttachmentReferences,
                                                     1,
                                                     &inputAttachmentReferences[1],
                                                     nullptr,
                                                     nullptr,
                                                     0,
                                                     nullptr};

    auto renderPassInfo = LvlInitStruct<VkRenderPassCreateInfo>();
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &inputAttachmentDescription;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;

    vk_testing::RenderPass renderPass(*m_device, renderPassInfo);

    // There are 2 OpLoad/OpAccessChain that point the same OpVariable
    // Make sure we are not just taking the first load and checking all loads on a variable
    const char *fs_source = R"(
        #version 460
        layout(input_attachment_index=0, set=0, binding=0) uniform subpassInput xs[2];
        layout(location=0) out vec4 color;
        void main() {
            color = subpassLoad(xs[1]); // valid
            color = subpassLoad(xs[0]); // invalid
        }
    )";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        helper.gp_ci_.renderPass = renderPass.handle();
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-renderPass-06038");
}

TEST_F(NegativeSubpass, SubpassInputWithoutFormat) {
    TEST_DESCRIPTION("Non-InputAttachment shader input with unknown image format");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    VkPhysicalDeviceFeatures features;
    vk::GetPhysicalDeviceFeatures(gpu(), &features);
    features.shaderStorageImageReadWithoutFormat = VK_FALSE;
    ASSERT_NO_FATAL_FAILURE(InitState(&features));

    if (DeviceExtensionSupported(gpu(), nullptr, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)) {
        GTEST_SKIP() << "VK_KHR_format_feature_flags2 is supported";
    }

    const std::string fs_source = R"(
               OpCapability Shader
               OpCapability StorageImageReadWithoutFormat
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color %img
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpName %main "main"
               OpName %color "color"
               OpName %img "img"
               OpDecorate %color Location 0
               OpDecorate %img DescriptorSet 0
               OpDecorate %img Binding 0
               OpDecorate %img NonWritable
               OpDecorate %img NonReadable
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
               ;
               ; Image has unknown format, but dimension != SubpassData and
               ; shaderStorageImageReadWithoutFormat == VK_FALSE, which is invalid
               ;
         %10 = OpTypeImage %float 2D 0 0 0 2 Unknown

%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
        %img = OpVariable %_ptr_UniformConstant_10 UniformConstant
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %17 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %img
         %18 = OpImageRead %v4float %13 %17
               OpStore %color %18
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fs_source.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2, SPV_SOURCE_ASM);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddShader(&fs);
    pipe.AddDefaultColorAttachment();
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkDescriptorSetLayoutBinding dslb = {0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    const VkDescriptorSetLayoutObj dsl(m_device, {dslb});
    const VkPipelineLayoutObj pl(m_device, {&dsl});

    VkAttachmentDescription descs[2] = {
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {0, VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL},
    };
    VkAttachmentReference color = {
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference input = {
        1,
        VK_IMAGE_LAYOUT_GENERAL,
    };

    VkSubpassDescription sd = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &input, 1, &color, nullptr, nullptr, 0, nullptr};

    VkRenderPassCreateInfo rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.flags = 0;
    rpci.attachmentCount = 2;
    rpci.pAttachments = descs;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &sd;
    rpci.dependencyCount = 0;
    vk_testing::RenderPass rp(*m_device, rpci);
    ASSERT_TRUE(rp.initialized());

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkShaderModuleCreateInfo-pCode-08740");
    pipe.CreateVKPipeline(pl.handle(), rp.handle());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSubpass, NextSubpassNoRenderPass) {
    TEST_DESCRIPTION("call next subpass outside a renderpass");
    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdNextSubpass-renderpass");
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    m_commandBuffer->EndRenderPass();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdNextSubpass-renderpass");
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}
