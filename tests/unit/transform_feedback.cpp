/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 * Modifications Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"

void NegativeTransformFeedback::InitBasicTransformFeedback(void *pNextFeatures) {
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < m_attempted_api_version) {
        GTEST_SKIP() << "At least Vulkan version 1." << m_attempted_api_version.minor() << " is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto tf_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(pNextFeatures);
    GetPhysicalDeviceFeatures2(tf_features);
    if (tf_features.transformFeedback == VK_FALSE) {
        GTEST_SKIP() << "transformFeedback not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &tf_features, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
}

TEST_F(NegativeTransformFeedback, FeatureEnabled) {
    TEST_DESCRIPTION("VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback must be enabled");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // transformFeedback not enabled
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 4;
        VkBufferObj buffer;
        buffer.init(*m_device, info);
        VkDeviceSize offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-transformFeedback-02355");
        vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-transformFeedback-02366");
        vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
    }

    {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-transformFeedback-02374");
        m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndTransformFeedbackEXT-None-02375");
        vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTransformFeedback, NoBoundPipeline) {
    TEST_DESCRIPTION("Call vkCmdBeginTransformFeedbackEXT without a bound pipeline");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-None-06233");
    vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeTransformFeedback, CmdBindTransformFeedbackBuffersEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdBindTransformFeedbackBuffersEXT");
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "Test temporarily disabled on S10 device";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    {
        auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
        GetPhysicalDeviceProperties2(tf_properties);

        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 8;
        VkBufferObj const buffer_obj(*m_device, info);

        // Request a firstBinding that is too large.
        {
            auto const firstBinding = tf_properties.maxTransformFeedbackBuffers;
            VkDeviceSize const offsets[1]{};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02356");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), firstBinding, 1, &buffer_obj.handle(), offsets,
                                                   nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request too many bindings.
        if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
            auto const bindingCount = tf_properties.maxTransformFeedbackBuffers + 1;
            std::vector<VkBuffer> buffers(bindingCount, buffer_obj.handle());

            std::vector<VkDeviceSize> offsets(bindingCount);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-firstBinding-02357");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, bindingCount, buffers.data(), offsets.data(),
                                                   nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request a size that is larger than the maximum size.
        if (tf_properties.maxTransformFeedbackBufferSize < std::numeric_limits<VkDeviceSize>::max()) {
            VkDeviceSize const offsets[1]{};
            VkDeviceSize const sizes[1]{tf_properties.maxTransformFeedbackBufferSize + 1};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSize-02361");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }
    }

    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 8;
        VkBufferObj const buffer_obj(*m_device, info);

        // Request an offset that is too large.
        {
            VkDeviceSize const offsets[1]{info.size + 4};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02358");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request an offset that is not a multiple of 4.
        {
            VkDeviceSize const offsets[1]{1};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02359");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request a size that is larger than the buffer's size.
        {
            VkDeviceSize const offsets[1]{};
            VkDeviceSize const sizes[1]{info.size + 1};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pSizes-02362");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }

        // Request an offset and size whose sum is larger than the buffer's size.
        {
            VkDeviceSize const offsets[1]{4};
            VkDeviceSize const sizes[1]{info.size - 3};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pOffsets-02363");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, sizes);
            m_errorMonitor->VerifyFound();
        }

        // Bind while transform feedback is active.
        if (!IsDriver(VK_DRIVER_ID_MESA_RADV)) {
            vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

            VkDeviceSize const offsets[1]{};

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-None-02365");
            vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
            m_errorMonitor->VerifyFound();

            vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        }
    }

    // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT.
    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        // info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
        info.size = 4;
        m_errorMonitor->SetUnexpectedError("VUID-VkBufferCreateInfo-usage-requiredbitmask");
        VkBufferObj const buffer_obj(*m_device, info);

        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02360");
        vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }

    // Don't bind memory.
    {
        vk_testing::Buffer buffer;
        {
            auto info = LvlInitStruct<VkBufferCreateInfo>();
            info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
            info.size = 4;
            buffer.init_no_mem(*m_device, info);
        }

        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindTransformFeedbackBuffersEXT-pBuffers-02364");
        vk::CmdBindTransformFeedbackBuffersEXT(m_commandBuffer->handle(), 0, 1, &buffer.handle(), offsets, nullptr);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeTransformFeedback, CmdBeginTransformFeedbackEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdBeginTransformFeedbackEXT");
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "Test temporarily disabled on S10 device";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    {
        auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
        GetPhysicalDeviceProperties2(tf_properties);

        // Request a firstCounterBuffer that is too large.
        {
            auto const firstCounterBuffer = tf_properties.maxTransformFeedbackBuffers;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02368");
            m_errorMonitor->SetUnexpectedError("VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369");
            vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), firstCounterBuffer, 1, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }

        // Request too many buffers.
        if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
            auto const counterBufferCount = tf_properties.maxTransformFeedbackBuffers + 1;

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-firstCounterBuffer-02369");
            vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, counterBufferCount, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }
    }

    // Request an out-of-bounds location.
    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
        info.size = 4;
        VkBufferObj const buffer_obj(*m_device, info);

        VkDeviceSize const offsets[1]{1};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBufferOffsets-02370");
        vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets);
        m_errorMonitor->VerifyFound();
    }

    // Request specific offsets without specifying buffers.
    {
        VkDeviceSize const offsets[1]{};

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffer-02371");
        vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, offsets);
        m_errorMonitor->VerifyFound();
    }

    // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT.
    {
        auto info = LvlInitStruct<VkBufferCreateInfo>();
        // info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
        info.size = 4;
        m_errorMonitor->SetUnexpectedError("VUID-VkBufferCreateInfo-usage-requiredbitmask");
        VkBufferObj const buffer_obj(*m_device, info);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-pCounterBuffers-02372");
        vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), nullptr);
        m_errorMonitor->VerifyFound();
    }

    // Begin while transform feedback is active.
    if (!IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-None-02367");
        vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
        m_errorMonitor->VerifyFound();

        vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    }
}

TEST_F(NegativeTransformFeedback, CmdEndTransformFeedbackEXT) {
    TEST_DESCRIPTION("Submit invalid arguments to vkCmdEndTransformFeedbackEXT");
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    if (IsPlatform(kGalaxyS10)) {
        GTEST_SKIP() << "Test temporarily disabled on S10 device";
    }
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    if (!IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        {
            // Activate transform feedback.
            vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

            {
                auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
                GetPhysicalDeviceProperties2(tf_properties);

                // Request a firstCounterBuffer that is too large.
                {
                    auto const firstCounterBuffer = tf_properties.maxTransformFeedbackBuffers;

                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02376");
                    m_errorMonitor->SetUnexpectedError("VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377");
                    vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), firstCounterBuffer, 1, nullptr, nullptr);
                    m_errorMonitor->VerifyFound();
                }

                // Request too many buffers.
                if (tf_properties.maxTransformFeedbackBuffers < std::numeric_limits<uint32_t>::max()) {
                    auto const counterBufferCount = tf_properties.maxTransformFeedbackBuffers + 1;

                    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-firstCounterBuffer-02377");
                    vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, counterBufferCount, nullptr, nullptr);
                    m_errorMonitor->VerifyFound();
                }
            }

            // Request an out-of-bounds location.
            {
                auto info = LvlInitStruct<VkBufferCreateInfo>();
                info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
                info.size = 4;
                VkBufferObj const buffer_obj(*m_device, info);

                VkDeviceSize const offsets[1]{1};

                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBufferOffsets-02378");
                vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets);
                m_errorMonitor->VerifyFound();
            }

            // Request specific offsets without specifying buffers.
            {
                VkDeviceSize const offsets[1]{};

                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffer-02379");
                vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, offsets);
                m_errorMonitor->VerifyFound();
            }

            // Don't set VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT.
            {
                auto info = LvlInitStruct<VkBufferCreateInfo>();
                // info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
                info.size = 4;
                m_errorMonitor->SetUnexpectedError("VUID-VkBufferCreateInfo-usage-requiredbitmask");
                VkBufferObj const buffer_obj(*m_device, info);

                m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-pCounterBuffers-02380");
                vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), nullptr);
                m_errorMonitor->VerifyFound();
            }
        }

        // End while transform feedback is inactive.
        {
            vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);

            m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndTransformFeedbackEXT-None-02375");
            vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeTransformFeedback, ExecuteSecondaryCommandBuffers) {
    TEST_DESCRIPTION("Call CmdExecuteCommandBuffers when transform feedback is active");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // A pool we can reset in.
    VkCommandPoolObj pool(m_device, m_device->graphics_queue_node_index_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VkCommandBufferObj secondary(m_device, &pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

    VkCommandBufferBeginInfo info = LvlInitStruct<VkCommandBufferBeginInfo>();
    VkCommandBufferInheritanceInfo hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    info.pInheritanceInfo = &hinfo;
    hinfo.renderPass = renderPassBeginInfo().renderPass;
    hinfo.subpass = 0;
    hinfo.framebuffer = VK_NULL_HANDLE;
    hinfo.occlusionQueryEnable = VK_FALSE;
    hinfo.queryFlags = 0;
    hinfo.pipelineStatistics = 0;

    secondary.begin(&info);
    secondary.end();

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-CoreValidation-DrawState-InvalidCommandBuffer");
    vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->VerifyFound();
    // TODO - When proper VU above is added, see if 02286 is still needed
    // m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdExecuteCommands-None-02286");
    // vk::CmdExecuteCommands(m_commandBuffer->handle(), 1, &secondary.handle());
    // m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, BindPipeline) {
    TEST_DESCRIPTION("Call CmdBindPipeline when transform feedback is active");
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe_one(*this);
    pipe_one.InitInfo();
    pipe_one.InitState();
    pipe_one.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_two(*this);
    pipe_two.InitInfo();
    pipe_two.InitState();
    pipe_two.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_one.pipeline_);
    vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBindPipeline-None-02323");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_two.pipeline_);
    m_errorMonitor->VerifyFound();
    vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeTransformFeedback, EndRenderPass) {
    TEST_DESCRIPTION("Call CmdEndRenderPass when transform feedback is active");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(renderPassBeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdEndRenderPass-None-02351");
    m_commandBuffer->EndRenderPass();
    m_errorMonitor->VerifyFound();
    vk::CmdEndTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeTransformFeedback, DrawIndirectByteCountEXT) {
    TEST_DESCRIPTION("Test covered valid usage for vkCmdDrawIndirectByteCountEXT");
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto tf_properties = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    GetPhysicalDeviceProperties2(tf_properties);

    VkBufferCreateInfo buffer_create_info = LvlInitStruct<VkBufferCreateInfo>();
    buffer_create_info.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    buffer_create_info.size = 1024;
    VkBufferObj counter_buffer;
    counter_buffer.init(*m_device, buffer_create_info);

    {
        CreatePipelineHelper pipeline(*this);
        pipeline.InitInfo();
        pipeline.InitState();
        pipeline.CreateGraphicsPipeline();

        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_);

        // Greater stride than maxTransformFeedbackBufferDataStride
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-vertexStride-02289");
        vk::CmdDrawIndirectByteCountEXT(m_commandBuffer->handle(), 1, 0, counter_buffer.handle(), 0, 0,
                                        tf_properties.maxTransformFeedbackBufferDataStride + 4);
        m_errorMonitor->VerifyFound();

        // non-4 multiple stride
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-counterBufferOffset-04568");
        vk::CmdDrawIndirectByteCountEXT(m_commandBuffer->handle(), 1, 0, counter_buffer.handle(), 0, 1, 4);
        m_errorMonitor->VerifyFound();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();
    }

    std::vector<const char *> device_extension_names;
    device_extension_names.push_back(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    VkDeviceObj test_device(0, gpu(), device_extension_names);
    VkCommandPoolObj commandPool(&test_device, 0);
    VkCommandBufferObj commandBuffer(&test_device, &commandPool);
    VkBufferObj counter_buffer2;
    counter_buffer2.init(test_device, buffer_create_info);

    VkPipelineLayoutObj pipelineLayout(&test_device);

    VkRenderPassCreateInfo rp_info = LvlInitStruct<VkRenderPassCreateInfo>();
    VkSubpassDescription subpass = {};
    rp_info.pSubpasses = &subpass;
    rp_info.subpassCount = 1;
    vk_testing::RenderPass renderpass(test_device, rp_info);
    ASSERT_TRUE(renderpass.handle());

    VkPipelineObj pipeline(&test_device);
    VkShaderObj vs(this, kVertexMinimalGlsl, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
    vs.InitFromGLSLTry(false, &test_device);
    fs.InitFromGLSLTry(false, &test_device);
    pipeline.AddShader(&vs);
    pipeline.AddShader(&fs);
    pipeline.CreateVKPipeline(pipelineLayout.handle(), renderpass.handle());

    m_renderPassBeginInfo.renderPass = renderpass.handle();
    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, renderpass.handle(), 0, nullptr, 256, 256, 1};
    vk_testing::Framebuffer fb(test_device, fbci);
    ASSERT_TRUE(fb.initialized());
    m_renderPassBeginInfo.framebuffer = fb.handle();
    m_renderPassBeginInfo.renderPass = renderpass.handle();
    commandBuffer.begin();
    VkViewport viewport = {0, 0, 16, 16, 0, 1};
    vk::CmdSetViewport(commandBuffer.handle(), 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {16, 16}};
    vk::CmdSetScissor(commandBuffer.handle(), 0, 1, &scissor);
    vk::CmdBindPipeline(commandBuffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());
    commandBuffer.BeginRenderPass(m_renderPassBeginInfo);
    if (!tf_properties.transformFeedbackDraw) {
        m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-transformFeedbackDraw-02288");
    }
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawIndirectByteCountEXT-transformFeedback-02287");
    vk::CmdDrawIndirectByteCountEXT(commandBuffer.handle(), 1, 0, counter_buffer2.handle(), 0, 0, 1);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, UsingRasterizationStateStreamExtDisabled) {
    TEST_DESCRIPTION("Test using TestRasterizationStateStreamCreateInfoEXT but it doesn't enable geometryStreams.");

    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto transform_feedback_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    transform_feedback_features.geometryStreams = VK_FALSE;  // Invalid

    // Extension enabled via VK_EXT_transform_feedback dependency
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&transform_feedback_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto rasterization_state_stream_ci = LvlInitStruct<VkPipelineRasterizationStateStreamCreateInfoEXT>();
    pipe.rs_state_ci_.pNext = &rasterization_state_stream_ci;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-geometryStreams-02324");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, RuntimeSpirv) {
    TEST_DESCRIPTION("Test runtime spirv transform feedback.");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Test currently crashes with valid SPIR-V
    // Using EmitStreamVertex() with transfer_feedback_props.maxTransformFeedbackStreams
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "Test does not run on AMD proprietary driver";
    }

    VkPhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_features =
        LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    transform_feedback_features.transformFeedback = VK_TRUE;
    transform_feedback_features.geometryStreams = VK_TRUE;
    VkPhysicalDeviceVulkan12Features features12 = LvlInitStruct<VkPhysicalDeviceVulkan12Features>(&transform_feedback_features);

    auto features2 = GetPhysicalDeviceFeatures2(features12);
    if (features2.features.geometryShader == VK_FALSE) {
        GTEST_SKIP() << "Device does not support the required geometry shader features";
    }
    if (!transform_feedback_features.transformFeedback || !transform_feedback_features.geometryStreams) {
        GTEST_SKIP() << "transformFeedback or geometryStreams feature is not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto transform_feedback_props = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    GetPhysicalDeviceProperties2(transform_feedback_props);

    // seen sometimes when using profiles and will crash
    if (transform_feedback_props.maxTransformFeedbackStreams == 0) {
        GTEST_SKIP() << "maxTransformFeedbackStreams is zero";
    }

    {
        std::stringstream vsSource;
        vsSource << R"asm(
               OpCapability Shader
               OpCapability TransformFeedback
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %tf
               OpExecutionMode %main Xfb

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %8

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride )asm";
        vsSource << transform_feedback_props.maxTransformFeedbackBufferDataStride + 4;
        vsSource << R"asm(
               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )asm";

        auto vs = VkShaderObj::CreateFromASM(this, vsSource.str().c_str(), VK_SHADER_STAGE_VERTEX_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {vs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-XfbStride-06313");
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %tf
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputTriangleStrip
               OpExecutionMode %main OutputVertices 1

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %tf "tf"  ; id %10

               ; Annotations
               OpDecorate %tf Location 0
               OpDecorate %tf Stream 0
               OpDecorate %tf XfbBuffer 0
               OpDecorate %tf XfbStride 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
     %int_17 = OpConstant %int )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackStreams;
        gsSource << R"asm(
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
         %tf = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_17
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-OpEmitStreamVertex-06310");
    }

    if (transform_feedback_props.transformFeedbackStreamsLinesTriangles == VK_FALSE) {
        const char *gsSource = R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a %b
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %11
               OpName %b "b"  ; id %12

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream 0
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 4
               OpDecorate %a Offset 0
               OpDecorate %b Location 1
               OpDecorate %b Stream 0
               OpDecorate %b XfbBuffer 1
               OpDecorate %b XfbStride 4
               OpDecorate %b Offset 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output
          %b = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpEmitStreamVertex %int_1
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(this, gsSource, VK_SHADER_STAGE_GEOMETRY_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                          "VUID-RuntimeSpirv-transformFeedbackStreamsLinesTriangles-06311");
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %10

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream 0
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 20
               OpDecorate %a Offset  )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackBufferDataSize;
        gsSource << R"asm(

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        std::vector<std::string> vuids = {"VUID-RuntimeSpirv-Offset-06308"};
        if (transform_feedback_props.maxTransformFeedbackBufferDataSize + 4 >=
            transform_feedback_props.maxTransformFeedbackStreamDataSize) {
            vuids.push_back("VUID-RuntimeSpirv-XfbBuffer-06309");
        }
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, vuids);
    }

    {
        std::stringstream gsSource;
        gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main" %a
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4
               OpName %a "a"  ; id %10

               ; Annotations
               OpDecorate %a Location 0
               OpDecorate %a Stream  )asm";
        gsSource << transform_feedback_props.maxTransformFeedbackStreams;
        gsSource << R"asm(
               OpDecorate %a XfbBuffer 0
               OpDecorate %a XfbStride 4
               OpDecorate %a Offset 0

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
          %a = OpVariable %_ptr_Output_float Output

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

        auto gs = VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);

        const auto set_info = [&](CreatePipelineHelper &helper) {
            helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        };
        CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-Stream-06312");
    }

    {
        uint32_t offset = transform_feedback_props.maxTransformFeedbackBufferDataSize / 2;
        uint32_t count = transform_feedback_props.maxTransformFeedbackStreamDataSize / offset + 1;
        // Limit to 25, because we are dynamically adding variables using letters as names
        if (count < 25) {
            std::stringstream gsSource;
            gsSource << R"asm(
               OpCapability Geometry
               OpCapability TransformFeedback
               OpCapability GeometryStreams
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Geometry %main "main"
               OpExecutionMode %main Xfb
               OpExecutionMode %main Triangles
               OpExecutionMode %main Invocations 1
               OpExecutionMode %main OutputLineStrip
               OpExecutionMode %main OutputVertices 6

               ; Debug Information
               OpSource GLSL 450
               OpName %main "main"  ; id %4)asm";

            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + static_cast<char>(i);
                gsSource << "\nOpName %var" << v << " \"" << v << "\"";
            }
            gsSource << "\n; Annotations\n";

            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + static_cast<char>(i);
                gsSource << "OpDecorate %var" << v << " Location " << i << "\n";
                gsSource << "OpDecorate %var" << v << " Stream 0\n";
                gsSource << "OpDecorate %var" << v << " XfbBuffer " << i << "\n";
                gsSource << "OpDecorate %var" << v << " XfbStride 20\n";
                gsSource << "OpDecorate %var" << v << " Offset " << offset << "\n";
            }
            gsSource << R"asm(

               ; Types, variables and constants
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float)asm";

            gsSource << "\n";
            for (uint32_t i = 0; i < count; ++i) {
                char v = 'a' + static_cast<char>(i);
                gsSource << "%var" << v << " = OpVariable %_ptr_Output_float Output\n";
            }

            gsSource << R"asm(

               ; Function main
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpEmitStreamVertex %int_0
               OpReturn
               OpFunctionEnd
        )asm";

            auto gs = VkShaderObj::CreateFromASM(this, gsSource.str().c_str(), VK_SHADER_STAGE_GEOMETRY_BIT);

            const auto set_info = [&](CreatePipelineHelper &helper) {
                helper.shader_stages_ = {helper.vs_->GetStageCreateInfo(), gs->GetStageCreateInfo(),
                                         helper.fs_->GetStageCreateInfo()};
            };
            CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-XfbBuffer-06309");
        }
    }
}

TEST_F(NegativeTransformFeedback, PipelineRasterizationStateStreamCreateInfoEXT) {
    TEST_DESCRIPTION("Test using TestRasterizationStateStreamCreateInfoEXT with invalid rasterizationStream.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto transform_feedback_features = LvlInitStruct<VkPhysicalDeviceTransformFeedbackFeaturesEXT>();
    transform_feedback_features.geometryStreams = VK_TRUE;

    // Extension enabled via dependencies
    VkPhysicalDeviceFeatures2KHR features2 = LvlInitStruct<VkPhysicalDeviceFeatures2KHR>(&transform_feedback_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    auto transfer_feedback_props = LvlInitStruct<VkPhysicalDeviceTransformFeedbackPropertiesEXT>();
    GetPhysicalDeviceProperties2(transfer_feedback_props);

    if (!transfer_feedback_props.transformFeedbackRasterizationStreamSelect &&
        transfer_feedback_props.maxTransformFeedbackStreams == 0) {
        GTEST_SKIP() << "VkPhysicalDeviceTransformFeedbackPropertiesEXT::transformFeedbackRasterizationStreamSelect is 0";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto rasterization_state_stream_ci = LvlInitStruct<VkPipelineRasterizationStateStreamCreateInfoEXT>();
    rasterization_state_stream_ci.rasterizationStream = transfer_feedback_props.maxTransformFeedbackStreams;
    pipe.rs_state_ci_.pNext = &rasterization_state_stream_ci;
    pipe.InitState();

    if (transfer_feedback_props.transformFeedbackRasterizationStreamSelect) {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02325");
    } else {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                             "VUID-VkPipelineRasterizationStateStreamCreateInfoEXT-rasterizationStream-02326");
    }
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, CmdNextSubpass) {
    TEST_DESCRIPTION("Call CmdNextSubpass while transform feeback is active");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;

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

    auto rpci = LvlInitStruct<VkRenderPassCreateInfo>();
    rpci.attachmentCount = 1;
    rpci.pAttachments = attach;
    rpci.subpassCount = 2;
    rpci.pSubpasses = subpasses;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    vk_testing::RenderPass rp(*m_device, rpci);

    VkImageObj image(m_device);
    image.InitNoLayout(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView imageView = image.targetView(VK_FORMAT_R8G8B8A8_UNORM);

    auto fbci = LvlInitStruct<VkFramebufferCreateInfo>();
    fbci.renderPass = rp.handle();
    fbci.attachmentCount = 1;
    fbci.pAttachments = &imageView;
    fbci.width = 32;
    fbci.height = 32;
    fbci.layers = 1;

    vk_testing::Framebuffer fb(*m_device, fbci);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.gp_ci_.renderPass = rp.handle();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    auto rpbi = LvlInitStruct<VkRenderPassBeginInfo>();
    rpbi.renderPass = rp.handle();
    rpbi.framebuffer = fb.handle();
    rpbi.renderArea.offset = {0, 0};
    rpbi.renderArea.extent = {32, 32};

    vk::CmdBeginRenderPass(m_commandBuffer->handle(), &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, nullptr, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdNextSubpass-None-02349");
    vk::CmdNextSubpass(m_commandBuffer->handle(), VK_SUBPASS_CONTENTS_INLINE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeTransformFeedback, CmdBeginTransformFeedbackOutsideRenderPass) {
    TEST_DESCRIPTION("call vkCmdBeginTransformFeedbackEXT without renderpass");
    InitBasicTransformFeedback();
    if (::testing::Test::IsSkipped()) return;
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    auto info = LvlInitStruct<VkBufferCreateInfo>();
    info.usage = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
    info.size = 4;
    VkBufferObj const buffer_obj(*m_device, info);
    VkDeviceSize const offsets[1]{1};

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdBeginTransformFeedbackEXT-renderpass");
    vk::CmdBeginTransformFeedbackEXT(m_commandBuffer->handle(), 0, 1, &buffer_obj.handle(), offsets);
    m_errorMonitor->VerifyFound();
}
