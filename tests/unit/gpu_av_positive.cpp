/* Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../framework/layer_validation_tests.h"

class PositiveGpuAssistedLayer : public VkGpuAssistedLayerTest {};

TEST_F(PositiveGpuAssistedLayer, SetSSBOBindDescriptor) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdBindDescriptorSets()");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, &validation_features));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    if (IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on Shield TV";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceProperties properties = {};
    vk::GetPhysicalDeviceProperties(gpu(), &properties);
    if (properties.limits.maxBoundDescriptorSets < 8) {
        GTEST_SKIP() << "maxBoundDescriptorSets is too low";
    }

    char const *csSource = R"glsl(
        #version 450
        layout(constant_id=0) const uint _const_2_0 = 1;
        layout(constant_id=1) const uint _const_3_0 = 1;
        layout(std430, binding=0) readonly restrict buffer _SrcBuf_0_0 {
            layout(offset=0) uint src[256];
        };
        layout(std430, binding=1) writeonly restrict buffer _DstBuf_1_0 {
            layout(offset=0) uint dst[256];
        };
        layout (local_size_x = 256, local_size_y = 1) in;

        void main() {
            uint word = src[_const_2_0 + gl_GlobalInvocationID.x];
            word = (word & 0xFF00FF00u) >> 8 |
                (word & 0x00FF00FFu) << 8;
            dst[_const_3_0 + gl_GlobalInvocationID.x] = word;
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet descriptor_set_0(m_device,
                                         {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}});
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set_0.layout_});
    ASSERT_TRUE(pipeline_layout.initialized());

    auto pipeline_info = LvlInitStruct<VkComputePipelineCreateInfo>();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = cs.GetStageCreateInfo();

    VkPipeline pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);

    VkBufferObj buffer_0;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_ci.size = 262144;
    buffer_0.init(*m_device, buffer_ci);
    VkBufferObj buffer_1;
    buffer_1.init(*m_device, buffer_ci);

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set_0.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VkDescriptorBufferInfo buffer_info_0 = {buffer_0.handle(), 0, 1024};
    descriptor_writes[0].pBufferInfo = &buffer_info_0;

    descriptor_writes[1] = descriptor_writes[0];
    descriptor_writes[1].dstBinding = 1;
    VkDescriptorBufferInfo buffer_info_1 = {buffer_1.handle(), 0, 1024};
    descriptor_writes[1].pBufferInfo = &buffer_info_1;

    vk::UpdateDescriptorSets(device(), 2, descriptor_writes, 0, nullptr);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);

    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyPipeline(m_device->device(), pipeline, nullptr);
}

TEST_F(PositiveGpuAssistedLayer, SetSSBOPushDescriptor) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdPushDescriptorSetKHR instead of vkUpdateDescriptorSets");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, &validation_features));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    if (IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on Shield TV";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceProperties properties = {};
    vk::GetPhysicalDeviceProperties(gpu(), &properties);
    if (properties.limits.maxBoundDescriptorSets < 8) {
        GTEST_SKIP() << "maxBoundDescriptorSets is too low";
    }

    char const *csSource = R"glsl(
        #version 450
        layout(constant_id=0) const uint _const_2_0 = 1;
        layout(constant_id=1) const uint _const_3_0 = 1;
        layout(std430, binding=0) readonly restrict buffer _SrcBuf_0_0 {
            layout(offset=0) uint src[256];
        };
        layout(std430, binding=1) writeonly restrict buffer _DstBuf_1_0 {
            layout(offset=0) uint dst[256];
        };
        layout (local_size_x = 256, local_size_y = 1) in;

        void main() {
            uint word = src[_const_2_0 + gl_GlobalInvocationID.x];
            word = (word & 0xFF00FF00u) >> 8 |
                (word & 0x00FF00FFu) << 8;
            dst[_const_3_0 + gl_GlobalInvocationID.x] = word;
        }
    )glsl";

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet descriptor_set_0(m_device,
                                         {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                         VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set_0.layout_});
    ASSERT_TRUE(pipeline_layout.initialized());

    auto pipeline_info = LvlInitStruct<VkComputePipelineCreateInfo>();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = cs.GetStageCreateInfo();

    VkPipeline pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);

    VkBufferObj buffer_0;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_ci.size = 262144;
    buffer_0.init(*m_device, buffer_ci);
    VkBufferObj buffer_1;
    buffer_1.init(*m_device, buffer_ci);

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = 0;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VkDescriptorBufferInfo buffer_info_0 = {buffer_0.handle(), 0, 1024};
    descriptor_writes[0].pBufferInfo = &buffer_info_0;

    descriptor_writes[1] = descriptor_writes[0];
    descriptor_writes[1].dstBinding = 1;
    VkDescriptorBufferInfo buffer_info_1 = {buffer_1.handle(), 0, 1024};
    descriptor_writes[1].pBufferInfo = &buffer_info_1;

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdPushDescriptorSetKHR(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 2,
                                descriptor_writes);

    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyPipeline(m_device->device(), pipeline, nullptr);
}

TEST_F(PositiveGpuAssistedLayer, GpuBufferDeviceAddress) {
    TEST_DESCRIPTION("Makes sure that writing to a buffer that was created after command buffer record doesn't get OOB error");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, &validation_features));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (IsDriver(VK_DRIVER_ID_MESA_RADV)) {
        GTEST_SKIP() << "This test should not be run on the RADV driver.";
    }
    if (IsDriver(VK_DRIVER_ID_AMD_PROPRIETARY)) {
        GTEST_SKIP() << "This test should not be run on the AMD proprietary driver.";
    }
    auto bda_features = LvlInitStruct<VkPhysicalDeviceBufferDeviceAddressFeaturesKHR>();
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(bda_features);
    features2.features.robustBufferAccess = VK_FALSE;

    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2, pool_flags));
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    uint32_t qfi = 0;
    auto bci = LvlInitStruct<VkBufferCreateInfo>();
    bci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bci.size = 12;  // 64 bit pointer + int
    bci.queueFamilyIndexCount = 1;
    bci.pQueueFamilyIndices = &qfi;
    vk_testing::Buffer buffer0;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer0.init(*m_device, bci, mem_props);

    VkViewport viewport = m_viewports[0];
    VkRect2D scissors = m_scissors[0];

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    auto begin_info = LvlInitStruct<VkCommandBufferBeginInfo>();
    auto hinfo = LvlInitStruct<VkCommandBufferInheritanceInfo>();
    begin_info.pInheritanceInfo = &hinfo;

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set.layout_});
    VkDescriptorBufferInfo buffer_test_buffer_info = {};
    buffer_test_buffer_info.buffer = buffer0.handle();
    buffer_test_buffer_info.offset = 0;
    buffer_test_buffer_info.range = sizeof(uint32_t);

    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.pBufferInfo = &buffer_test_buffer_info;
    vk::UpdateDescriptorSets(m_device->device(), 1, &descriptor_write, 0, NULL);

    char const *shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
        layout(set = 0, binding = 0) uniform ufoo {
            bufStruct data;
            int nWrites;
        } u_info;
        layout(buffer_reference, std140) buffer bufStruct {
            int a[4];
        };
        void main() {
            for (int i=0; i < u_info.nWrites; ++i) {
                u_info.data.a[i] = 0xdeadca71;
            }
        }
    )glsl";
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", true);

    VkPipelineObj pipe(m_device);
    pipe.AddShader(&vs);
    pipe.AddDefaultColorAttachment();
    pipe.DisableRasterization();
    auto subcase_err = pipe.CreateVKPipeline(pipeline_layout.handle(), renderPass());
    ASSERT_VK_SUCCESS(subcase_err);

    m_commandBuffer->begin(&begin_info);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdSetViewport(m_commandBuffer->handle(), 0, 1, &viewport);
    vk::CmdSetScissor(m_commandBuffer->handle(), 0, 1, &scissors);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    vk::CmdEndRenderPass(m_commandBuffer->handle());
    m_commandBuffer->end();

    // Make another buffer to write to
    bci.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR;
    bci.size = 64;  // Buffer should be 16*4 = 64 bytes
    auto allocate_flag_info = LvlInitStruct<VkMemoryAllocateFlagsInfo>();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vk_testing::Buffer buffer1(*m_device, bci, mem_props, &allocate_flag_info);

    // Get device address of buffer to write to
    auto pBuffer = buffer1.address();

    auto *data = static_cast<VkDeviceAddress *>(buffer0.memory().map());
    data[0] = pBuffer;
    data[1] = 4;
    buffer0.memory().unmap();

    subcase_err = vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    ASSERT_VK_SUCCESS(subcase_err);
    subcase_err = vk::QueueWaitIdle(m_device->m_queue);
    ASSERT_VK_SUCCESS(subcase_err);
}

// Regression test for semaphore timeout with GPU-AV enabled:
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveGpuAssistedLayer, GetCounterFromSignaledSemaphoreAfterSubmit) {
    TEST_DESCRIPTION("Get counter value from the semaphore signaled by queue submit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, &validation_features));
    if (DeviceValidationVersion() < VK_API_VERSION_1_3) {
        GTEST_SKIP() << "At least Vulkan version 1.3 is required";
    }
    auto sync2_features = LvlInitStruct<VkPhysicalDeviceSynchronization2Features>();
    auto timeline_semaphore_features = LvlInitStruct<VkPhysicalDeviceTimelineSemaphoreFeatures>(&sync2_features);
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &timeline_semaphore_features));

    auto semaphore_type_info = LvlInitStruct<VkSemaphoreTypeCreateInfo>();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const auto create_info = LvlInitStruct<VkSemaphoreCreateInfo>(&semaphore_type_info);
    vk_testing::Semaphore semaphore(*m_device, create_info);

    auto signal_info = LvlInitStruct<VkSemaphoreSubmitInfo>();
    signal_info.semaphore = semaphore;
    signal_info.value = 1;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    auto submit_info = LvlInitStruct<VkSubmitInfo2>();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    ASSERT_VK_SUCCESS(vk::QueueSubmit2(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE));

    std::uint64_t counter = 0;
    ASSERT_VK_SUCCESS(vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
}

TEST_F(PositiveGpuAssistedLayer, MutableBuffer) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdBindDescriptorSets()");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = GetValidationFeatures();
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor, &validation_features));
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }
    if (!CanEnableGpuAV()) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    if (IsPlatform(kShieldTVb)) {
        GTEST_SKIP() << "This test should not run on Shield TV";
    }
    auto mutable_descriptor_type_features = LvlInitStruct<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>();
    GetPhysicalDeviceFeatures2(mutable_descriptor_type_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mutable_descriptor_type_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceProperties properties = {};
    vk::GetPhysicalDeviceProperties(gpu(), &properties);
    if (properties.limits.maxBoundDescriptorSets < 8) {
        GTEST_SKIP() << "maxBoundDescriptorSets is too low";
    }

    char const *csSource = R"glsl(
        #version 450
        layout(constant_id=0) const uint _const_2_0 = 1;
        layout(constant_id=1) const uint _const_3_0 = 1;
        layout(std430, binding=0) readonly restrict buffer _SrcBuf_0_0 {
            layout(offset=0) uint src[256];
        };
        layout(std430, binding=1) writeonly restrict buffer _DstBuf_1_0 {
            layout(offset=0) uint dst[2][256];
        };
        layout (local_size_x = 256, local_size_y = 1) in;

        void main() {
            uint word = src[_const_2_0 + gl_GlobalInvocationID.x];
            word = (word & 0xFF00FF00u) >> 8 |
                (word & 0x00FF00FFu) << 8;
            dst[0][_const_3_0 + gl_GlobalInvocationID.x] = word;
            dst[1][_const_3_0 + gl_GlobalInvocationID.x] = word;
        }
    )glsl";

    VkDescriptorType desc_types[2] = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    };

    VkMutableDescriptorTypeListEXT lists[3] = {};
    lists[1].descriptorTypeCount = 2;
    lists[1].pDescriptorTypes = desc_types;

    auto mdtci = LvlInitStruct<VkMutableDescriptorTypeCreateInfoEXT>();
    mdtci.mutableDescriptorTypeListCount = 3;
    mdtci.pMutableDescriptorTypeLists = lists;

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet descriptor_set_0(m_device,
                                         {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                          {1, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 2, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                         0, &mdtci);

    const VkPipelineLayoutObj pipeline_layout(m_device, {&descriptor_set_0.layout_});
    ASSERT_TRUE(pipeline_layout.initialized());

    auto pipeline_info = LvlInitStruct<VkComputePipelineCreateInfo>();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = cs.GetStageCreateInfo();

    VkPipeline pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);

    VkBufferObj buffer_0;
    auto buffer_ci = LvlInitStruct<VkBufferCreateInfo>();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_ci.size = 262144;
    buffer_0.init(*m_device, buffer_ci);
    VkBufferObj buffer_1;
    buffer_1.init(*m_device, buffer_ci);

    auto descriptor_write = LvlInitStruct<VkWriteDescriptorSet>();
    descriptor_write.dstSet = descriptor_set_0.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VkDescriptorBufferInfo buffer_info_0 = {buffer_0.handle(), 0, 1024};
    descriptor_write.pBufferInfo = &buffer_info_0;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    auto descriptor_copy = LvlInitStruct<VkCopyDescriptorSet>();
    // copy the storage descriptor to the first mutable descriptor
    descriptor_copy.srcSet = descriptor_set_0.set_;
    descriptor_copy.srcBinding = 0;
    descriptor_copy.dstSet = descriptor_set_0.set_;
    descriptor_copy.dstBinding = 1;
    descriptor_copy.dstArrayElement = 1;
    descriptor_copy.descriptorCount = 1;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &descriptor_copy);

    // copy the first mutable descriptor to the second storage desc
    descriptor_copy.srcBinding = 1;
    descriptor_copy.srcArrayElement = 1;
    descriptor_copy.dstBinding = 1;
    descriptor_copy.dstArrayElement = 0;
    vk::UpdateDescriptorSets(device(), 0, nullptr, 1, &descriptor_copy);

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);

    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    auto submit_info = LvlInitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_device->m_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_device->m_queue);

    vk::DestroyPipeline(m_device->device(), pipeline, nullptr);
}
