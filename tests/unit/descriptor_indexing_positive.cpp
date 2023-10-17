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

void DescriptorIndexingTest::InitBasicDescriptorIndexing(void* pNextFeatures) {
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    descriptor_indexing_features = vku::InitStructHelper(pNextFeatures);
    GetPhysicalDeviceFeatures2(descriptor_indexing_features);
    RETURN_IF_SKIP(InitState(nullptr, &descriptor_indexing_features));
}

void DescriptorIndexingTest::ComputePipelineShaderTest(const char *shader, std::vector<VkDescriptorSetLayoutBinding> &bindings) {
    RETURN_IF_SKIP(InitBasicDescriptorIndexing())
    InitRenderTarget();

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_.resize(bindings.size());
    memcpy(pipe.dsl_bindings_.data(), bindings.data(), bindings.size() * sizeof(VkDescriptorSetLayoutBinding));
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.InitState();
    pipe.CreateComputePipeline();
}

TEST_F(PositiveDescriptorIndexing, BindingPartiallyBound) {
    TEST_DESCRIPTION("Ensure that no validation errors for invalid descriptors if binding is PARTIALLY_BOUND");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    RETURN_IF_SKIP(InitBasicDescriptorIndexing())

    if (!descriptor_indexing_features.descriptorBindingPartiallyBound) {
        GTEST_SKIP() << "Partially bound bindings not supported, skipping test";
    }

    InitRenderTarget();

    VkDescriptorBindingFlagsEXT ds_binding_flags[2] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags = vku::InitStructHelper();
    ds_binding_flags[0] = 0;
    // No Error
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
    // Uncomment for Error
    // ds_binding_flags[1] = 0;

    layout_createinfo_binding_flags.bindingCount = 2;
    layout_createinfo_binding_flags.pBindingFlags = ds_binding_flags;

    // Prepare descriptors
    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                           {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                       },
                                       0, &layout_createinfo_binding_flags, 0);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    uint32_t qfi = 0;
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.size = 32;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer_create_info.queueFamilyIndexCount = 1;
    buffer_create_info.pQueueFamilyIndices = &qfi;

    vkt::Buffer buffer(*m_device, buffer_create_info);

    VkDescriptorBufferInfo buffer_info[2] = {};
    buffer_info[0].buffer = buffer.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkBufferCreateInfo index_buffer_create_info = vku::InitStructHelper();
    index_buffer_create_info.size = sizeof(uint32_t);
    index_buffer_create_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vkt::Buffer index_buffer(*m_device, index_buffer_create_info);

    // Only update binding 0
    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStructHelper();
    descriptor_writes[0].dstSet = descriptor_set.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, descriptor_writes, 0, NULL);

    char const *shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) uniform foo_0 { int val; } doit;
        layout(set = 0, binding = 1) uniform foo_1 { int val; } readit;
        void main() {
            if (doit.val == 0)
                gl_Position = vec4(0.0);
            else
                gl_Position = vec4(readit.val);
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_commandBuffer->begin(&begin_info);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexed(m_commandBuffer->handle(), 1, 1, 0, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    m_commandBuffer->QueueCommandBuffer();
}

TEST_F(PositiveDescriptorIndexing, UpdateAfterBind) {
    TEST_DESCRIPTION("Test UPDATE_AFTER_BIND does not reset command buffers.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDescriptorIndexing(&synchronization2))

    if (descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind == VK_FALSE) {
        GTEST_SKIP() << "descriptorBindingStorageBufferUpdateAfterBind feature is not available";
    }
    if (synchronization2.synchronization2 == VK_FALSE) {
        GTEST_SKIP() << "synchronization2 feature is not available";
    }
    InitRenderTarget();

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBuffer buffer1, buffer2, buffer3;
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer1);
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer2);
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer3);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer1, &buffer_mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_mem_reqs.size;

    VkDeviceMemory memory1, memory2, memory3;
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory1);
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory2);
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory3);

    vk::BindBufferMemory(device(), buffer1, memory1, 0);
    vk::BindBufferMemory(device(), buffer2, memory2, 0);
    vk::BindBufferMemory(device(), buffer3, memory3, 0);

    OneOffDescriptorSet::Bindings binding_defs = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    VkDescriptorBindingFlagsEXT flags[2] = {VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT, 0};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;
    OneOffDescriptorSet descriptor_set(m_device, binding_defs, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
                                       &flags_create_info, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info = {buffer1, 0, sizeof(uint32_t)};

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    descriptor_write.dstBinding = 1;
    buffer_info.buffer = buffer3;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    descriptor_write.dstBinding = 0;

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer buf1 {
            float a;
        } ubuf1;
        layout (set = 0, binding = 1) buffer buf2 {
            float a;
        } ubuf2;
        void main() {
           float f = ubuf1.a * ubuf2.a;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroyBuffer(device(), buffer1, nullptr);
    buffer_info.buffer = buffer2;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    VkCommandBufferSubmitInfoKHR cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = m_commandBuffer->handle();

    VkSubmitInfo2KHR submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    vk::DestroyBuffer(device(), buffer2, nullptr);
    vk::DestroyBuffer(device(), buffer3, nullptr);

    vk::FreeMemory(device(), memory1, nullptr);
    vk::FreeMemory(device(), memory2, nullptr);
    vk::FreeMemory(device(), memory3, nullptr);
}

TEST_F(PositiveDescriptorIndexing, PartiallyBoundDescriptors) {
    TEST_DESCRIPTION("Test partially bound descriptors do not reset command buffers.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    VkPhysicalDeviceSynchronization2FeaturesKHR synchronization2 = vku::InitStructHelper();
    RETURN_IF_SKIP(InitBasicDescriptorIndexing(&synchronization2))

    if (descriptor_indexing_features.descriptorBindingStorageBufferUpdateAfterBind == VK_FALSE) {
        GTEST_SKIP() << "descriptorBindingStorageBufferUpdateAfterBind feature is not available";
    }
    if (synchronization2.synchronization2 == VK_FALSE) {
        GTEST_SKIP() << "synchronization2 feature is not available";
    }

    InitRenderTarget();

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 4096;
    buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkBuffer buffer1, buffer3;
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer1);
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer3);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer1, &buffer_mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buffer_mem_reqs.size;

    VkDeviceMemory memory1, memory3;
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory1);
    vk::AllocateMemory(device(), &alloc_info, nullptr, &memory3);

    vk::BindBufferMemory(device(), buffer1, memory1, 0);
    vk::BindBufferMemory(device(), buffer3, memory3, 0);

    OneOffDescriptorSet::Bindings binding_defs = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
    };
    VkDescriptorBindingFlagsEXT flags[2] = {VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, 0};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flags_create_info = vku::InitStructHelper();
    flags_create_info.bindingCount = 2;
    flags_create_info.pBindingFlags = flags;
    OneOffDescriptorSet descriptor_set(m_device, binding_defs, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
                                       &flags_create_info, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT);
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorBufferInfo buffer_info = {buffer1, 0, sizeof(uint32_t)};

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.pBufferInfo = &buffer_info;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    descriptor_write.dstBinding = 1;
    buffer_info.buffer = buffer3;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);
    descriptor_write.dstBinding = 0;

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) buffer buf1 {
            float a;
        } ubuf1;
        layout (set = 0, binding = 1) buffer buf2 {
            float a;
        } ubuf2;
        void main() {
           float f = ubuf2.a;
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    vk::DestroyBuffer(device(), buffer1, nullptr);

    VkCommandBufferSubmitInfoKHR cb_info = vku::InitStructHelper();
    cb_info.commandBuffer = m_commandBuffer->handle();

    VkSubmitInfo2KHR submit_info = vku::InitStructHelper();
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &cb_info;

    vk::QueueSubmit2KHR(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    vk::DestroyBuffer(device(), buffer3, nullptr);

    vk::FreeMemory(device(), memory1, nullptr);
    vk::FreeMemory(device(), memory3, nullptr);
}

TEST_F(PositiveDescriptorIndexing, PipelineShaderBasic) {
    TEST_DESCRIPTION("Test basic usage of GL_EXT_nonuniform_qualifier.");
    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) buffer block { int x; };
        void main() {
            nonuniformEXT int data;
            int table[5];
            data = table[nonuniformEXT(x)];
        }
    )glsl";

    ComputePipelineShaderTest(csSource, bindings);
}

TEST_F(PositiveDescriptorIndexing, PipelineShaderSampler2D) {
    TEST_DESCRIPTION("Indexing into a Sampler2D (combined image sampler).");
    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) buffer block { vec2 x; };
        layout(set=0, binding=1) uniform sampler2D t;
        void main() {
            vec4 vColor4 = texture(t, nonuniformEXT(x));
        }
    )glsl";

    ComputePipelineShaderTest(csSource, bindings);
}

TEST_F(PositiveDescriptorIndexing, PipelineShaderImageBufferArray) {
    TEST_DESCRIPTION("Indexing into a ImageVuffer array (texel buffer).");
    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set=0, binding=0) buffer block { int x; };
        layout(set=0, binding=1, rgba8ui) uniform uimageBuffer image_buffer_array[];
        void main() {
            vec4 color = vec4(1.0);
            color += imageLoad(image_buffer_array[x], 0);
            // uses a OpCopyObject
            color += imageLoad(image_buffer_array[nonuniformEXT(x)], 0);
        }
    )glsl";

    ComputePipelineShaderTest(csSource, bindings);
}

TEST_F(PositiveDescriptorIndexing, PipelineShaderMultiArrayIndexing) {
    TEST_DESCRIPTION("Indexing into a nested array.");
    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
    };

    char const *csSource = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable
        layout(set = 0, binding = 0) uniform A { uint value; };
        layout(set = 0, binding = 1) uniform B { uint tex_index[1]; };
        layout(set = 0, binding = 2) uniform sampler2D tex[6];
        void main() {
            vec4 color = vec4(1.0);
            color +=  texture(tex[tex_index[value]], vec2(0, 0));
            color +=  texture(tex[tex_index[nonuniformEXT(value)]], vec2(0, 0));
            color +=  texture(tex[nonuniformEXT(tex_index[value])], vec2(0, 0));
        }
    )glsl";

    ComputePipelineShaderTest(csSource, bindings);
}
