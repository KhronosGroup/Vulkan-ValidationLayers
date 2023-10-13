/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/cast_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

const char* kEnableArmValidation = "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ARM";

class VkConstantBufferObj : public vkt::Buffer {
  public:
    VkConstantBufferObj(vkt::Device* device, VkDeviceSize size, const void* data,
                        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
        VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        init(*device, create_info(size, usage), reqs);

        void* pData = memory().map();
        memcpy(pData, data, static_cast<size_t>(size));
        memory().unmap();
    }
};

VkFramebuffer VkArmBestPracticesLayerTest::CreateFramebuffer(const uint32_t width, const uint32_t height, VkImageView image_view,
                                                             VkRenderPass renderpass) {
    VkFramebuffer framebuffer{VK_NULL_HANDLE};

    VkFramebufferCreateInfo framebuffer_create_info = vku::InitStructHelper();
    framebuffer_create_info.renderPass = renderpass;
    framebuffer_create_info.attachmentCount = 1;
    framebuffer_create_info.pAttachments = &image_view;
    framebuffer_create_info.width = width;
    framebuffer_create_info.height = height;
    framebuffer_create_info.layers = 1;

    VkResult result = vk::CreateFramebuffer(m_device->handle(), &framebuffer_create_info, nullptr, &framebuffer);
    assert(result == VK_SUCCESS);
    (void)result;

    return framebuffer;
}

std::unique_ptr<VkImageObj> VkArmBestPracticesLayerTest::CreateImage(VkFormat format, const uint32_t width, const uint32_t height,
                                                                     VkImageUsageFlags attachment_usage) {
    auto img = std::unique_ptr<VkImageObj>(new VkImageObj(m_device));
    img->Init(width, height, 1, format,
              VK_IMAGE_USAGE_SAMPLED_BIT | attachment_usage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
              VK_IMAGE_TILING_OPTIMAL);
    return img;
}

VkRenderPass VkArmBestPracticesLayerTest::CreateRenderPass(VkFormat format, VkAttachmentLoadOp load_op,
                                                           VkAttachmentStoreOp store_op) {
    VkRenderPass renderpass{VK_NULL_HANDLE};

    // Create renderpass
    VkAttachmentDescription attachment = {};
    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = load_op;
    attachment.storeOp = store_op;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo rpinf = vku::InitStructHelper();
    rpinf.attachmentCount = 1;
    rpinf.pAttachments = &attachment;
    rpinf.subpassCount = 1;
    rpinf.pSubpasses = &subpass;
    rpinf.dependencyCount = 0;
    rpinf.pDependencies = nullptr;

    VkResult result = vk::CreateRenderPass(m_device->handle(), &rpinf, nullptr, &renderpass);
    assert(result == VK_SUCCESS);
    (void)result;

    return renderpass;
}
// Tests for Arm-specific best practices

TEST_F(VkArmBestPracticesLayerTest, TooManySamples) {
    TEST_DESCRIPTION("Test for multisampled images with too many samples");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateImage-too-large-sample-count");

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.extent = {1920, 1080, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_8_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImage image = VK_NULL_HANDLE;
    vk::CreateImage(m_device->device(), &image_info, nullptr, &image);

    m_errorMonitor->VerifyFound();

    if (image) {
        vk::DestroyImage(m_device->device(), image, nullptr);
    }
}

TEST_F(VkArmBestPracticesLayerTest, NonTransientMSImage) {
    TEST_DESCRIPTION("Test for non-transient multisampled images");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateImage-non-transient-ms-image");

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.extent = {1920, 1080, 1};
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_info.samples = VK_SAMPLE_COUNT_4_BIT;
    image_info.arrayLayers = 1;
    image_info.mipLevels = 1;

    VkImage image;
    vk::CreateImage(m_device->device(), &image_info, nullptr, &image);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, SamplerCreation) {
    TEST_DESCRIPTION("Test for various checks during sampler creation");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-different-wrapping-modes");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-lod-clamping");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-lod-bias");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-border-clamp-color");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSampler-unnormalized-coordinates");

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.mipLodBias = 1.0f;
    sampler_info.unnormalizedCoordinates = VK_TRUE;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 4.0f;

    vkt::Sampler sampler(*m_device, sampler_info);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, MultisampledBlending) {
    TEST_DESCRIPTION("Test for multisampled blending");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreatePipelines-multisampled-blending");

    VkAttachmentDescription attachment{};
    attachment.samples = VK_SAMPLE_COUNT_4_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &attachment;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    vk::CreateRenderPass(m_device->device(), &rp_info, nullptr, &m_renderPass);
    m_renderPass_info = rp_info;

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;

    VkPipelineColorBlendAttachmentState blend_att = {};
    blend_att.blendEnable = VK_TRUE;
    blend_att.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo pipe_cb_state_ci = {};
    pipe_cb_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipe_cb_state_ci.attachmentCount = 1;
    pipe_cb_state_ci.pAttachments = &blend_att;

    CreatePipelineHelper pipe(*this);
    pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    pipe.cb_ci_ = pipe_cb_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, AttachmentNeedsReadback) {
    TEST_DESCRIPTION("Test for attachments that need readback");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_clear_via_load_op = false;  // Force LOAD_OP_LOAD
    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, ManySmallIndexedDrawcalls) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCmdDrawIndexed-many-small-indexed-drawcalls");

    // This test may also trigger other warnings
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");

    InitRenderTarget();

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = sizeof(uint32_t) * 3;
    buffer_ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer indexBuffer(*m_device, buffer_ci);

    VkPipelineMultisampleStateCreateInfo pipe_ms_state_ci = {};
    pipe_ms_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipe_ms_state_ci.pNext = NULL;
    pipe_ms_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipe_ms_state_ci.sampleShadingEnable = 0;
    pipe_ms_state_ci.minSampleShading = 1.0;
    pipe_ms_state_ci.pSampleMask = NULL;

    CreatePipelineHelper pipe(*this);
    pipe.pipe_ms_state_ci_ = pipe_ms_state_ci;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), indexBuffer.handle(), 0, VK_INDEX_TYPE_UINT32);

    for (int i = 0; i < 10; i++) {
        vk::CmdDrawIndexed(m_commandBuffer->handle(), 3, 1, 0, 0, 0);
    }

    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(VkArmBestPracticesLayerTest, SuboptimalDescriptorReuseTest) {
    TEST_DESCRIPTION("Test for validation warnings of potentially suboptimal re-use of descriptor set allocations");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();
    InitRenderTarget();

    VkDescriptorPoolSize ds_type_count = {};
    ds_type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    ds_type_count.descriptorCount = 6;

    VkDescriptorPoolCreateInfo ds_pool_ci = {};
    ds_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ds_pool_ci.pNext = NULL;
    ds_pool_ci.maxSets = 6;
    ds_pool_ci.poolSizeCount = 1;
    ds_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    ds_pool_ci.pPoolSizes = &ds_type_count;

    vkt::DescriptorPool ds_pool(*m_device, ds_pool_ci);

    VkDescriptorSetLayoutBinding ds_binding = {};
    ds_binding.binding = 0;
    ds_binding.descriptorCount = 1;
    ds_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    VkDescriptorSetLayoutCreateInfo ds_layout_info = {};
    ds_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ds_layout_info.bindingCount = 1;
    ds_layout_info.pBindings = &ds_binding;

    vkt::DescriptorSetLayout ds_layout(*m_device, ds_layout_info);

    auto ds_layouts = std::vector<VkDescriptorSetLayout>(ds_pool_ci.maxSets, ds_layout.handle());

    std::vector<VkDescriptorSet> descriptor_sets = {};
    descriptor_sets.resize(ds_layouts.size());

    // allocate N/2 descriptor sets
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.descriptorSetCount = descriptor_sets.size() / 2;
    alloc_info.pSetLayouts = ds_layouts.data();

    VkResult err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, descriptor_sets.data());
    ASSERT_EQ(VK_SUCCESS, err);

    // free one descriptor set
    VkDescriptorSet* ds = descriptor_sets.data();
    err = vk::FreeDescriptorSets(m_device->device(), ds_pool.handle(), 1, ds);

    // the previous allocate and free should not cause any warning
    ASSERT_EQ(VK_SUCCESS, err);

    // allocate the previously freed descriptor set
    alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = ds_pool.handle();
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = ds_layouts.data();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkAllocateDescriptorSets-suboptimal-reuse");

    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, ds);

    // this should create a validation warning, in addition to the appropriate warning message
    m_errorMonitor->VerifyFound();

    // allocate the remaining descriptor sets (N - (N/2))
    alloc_info.descriptorSetCount = descriptor_sets.size() - (descriptor_sets.size() / 2);
    err = vk::AllocateDescriptorSets(m_device->device(), &alloc_info, ds);

    // this should create no validation warnings
}

TEST_F(VkArmBestPracticesLayerTest, SparseIndexBufferTest) {
    TEST_DESCRIPTION(
        "Test for appropriate warnings to be thrown when recording an indexed draw call with sparse/non-sparse index buffers.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();
    InitRenderTarget();

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    // create a non-sparse index buffer
    std::vector<uint16_t> nonsparse_indices;
    nonsparse_indices.resize(128);
    std::generate(nonsparse_indices.begin(), nonsparse_indices.end(), [n = uint16_t(0)]() mutable { return ++n; });

    // another example of non-sparsity where the number of indices is also very small
    std::vector<uint16_t> nonsparse_indices_2 = {0, 1, 2, 3, 4, 5, 6, 7};

    // smallest possible meaningful index buffer
    std::vector<uint16_t> nonsparse_indices_3 = {0};

    // another example of non-sparsity, all the indices are the same value (42)
    std::vector<uint16_t> nonsparse_indices_4 = {};
    nonsparse_indices_4.resize(128);
    std::fill(nonsparse_indices_4.begin(), nonsparse_indices_4.end(), uint16_t(42));

    std::vector<uint16_t> sparse_indices = nonsparse_indices;
    // The buffer (0, 1, 2, ..., n) is completely un-sparse. However, if n < 0xFFFF, by adding 0xFFFF at the end, we
    // should trigger a warning due to loading all the indices in the range 0 to 0xFFFF, despite indices in the range
    // (n+1) to (0xFFFF - 1) not being used.
    sparse_indices[sparse_indices.size() - 1] = 0xFFFF;

    VkConstantBufferObj nonsparse_ibo(m_device, nonsparse_indices.size() * sizeof(uint16_t), nonsparse_indices.data(),
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkConstantBufferObj nonsparse_ibo_2(m_device, nonsparse_indices_2.size() * sizeof(uint16_t), nonsparse_indices_2.data(),
                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkConstantBufferObj nonsparse_ibo_3(m_device, nonsparse_indices_3.size() * sizeof(uint16_t), nonsparse_indices_3.data(),
                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkConstantBufferObj nonsparse_ibo_4(m_device, nonsparse_indices_4.size() * sizeof(uint16_t), nonsparse_indices_4.data(),
                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VkConstantBufferObj sparse_ibo(m_device, sparse_indices.size() * sizeof(uint16_t), sparse_indices.data(),
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    auto test_pipelines = [&](VkConstantBufferObj& ibo, size_t index_count, bool expect_error) -> void {
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.ia_ci_.primitiveRestartEnable = VK_FALSE;
        pipe.CreateGraphicsPipeline();

        // pipeline with primitive restarts enabled
        CreatePipelineHelper pr_pipe(*this);
        pr_pipe.InitState();
        pr_pipe.ia_ci_.primitiveRestartEnable = VK_TRUE;
        pr_pipe.CreateGraphicsPipeline();

        vk::ResetCommandPool(m_device->device(), m_commandPool->handle(), 0);
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);

        // the validation layer will only be able to analyse mapped memory, it's too expensive otherwise to do in the layer itself
        ibo.memory().map();
        if (expect_error) {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                                 "UNASSIGNED-BestPractices-vkCmdDrawIndexed-sparse-index-buffer");
        }
        vk::CmdDrawIndexed(m_commandBuffer->handle(), index_count, 0, 0, 0, 0);
        if (expect_error) {
            m_errorMonitor->VerifyFound();
        } else {
        }
        ibo.memory().unmap();

        if (expect_error) {
            m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                                 "UNASSIGNED-BestPractices-vkCmdDrawIndexed-sparse-index-buffer");
        }
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pr_pipe.pipeline_);
        vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);

        ibo.memory().map();
        vk::CmdDrawIndexed(m_commandBuffer->handle(), index_count, 0, 0, 0, 0);
        if (expect_error) {
            m_errorMonitor->VerifyFound();
        }
        ibo.memory().unmap();

        m_commandBuffer->EndRenderPass();
        m_commandBuffer->end();

        m_errorMonitor->Reset();
    };

    // our non-sparse indices should not trigger a warning for either pipeline in this case
    test_pipelines(nonsparse_ibo, nonsparse_indices.size(), false);
    test_pipelines(nonsparse_ibo_2, nonsparse_indices_2.size(), false);
    test_pipelines(nonsparse_ibo_3, nonsparse_indices_3.size(), false);
    test_pipelines(nonsparse_ibo_4, nonsparse_indices_4.size(), false);

    // our sparse indices should trigger warnings for both pipelines in this case
    test_pipelines(sparse_ibo, sparse_indices.size(), true);
}

TEST_F(VkArmBestPracticesLayerTest, PostTransformVertexCacheThrashingIndicesTest) {
    TEST_DESCRIPTION(
        "Test for appropriate warnings to be thrown when recording an indexed draw call where the indices thrash the "
        "post-transform vertex cache.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();
    InitRenderTarget();

    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "Test not supported by MockICD";
    }

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    std::vector<uint16_t> worst_indices;
    worst_indices.resize(128 * 16);
    for (size_t i = 0; i < 16; i++) {
        for (size_t j = 0; j < 128; j++) {
            // worst case index buffer sequence for re-use
            // (0, 1, 2, 3, ..., 127, 0, 1, 2, 3, ..., 127, 0, 1, 2, ...<x16>)
            worst_indices[j + i * 128] = j;
        }
    }

    std::vector<uint16_t> best_indices;
    best_indices.resize(128 * 16);
    for (size_t i = 0; i < 16; i++) {
        for (size_t j = 0; j < 128; j++) {
            // best case index buffer sequence for re-use
            // (0, 0, 0, ...<x16>, 1, 1, 1, ...<x16>, 2, 2, 2, ...<x16> , ..., 127)
            best_indices[i + j * 16] = j;
        }
    }

    // make sure the worst-case indices throw a warning
    VkConstantBufferObj worst_ibo(m_device, worst_indices.size() * sizeof(uint16_t), worst_indices.data(),
                                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), worst_ibo.handle(), static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);

    // the validation layer will only be able to analyse mapped memory, it's too expensive otherwise to do in the layer itself
    worst_ibo.memory().map();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCmdDrawIndexed-post-transform-cache-thrashing");
    vk::CmdDrawIndexed(m_commandBuffer->handle(), worst_indices.size(), 0, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    worst_ibo.memory().unmap();

    // make sure that the best-case indices don't throw a warning
    VkConstantBufferObj best_ibo(m_device, best_indices.size() * sizeof(uint16_t), best_indices.data(),
                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), best_ibo.handle(), static_cast<VkDeviceSize>(0), VK_INDEX_TYPE_UINT16);

    best_ibo.memory().map();
    vk::CmdDrawIndexed(m_commandBuffer->handle(), best_indices.size(), 0, 0, 0, 0);
    best_ibo.memory().unmap();
}

TEST_F(VkArmBestPracticesLayerTest, PresentModeTest) {
    TEST_DESCRIPTION("Test for usage of Presentation Modes");

    AddSurfaceExtension();
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo");
    if (!InitSurface()) {
        GTEST_SKIP() << "Cannot create surface, skipping test";
    }
    InitSwapchainInfo();

    VkBool32 supported;
    vk::GetPhysicalDeviceSurfaceSupportKHR(gpu(), m_device->graphics_queue_node_index_, m_surface, &supported);
    if (!supported) {
        GTEST_SKIP() << "Graphics queue does not support present, skipping test";
    }

    VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_surface_capabilities.minImageCount;
    swapchain_create_info.imageFormat = m_surface_formats[0].format;
    swapchain_create_info.imageColorSpace = m_surface_formats[0].colorSpace;
    swapchain_create_info.imageExtent = {m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height};
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = imageUsage;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.preTransform = preTransform;
    swapchain_create_info.clipped = VK_FALSE;
    swapchain_create_info.oldSwapchain = 0;
    swapchain_create_info.compositeAlpha = m_surface_composite_alpha;
    if (m_surface_present_modes.size() <= 1) {
        printf("TEST SKIPPED: Only %i presentation mode is available!", int(m_surface_present_modes.size()));
        return;
    }

    for (size_t i = 0; i < m_surface_present_modes.size(); i++) {
        if (m_surface_present_modes[i] != VK_PRESENT_MODE_FIFO_KHR) {
            swapchain_create_info.presentMode = m_surface_present_modes[i];
            break;
        }
    }

    ASSERT_EQ(VK_ERROR_VALIDATION_FAILED_EXT, vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateSwapchainKHR-swapchain-presentmode-not-fifo");
    swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    ASSERT_EQ(VK_SUCCESS, vk::CreateSwapchainKHR(device(), &swapchain_create_info, nullptr, &m_swapchain));
}

TEST_F(VkArmBestPracticesLayerTest, PipelineDepthBiasZeroTest) {
    TEST_DESCRIPTION("Test for unnecessary rasterization due to using 0 for depthBiasConstantFactor and depthBiasSlopeFactor");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();
    InitRenderTarget();

    CreatePipelineHelper pipe(*this);
    pipe.rs_state_ci_.depthBiasEnable = VK_TRUE;
    pipe.rs_state_ci_.depthBiasConstantFactor = 0.0f;
    pipe.rs_state_ci_.depthBiasSlopeFactor = 0.0f;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreatePipelines-depthbias-zero");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();

    pipe.rs_state_ci_.depthBiasEnable = VK_FALSE;
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreatePipelines-depthbias-zero");
    pipe.CreateGraphicsPipeline();
}

TEST_F(VkArmBestPracticesLayerTest, RobustBufferAccessTest) {
    TEST_DESCRIPTION("Test for appropriate warnings to be thrown when robustBufferAccess is enabled.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))

    VkDevice local_device;
    VkDeviceQueueCreateInfo queue_info = {};
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = nullptr;
    queue_info.queueFamilyIndex = 0;
    queue_info.queueCount = 1;
    float qp = 1.f;
    queue_info.pQueuePriorities = &qp;
    VkDeviceCreateInfo dev_info = {};
    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = nullptr;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &queue_info;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = nullptr;
    dev_info.enabledExtensionCount = m_device_extension_names.size();
    dev_info.ppEnabledExtensionNames = m_device_extension_names.data();

    VkPhysicalDeviceFeatures supported_features;
    vk::GetPhysicalDeviceFeatures(this->gpu(), &supported_features);
    if (supported_features.robustBufferAccess) {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-vkCreateDevice-RobustBufferAccess");
        VkPhysicalDeviceFeatures device_features = {};
        device_features.robustBufferAccess = VK_TRUE;
        dev_info.pEnabledFeatures = &device_features;
        vk::CreateDevice(this->gpu(), &dev_info, nullptr, &local_device);
        m_errorMonitor->VerifyFound();
    } else {
        GTEST_SKIP() << "robustBufferAccess is not available, skipping test";
    }
}

TEST_F(VkArmBestPracticesLayerTest, DepthPrePassUsage) {
    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_depth_stencil_fmt = FindSupportedDepthStencilFormat(gpu());

    m_depthStencil->Init(m_width, m_height, 1, m_depth_stencil_fmt, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                         VK_IMAGE_TILING_OPTIMAL);
    VkImageView depth_image_view =
        m_depthStencil->targetView(m_depth_stencil_fmt, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(&depth_image_view);

    VkPipelineColorBlendAttachmentState color_write_off = {};
    VkPipelineColorBlendAttachmentState color_write_on = {};
    color_write_on.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo cb_depth_only_ci = {};
    cb_depth_only_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb_depth_only_ci.attachmentCount = 1;
    cb_depth_only_ci.pAttachments = &color_write_off;

    VkPipelineColorBlendStateCreateInfo cb_depth_equal_ci = {};
    cb_depth_equal_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb_depth_equal_ci.attachmentCount = 1;
    cb_depth_equal_ci.pAttachments = &color_write_on;

    VkPipelineDepthStencilStateCreateInfo ds_depth_only_ci = {};
    ds_depth_only_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_depth_only_ci.depthTestEnable = VK_TRUE;
    ds_depth_only_ci.depthWriteEnable = VK_TRUE;
    ds_depth_only_ci.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineDepthStencilStateCreateInfo ds_depth_equal_ci = {};
    ds_depth_equal_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_depth_equal_ci.depthTestEnable = VK_TRUE;
    ds_depth_equal_ci.depthWriteEnable = VK_FALSE;
    ds_depth_equal_ci.depthCompareOp = VK_COMPARE_OP_EQUAL;

    CreatePipelineHelper pipe_depth_only(*this);
    pipe_depth_only.gp_ci_.pColorBlendState = &cb_depth_only_ci;
    pipe_depth_only.gp_ci_.pDepthStencilState = &ds_depth_only_ci;
    pipe_depth_only.InitState();
    pipe_depth_only.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_depth_equal(*this);
    pipe_depth_equal.gp_ci_.pColorBlendState = &cb_depth_equal_ci;
    pipe_depth_equal.gp_ci_.pDepthStencilState = &ds_depth_equal_ci;
    pipe_depth_equal.InitState();
    pipe_depth_equal.CreateGraphicsPipeline();

    // create a simple index buffer

    std::vector<uint32_t> indices = {};
    indices.resize(3);

    VkConstantBufferObj ibo(m_device, sizeof(uint32_t) * indices.size(), indices.data(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    m_commandBuffer->begin();
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), ibo.handle(), 0, VK_INDEX_TYPE_UINT32);

    // record a command buffer which doesn't use enough depth pre-passes or geometry to matter
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_only.pipeline_);
    for (size_t i = 0; i < 30; i++) vk::CmdDrawIndexed(m_commandBuffer->handle(), indices.size(), 10, 0, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_equal.pipeline_);
    for (size_t i = 0; i < 30; i++) vk::CmdDrawIndexed(m_commandBuffer->handle(), indices.size(), 10, 0, 0, 0);

    m_commandBuffer->EndRenderPass();

    // record a command buffer which records a significant number of depth pre-passes
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit,
                                         "UNASSIGNED-BestPractices-vkCmdEndRenderPass-depth-pre-pass-usage");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile");

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_only.pipeline_);
    for (size_t i = 0; i < 30; i++) vk::CmdDrawIndexed(m_commandBuffer->handle(), indices.size(), 1000, 0, 0, 0);

    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth_equal.pipeline_);
    for (size_t i = 0; i < 30; i++) vk::CmdDrawIndexed(m_commandBuffer->handle(), indices.size(), 1000, 0, 0, 0);

    m_commandBuffer->EndRenderPass();

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, ComputeShaderBadWorkGroupThreadAlignmentTest) {
    TEST_DESCRIPTION(
        "Testing for cases where compute shaders will be dispatched in an inefficient way, due to work group dispatch counts on "
        "Arm Mali architectures.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    VkShaderObj compute_4_1_1(this,
                              "#version 320 es\n"
                              "\n"
                              "layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;\n\n"
                              "void main() {}\n",
                              VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderObj compute_4_1_3(this,
                              "#version 320 es\n"
                              "\n"
                              "layout(local_size_x = 4, local_size_y = 1, local_size_z = 3) in;\n\n"
                              "void main() {}\n",
                              VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderObj compute_16_8_1(this,
                               "#version 320 es\n"
                               "\n"
                               "layout(local_size_x = 16, local_size_y = 8, local_size_z = 1) in;\n\n"
                               "void main() {}\n",
                               VK_SHADER_STAGE_COMPUTE_BIT);

    CreateComputePipelineHelper pipe(*this);

    auto makePipelineWithShader = [=](CreateComputePipelineHelper& pipe, const VkPipelineShaderStageCreateInfo& stage) {
        pipe.InitState();
        pipe.cp_ci_.stage = stage;
        pipe.dsl_bindings_ = {};
        pipe.cp_ci_.layout = pipe.pipeline_layout_.handle();

        pipe.CreateComputePipeline(false);
    };

    // these two pipelines should not cause any warning
    makePipelineWithShader(pipe, compute_4_1_1.GetStageCreateInfo());

    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-work-group-size");
    makePipelineWithShader(pipe, compute_16_8_1.GetStageCreateInfo());

    // this pipeline should cause a warning due to bad work group alignment

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-thread-group-alignment");
    makePipelineWithShader(pipe, compute_4_1_3.GetStageCreateInfo());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, ComputeShaderBadWorkGroupThreadCountTest) {
    TEST_DESCRIPTION(
        "Testing for cases where the number of work groups spawned is greater than advised for Arm Mali architectures.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    VkShaderObj compute_4_1_1(this,
                              "#version 320 es\n"
                              "\n"
                              "layout(local_size_x = 4, local_size_y = 1, local_size_z = 1) in;\n\n"
                              "void main() {}\n",
                              VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderObj compute_4_1_3(this,
                              "#version 320 es\n"
                              "\n"
                              "layout(local_size_x = 4, local_size_y = 1, local_size_z = 3) in;\n\n"
                              "void main() {}\n",
                              VK_SHADER_STAGE_COMPUTE_BIT);

    VkShaderObj compute_16_8_1(this,
                               "#version 320 es\n"
                               "\n"
                               "layout(local_size_x = 16, local_size_y = 8, local_size_z = 1) in;\n\n"
                               "void main() {}\n",
                               VK_SHADER_STAGE_COMPUTE_BIT);

    CreateComputePipelineHelper pipe(*this);

    auto make_pipeline_with_shader = [=](CreateComputePipelineHelper& pipe, const VkPipelineShaderStageCreateInfo& stage) {
        pipe.InitState();
        pipe.cp_ci_.stage = stage;
        pipe.dsl_bindings_ = {};
        pipe.cp_ci_.layout = pipe.pipeline_layout_.handle();

        pipe.CreateComputePipeline(false);
    };

    // these two pipelines should not cause any warning
    make_pipeline_with_shader(pipe, compute_4_1_1.GetStageCreateInfo());

    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-thread-group-alignment");
    make_pipeline_with_shader(pipe, compute_4_1_3.GetStageCreateInfo());

    // this pipeline should cause a warning due to the total workgroup count

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-work-group-size");
    make_pipeline_with_shader(pipe, compute_16_8_1.GetStageCreateInfo());
    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, ComputeShaderBadSpatialLocalityTest) {
    TEST_DESCRIPTION(
        "Testing for cases where a compute shader's configuration makes poor use of spatial locality, on Arm Mali architectures, "
        "for one or more of its resources.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    VkShaderObj compute_sampler_2d_8_8_1(this,
                                         "#version 450\n"
                                         "layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;\n\n"
                                         "layout(set = 0, binding = 0) uniform sampler2D uSampler;\n"
                                         "void main() {\n"
                                         "    vec4 value = textureLod(uSampler, vec2(0.5), 0.0);\n"
                                         "}\n",
                                         VK_SHADER_STAGE_COMPUTE_BIT);
    VkShaderObj compute_sampler_1d_64_1_1(this,
                                          "#version 450\n"
                                          "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;\n\n"
                                          "layout(set = 0, binding = 0) uniform sampler1D uSampler;\n"
                                          "void main() {\n"
                                          "    vec4 value = textureLod(uSampler, 0.5, 0.0);\n"
                                          "}\n",
                                          VK_SHADER_STAGE_COMPUTE_BIT);
    VkShaderObj compute_sampler_2d_64_1_1(this,
                                          "#version 450\n"
                                          "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;\n\n"
                                          "layout(set = 0, binding = 0) uniform sampler2D uSampler;\n"
                                          "void main() {\n"
                                          "    vec4 value = textureLod(uSampler, vec2(0.5), 0.0);\n"
                                          "}\n",
                                          VK_SHADER_STAGE_COMPUTE_BIT);

    CreateComputePipelineHelper pipe(*this);

    auto make_pipeline_with_shader = [this](CreateComputePipelineHelper& pipe, const VkPipelineShaderStageCreateInfo& stage) {
        VkDescriptorSetLayoutBinding sampler_binding = {};
        sampler_binding.binding = 0;
        sampler_binding.descriptorCount = 1;
        sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

        pipe.InitState();
        auto ds_layout = std::unique_ptr<vkt::DescriptorSetLayout>(new vkt::DescriptorSetLayout(*m_device, {sampler_binding}));
        auto pipe_layout = std::unique_ptr<vkt::PipelineLayout>(new vkt::PipelineLayout(*m_device, {ds_layout.get()}));
        pipe.cp_ci_.stage = stage;
        pipe.cp_ci_.layout = pipe_layout->handle();

        pipe.CreateComputePipeline(false);
    };

    auto* this_ptr = this;  // Required for older compilers with c++20 compatibility
    auto test_spatial_locality = [=](CreateComputePipelineHelper& pipe, const VkPipelineShaderStageCreateInfo& stage,
                                     bool positive_test) {
        if (!positive_test) {
            this_ptr->m_errorMonitor->SetDesiredFailureMsg(
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                "UNASSIGNED-BestPractices-vkCreateComputePipelines-compute-spatial-locality");
        }
        make_pipeline_with_shader(pipe, stage);
        if (!positive_test) {
            this_ptr->m_errorMonitor->VerifyFound();
        }
    };

    test_spatial_locality(pipe, compute_sampler_2d_8_8_1.GetStageCreateInfo(), true);
    test_spatial_locality(pipe, compute_sampler_1d_64_1_1.GetStageCreateInfo(), true);
    test_spatial_locality(pipe, compute_sampler_2d_64_1_1.GetStageCreateInfo(), false);
}

TEST_F(VkArmBestPracticesLayerTest, RedundantRenderPassStore) {
    TEST_DESCRIPTION("Test for appropriate warnings to be thrown when a redundant store is used.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-RenderPass-redundant-store");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile");

    const VkFormat FMT = VK_FORMAT_R8G8B8A8_UNORM;
    const uint32_t WIDTH = 512, HEIGHT = 512;

    std::vector<std::unique_ptr<VkImageObj>> images;
    std::vector<VkRenderPass> renderpasses;
    std::vector<VkFramebuffer> framebuffers;

    images.push_back(CreateImage(FMT, WIDTH, HEIGHT));
    renderpasses.push_back(CreateRenderPass(FMT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE));
    framebuffers.push_back(CreateFramebuffer(WIDTH, HEIGHT, images[0]->targetView(FMT), renderpasses[0]));

    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindImageMemory-non-lazy-transient-image");
    auto img = std::unique_ptr<VkImageObj>(new VkImageObj(m_device));
    img->Init(WIDTH, HEIGHT, 1, FMT, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
              VK_IMAGE_TILING_OPTIMAL);
    images.push_back(std::move(img));
    renderpasses.push_back(CreateRenderPass(FMT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE));
    framebuffers.push_back(CreateFramebuffer(WIDTH, HEIGHT, images[1]->targetView(FMT), renderpasses[1]));

    CreatePipelineHelper graphics_pipeline(*this);
    graphics_pipeline.dsl_bindings_[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDynamicState ds = VK_DYNAMIC_STATE_VIEWPORT;
    graphics_pipeline.dyn_state_ci_ = vku::InitStructHelper();
    graphics_pipeline.dyn_state_ci_.dynamicStateCount = 1;
    graphics_pipeline.dyn_state_ci_.pDynamicStates = &ds;

    graphics_pipeline.InitState();

    graphics_pipeline.gp_ci_.renderPass = renderpasses[1];
    graphics_pipeline.gp_ci_.flags = 0;

    graphics_pipeline.CreateGraphicsPipeline();

    VkClearValue clear_values[3];
    memset(clear_values, 0, sizeof(clear_values));

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = renderpasses[0];
    render_pass_begin_info.framebuffer = framebuffers[0];
    render_pass_begin_info.clearValueCount = 3;
    render_pass_begin_info.pClearValues = clear_values;

    const auto execute_work = [&](const std::function<void(vkt::CommandBuffer & command_buffer)>& work) {
        vk::ResetCommandPool(device(), m_commandPool->handle(), 0);
        m_commandBuffer->begin();

        work(*m_commandBuffer);

        m_commandBuffer->end();

        VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &m_commandBuffer->handle();
        vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
    };

    const auto start_and_end_renderpass = [&](vkt::CommandBuffer& command_buffer) {
        command_buffer.BeginRenderPass(render_pass_begin_info);
        command_buffer.EndRenderPass();
    };

    execute_work(start_and_end_renderpass);

    // Use the image somehow.
    execute_work([&](vkt::CommandBuffer& command_buffer) {
        VkRenderPassBeginInfo rpbi = vku::InitStructHelper();
        rpbi.renderPass = renderpasses[1];
        rpbi.framebuffer = framebuffers[1];
        rpbi.clearValueCount = 3;
        rpbi.pClearValues = clear_values;

        command_buffer.BeginRenderPass(rpbi);

        vk::CmdBindPipeline(command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline_);

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(WIDTH);
        viewport.height = static_cast<float>(HEIGHT);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vk::CmdSetViewport(command_buffer.handle(), 0, 1, &viewport);
        vk::CmdDraw(command_buffer.handle(), 3, 1, 0, 0);

        command_buffer.EndRenderPass();
    });

    execute_work(start_and_end_renderpass);

    m_errorMonitor->VerifyFound();

    for (auto rp : renderpasses) {
        vk::DestroyRenderPass(device(), rp, nullptr);
    }
    for (auto fb : framebuffers) {
        vk::DestroyFramebuffer(device(), fb, nullptr);
    }
}

TEST_F(VkArmBestPracticesLayerTest, RedundantRenderPassClear) {
    TEST_DESCRIPTION("Test for appropriate warnings to be thrown when a redundant clear is used.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-RenderPass-redundant-clear");

    const VkFormat FMT = VK_FORMAT_R8G8B8A8_UNORM;
    const uint32_t WIDTH = 512, HEIGHT = 512;

    std::vector<std::unique_ptr<VkImageObj>> images;
    std::vector<VkRenderPass> renderpasses;
    std::vector<VkFramebuffer> framebuffers;

    images.push_back(CreateImage(FMT, WIDTH, HEIGHT));
    images[0]->SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    renderpasses.push_back(CreateRenderPass(FMT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE));
    framebuffers.push_back(CreateFramebuffer(WIDTH, HEIGHT, images[0]->targetView(FMT), renderpasses[0]));

    CreatePipelineHelper graphics_pipeline(*this);
    graphics_pipeline.dsl_bindings_[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    graphics_pipeline.cb_attachments_[0].colorWriteMask = 0xf;
    graphics_pipeline.InitState();

    graphics_pipeline.gp_ci_.renderPass = renderpasses[0];
    graphics_pipeline.gp_ci_.flags = 0;

    graphics_pipeline.CreateGraphicsPipeline();

    VkClearValue clear_values[3];
    memset(clear_values, 0, sizeof(clear_values));

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = renderpasses[0];
    render_pass_begin_info.framebuffer = framebuffers[0];
    render_pass_begin_info.clearValueCount = 3;
    render_pass_begin_info.pClearValues = clear_values;

    m_commandBuffer->begin();

    VkClearColorValue clear_color_value = {};
    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;
    subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
    vk::CmdClearColorImage(m_commandBuffer->handle(), images[0]->image(), VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1,
                           &subresource_range);

    m_commandBuffer->BeginRenderPass(render_pass_begin_info);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(WIDTH);
    viewport.height = static_cast<float>(HEIGHT);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();

    VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();

    for (auto rp : renderpasses) {
        vk::DestroyRenderPass(device(), rp, nullptr);
    }
    for (auto fb : framebuffers) {
        vk::DestroyFramebuffer(device(), fb, nullptr);
    }
}

TEST_F(VkArmBestPracticesLayerTest, InefficientRenderPassClear) {
    TEST_DESCRIPTION("Test for appropriate warnings to be thrown when a redundant clear is used on a LOAD_OP_LOAD attachment.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-RenderPass-inefficient-clear");

    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");

    const VkFormat FMT = VK_FORMAT_R8G8B8A8_UNORM;
    const uint32_t WIDTH = 512, HEIGHT = 512;

    // Create renderpass
    VkAttachmentDescription attachment = {};
    attachment.format = FMT;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo rpinf = vku::InitStructHelper();
    rpinf.attachmentCount = 1;
    rpinf.pAttachments = &attachment;
    rpinf.subpassCount = 1;
    rpinf.pSubpasses = &subpass;

    vkt::RenderPass rp(*m_device, rpinf);

    std::unique_ptr<VkImageObj> image = CreateImage(FMT, WIDTH, HEIGHT);
    image->SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    VkFramebuffer fb = CreateFramebuffer(WIDTH, HEIGHT, image->targetView(FMT), rp.handle());

    CreatePipelineHelper graphics_pipeline(*this);
    graphics_pipeline.dsl_bindings_[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    graphics_pipeline.cb_attachments_[0].colorWriteMask = 0xf;
    graphics_pipeline.InitState();

    graphics_pipeline.gp_ci_.renderPass = rp.handle();
    graphics_pipeline.gp_ci_.flags = 0;

    graphics_pipeline.CreateGraphicsPipeline();

    VkClearValue clear_values[3];
    memset(clear_values, 0, sizeof(clear_values));

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = rp.handle();
    render_pass_begin_info.framebuffer = fb;
    render_pass_begin_info.clearValueCount = 3;
    render_pass_begin_info.pClearValues = clear_values;

    m_commandBuffer->begin();

    VkClearColorValue clear_color_value = {};
    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;
    subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
    vk::CmdClearColorImage(m_commandBuffer->handle(), image->handle(), VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1,
                           &subresource_range);

    m_commandBuffer->BeginRenderPass(render_pass_begin_info);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(WIDTH);
    viewport.height = static_cast<float>(HEIGHT);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline_);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);

    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();

    VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();

    vk::DestroyFramebuffer(device(), fb, nullptr);
}

TEST_F(VkArmBestPracticesLayerTest, DescriptorTracking) {
    TEST_DESCRIPTION("Tests that we track descriptors, which means we should not trigger warnings.");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation))
    InitState();

    m_errorMonitor->SetDesiredFailureMsg(kPerformanceWarningBit, "UNASSIGNED-BestPractices-RenderPass-inefficient-clear");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");

    const VkFormat FMT = VK_FORMAT_R8G8B8A8_UNORM;
    const uint32_t WIDTH = 512, HEIGHT = 512;

    // Create renderpass
    VkAttachmentDescription attachment = {};
    attachment.format = FMT;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_GENERAL;

    VkSubpassDescription subpass = {};
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo rpinf = vku::InitStructHelper();
    rpinf.attachmentCount = 1;
    rpinf.pAttachments = &attachment;
    rpinf.subpassCount = 1;
    rpinf.pSubpasses = &subpass;

    vkt::RenderPass rp(*m_device, rpinf);

    std::vector<std::unique_ptr<VkImageObj>> images;
    std::vector<VkFramebuffer> framebuffers;

    images.push_back(CreateImage(FMT, WIDTH, HEIGHT));
    images.push_back(CreateImage(FMT, WIDTH, HEIGHT));

    for (auto& image : images) {
        image->SetLayout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
    }

    framebuffers.push_back(CreateFramebuffer(WIDTH, HEIGHT, images[0]->targetView(FMT), rp.handle()));
    framebuffers.push_back(CreateFramebuffer(WIDTH, HEIGHT, images[1]->targetView(FMT), rp.handle()));

    CreatePipelineHelper graphics_pipeline(*this);
    graphics_pipeline.dsl_bindings_.resize(2);
    graphics_pipeline.dsl_bindings_[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    // Tests that we correctly handle weird binding layouts.
    graphics_pipeline.dsl_bindings_[0].binding = 20;
    graphics_pipeline.dsl_bindings_[0].descriptorCount = 1;
    graphics_pipeline.dsl_bindings_[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    graphics_pipeline.dsl_bindings_[1].binding = 10;
    graphics_pipeline.dsl_bindings_[1].descriptorCount = 4;
    graphics_pipeline.cb_attachments_[0].colorWriteMask = 0xf;
    graphics_pipeline.InitState();

    graphics_pipeline.gp_ci_.renderPass = rp.handle();
    graphics_pipeline.gp_ci_.flags = 0;

    graphics_pipeline.CreateGraphicsPipeline();

    VkDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = 4;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptor_pool_create_info.maxSets = 1;
    descriptor_pool_create_info.poolSizeCount = 2;
    descriptor_pool_create_info.pPoolSizes = pool_sizes;
    vkt::DescriptorPool pool(*m_device, descriptor_pool_create_info);

    VkDescriptorSet descriptor_set{VK_NULL_HANDLE};
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptor_set_allocate_info.descriptorPool = pool.handle();
    descriptor_set_allocate_info.descriptorSetCount = 1;
    descriptor_set_allocate_info.pSetLayouts = &graphics_pipeline.descriptor_set_->layout_.handle();
    vk::AllocateDescriptorSets(m_device->handle(), &descriptor_set_allocate_info, &descriptor_set);

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = images[1]->targetView(FMT);
    image_info.sampler = VK_NULL_HANDLE;
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.descriptorCount = 1;
    write.dstBinding = 10;
    write.dstArrayElement = 1;
    write.dstSet = descriptor_set;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.pImageInfo = &image_info;
    vk::UpdateDescriptorSets(m_device->handle(), 1, &write, 0, nullptr);

    VkClearValue clear_values[3];
    memset(clear_values, 0, sizeof(clear_values));

    VkRenderPassBeginInfo render_pass_begin_info = vku::InitStructHelper();
    render_pass_begin_info.renderPass = rp.handle();
    render_pass_begin_info.framebuffer = framebuffers[0];
    render_pass_begin_info.clearValueCount = 3;
    render_pass_begin_info.pClearValues = clear_values;

    m_commandBuffer->begin();

    VkClearColorValue clear_color_value = {};
    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;
    subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
    vk::CmdClearColorImage(m_commandBuffer->handle(), images[1]->image(), VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1,
                           &subresource_range);

    // Trigger a read on the image.
    m_commandBuffer->BeginRenderPass(render_pass_begin_info);
    {
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  graphics_pipeline.pipeline_layout_.handle(), 0, 1, &descriptor_set, 0, nullptr);

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(WIDTH);
        viewport.height = static_cast<float>(HEIGHT);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }
    m_commandBuffer->EndRenderPass();

    // Now, LOAD_OP_LOAD, which should not trigger since we already read the image.
    render_pass_begin_info.framebuffer = framebuffers[1];
    m_commandBuffer->BeginRenderPass(render_pass_begin_info);
    {
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(WIDTH);
        viewport.height = static_cast<float>(HEIGHT);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    }
    m_commandBuffer->EndRenderPass();

    m_commandBuffer->end();

    VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    for (auto fb : framebuffers) {
        vk::DestroyFramebuffer(device(), fb, nullptr);
    }
}

TEST_F(VkArmBestPracticesLayerTest, BlitImageLoadOpLoad) {
    TEST_DESCRIPTION("Test for vkBlitImage followed by a LoadOpLoad renderpass");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation));
    InitState();

    m_clear_via_load_op = false;  // Force LOAD_OP_LOAD
    InitRenderTarget();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                         "UNASSIGNED-BestPractices-RenderPass-blitimage-loadopload");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkAllocateMemory-small-allocation");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkBindMemory-small-dedicated-allocation");
    // On tiled renderers, this can also trigger a warning about LOAD_OP_LOAD causing a readback
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdBeginRenderPass-attachment-needs-readback");
    m_errorMonitor->SetAllowedFailureMsg("UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile");
    m_commandBuffer->begin();

    const VkFormat FMT = VK_FORMAT_R8G8B8A8_UNORM;
    const uint32_t WIDTH = 512, HEIGHT = 512;

    std::vector<std::unique_ptr<VkImageObj>> images;
    images.push_back(CreateImage(FMT, WIDTH, HEIGHT));
    images.push_back(CreateImage(FMT, WIDTH, HEIGHT));

    VkImageMemoryBarrier image_barriers[2] = {
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         nullptr,
         0,
         VK_ACCESS_TRANSFER_READ_BIT,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
         VK_QUEUE_FAMILY_IGNORED,
         VK_QUEUE_FAMILY_IGNORED,
         images[0]->image(),
         {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         nullptr,
         0,
         VK_ACCESS_TRANSFER_WRITE_BIT,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         VK_QUEUE_FAMILY_IGNORED,
         VK_QUEUE_FAMILY_IGNORED,
         images[1]->image(),
         {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
    };
    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                           nullptr, 0, nullptr, 2, image_barriers);

    VkOffset3D blit_size{WIDTH, HEIGHT, 1};
    VkImageBlit blit_region{};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcOffsets[1] = blit_size;
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstOffsets[1] = blit_size;

    vk::CmdBlitImage(m_commandBuffer->handle(), images[0]->image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, images[1]->image(),
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_LINEAR);

    VkImageMemoryBarrier pre_render_pass_barriers[2] = {
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         nullptr,
         VK_ACCESS_TRANSFER_READ_BIT,
         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_QUEUE_FAMILY_IGNORED,
         VK_QUEUE_FAMILY_IGNORED,
         images[0]->image(),
         {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
         nullptr,
         VK_ACCESS_TRANSFER_WRITE_BIT,
         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         VK_QUEUE_FAMILY_IGNORED,
         VK_QUEUE_FAMILY_IGNORED,
         images[1]->image(),
         {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
    };

    vk::CmdPipelineBarrier(m_commandBuffer->handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                           0, 0, nullptr, 0, nullptr, 2, pre_render_pass_barriers);

    // A renderpass with two subpasses, both writing the same attachment.
    VkAttachmentDescription attach[] = {
        {0, FMT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    };
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {
        0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &ref, nullptr, nullptr, 0, nullptr,
    };
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, 1, attach, 1, &subpass, 0, nullptr};
    vkt::RenderPass rp(*m_device, rpci);

    auto imageView = images[1]->targetView(FMT);
    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, rp.handle(), 1, &imageView, WIDTH, HEIGHT, 1};
    vkt::Framebuffer fb(*m_device, fbci);

    VkRenderPassBeginInfo rpbi =
        vku::InitStruct<VkRenderPassBeginInfo>(nullptr, rp.handle(), fb.handle(), VkRect2D{{0, 0}, {WIDTH, HEIGHT}}, 0u, nullptr);

    // subtest 1: bind in the wrong subpass
    m_commandBuffer->BeginRenderPass(rpbi);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    m_errorMonitor->VerifyFound();
}

TEST_F(VkArmBestPracticesLayerTest, RedundantAttachment) {
    TEST_DESCRIPTION("Test for redundant renderpasses which consume bandwidth");

    RETURN_IF_SKIP(InitBestPracticesFramework(kEnableArmValidation));
    InitState();

    // One of these formats must be supported.
    VkFormat ds_format = VK_FORMAT_D24_UNORM_S8_UINT;
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(gpu(), ds_format, &format_props);
    if ((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) {
        ds_format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        vk::GetPhysicalDeviceFormatProperties(gpu(), ds_format, &format_props);
        ASSERT_TRUE((format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0);
    }

    auto ds = CreateImage(ds_format, m_width, m_height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    m_clear_via_load_op = true;
    m_depth_stencil_fmt = ds_format;
    auto ds_view = ds->targetView(ds_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    InitRenderTarget(1, &ds_view);

    CreatePipelineHelper pipe_all(*this);
    pipe_all.InitState();
    pipe_all.cb_attachments_[0].colorWriteMask = 0xf;
    pipe_all.ds_ci_ = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    pipe_all.gp_ci_.pDepthStencilState = &pipe_all.ds_ci_;
    pipe_all.ds_ci_.depthTestEnable = VK_TRUE;
    pipe_all.ds_ci_.stencilTestEnable = VK_TRUE;
    pipe_all.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_color(*this);
    pipe_color.InitState();
    pipe_color.cb_attachments_[0].colorWriteMask = 0xf;
    pipe_color.ds_ci_ = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    pipe_color.gp_ci_.pDepthStencilState = &pipe_color.ds_ci_;
    pipe_color.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_depth(*this);
    pipe_depth.InitState();
    pipe_depth.cb_attachments_[0].colorWriteMask = 0;
    pipe_depth.ds_ci_ = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    pipe_depth.gp_ci_.pDepthStencilState = &pipe_depth.ds_ci_;
    pipe_depth.ds_ci_.depthTestEnable = VK_TRUE;
    pipe_depth.CreateGraphicsPipeline();

    CreatePipelineHelper pipe_stencil(*this);
    pipe_stencil.InitState();
    pipe_stencil.cb_attachments_[0].colorWriteMask = 0;
    pipe_stencil.ds_ci_ = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    pipe_stencil.gp_ci_.pDepthStencilState = &pipe_stencil.ds_ci_;
    pipe_stencil.ds_ci_.stencilTestEnable = VK_TRUE;
    pipe_stencil.CreateGraphicsPipeline();

    m_commandBuffer->begin();

    // Nothing is redundant.
    {
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_all.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
    }

    // Only color is redundant.
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile");
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_stencil.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
    }

    // Only depth is redundant.
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile");
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_color.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_stencil.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
    }

    // Only stencil is redundant.
    {
        m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
                                             "UNASSIGNED-BestPractices-vkCmdEndRenderPass-redundant-attachment-on-tile");
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

        // Test that clear attachments counts as an access.
        VkClearAttachment clear_att = {};
        VkClearRect clear_rect = {};

        clear_att.colorAttachment = 0;
        clear_att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clear_rect.layerCount = 1;
        clear_rect.rect = {{0, 0}, {1, 1}};
        vk::CmdClearAttachments(m_commandBuffer->handle(), 1, &clear_att, 1, &clear_rect);

        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_depth.pipeline_);
        vk::CmdDraw(m_commandBuffer->handle(), 1, 1, 0, 0);
        m_commandBuffer->EndRenderPass();
        m_errorMonitor->VerifyFound();
        m_commandBuffer->EndRenderPass();
    }

    m_commandBuffer->end();
}
