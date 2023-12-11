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
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "../framework/gpu_av_helper.h"
#include "../../layers/gpu_shaders/gpu_shaders_constants.h"

static const std::array gpu_av_enables = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                          VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
static const std::array gpu_av_disables = {VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
                                           VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};

// All GpuAVTest should use this for setup as a single access point to more easily toggle which validation features are
// enabled/disabled
VkValidationFeaturesEXT GpuAVTest::GetGpuAvValidationFeatures() {
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = size32(gpu_av_enables);
    features.pEnabledValidationFeatures = gpu_av_enables.data();
    if (!m_gpuav_enable_core) {
        features.disabledValidationFeatureCount = size32(gpu_av_disables);
        features.pDisabledValidationFeatures = gpu_av_disables.data();
    }
    return features;
}

// This checks any requirements needed for GPU-AV are met otherwise devices not meeting them will "fail" the tests
void GpuAVTest::InitGpuAvFramework(void *p_next) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    validation_features.pNext = p_next;
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
}

TEST_F(PositiveGpuAV, SetSSBOBindDescriptor) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdBindDescriptorSets()");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

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

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkBufferUsageFlags buffer_usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    uint32_t buffer_size = 262144;
    vkt::Buffer buffer_0(*m_device, buffer_size, buffer_usage);
    vkt::Buffer buffer_1(*m_device, buffer_size, buffer_usage);

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer_0.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(1, buffer_1.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveGpuAV, SetSSBOPushDescriptor) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdPushDescriptorSetKHR instead of vkUpdateDescriptorSets");
    AddRequiredExtensions(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

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
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set_0.layout_});

    VkComputePipelineCreateInfo pipeline_info = vku::InitStructHelper();
    pipeline_info.flags = 0;
    pipeline_info.layout = pipeline_layout.handle();
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.stage = cs.GetStageCreateInfo();

    VkPipeline pipeline;
    vk::CreateComputePipelines(device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_ci.size = 262144;
    vkt::Buffer buffer_0(*m_device, buffer_ci);
    vkt::Buffer buffer_1(*m_device, buffer_ci);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set_0.layout_});
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkWriteDescriptorSet descriptor_writes[2];
    descriptor_writes[0] = vku::InitStructHelper();
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

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);

    vk::DestroyPipeline(m_device->device(), pipeline, nullptr);
}

TEST_F(PositiveGpuAV, BufferDeviceAddress) {
    TEST_DESCRIPTION("Makes sure that writing to a buffer that was created after command buffer record doesn't get OOB error");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bda_features = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2KHR features2 = GetPhysicalDeviceFeatures2(bda_features);
    if (!features2.features.shaderInt64) {
        GTEST_SKIP() << "shaderInt64 is not supported";
    }
    features2.features.robustBufferAccess = VK_FALSE;

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    // Make a uniform buffer to be passed to the shader that contains the pointer and write count
    uint32_t buffer_size = 12;  // 64 bit pointer + int
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);

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
    VkShaderObj vs(this, shader_source, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_ = {vs.GetStageCreateInfo()};
    pipe.rs_state_ci_.rasterizerDiscardEnable = VK_TRUE;
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.CreateGraphicsPipeline();

    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer0.handle(), 0, sizeof(uint32_t));
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();

    // Make another buffer to write to
    buffer_size = 64;  // Buffer should be 16*4 = 64 bytes
    VkMemoryAllocateFlagsInfo allocate_flag_info = vku::InitStructHelper();
    allocate_flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    vkt::Buffer buffer1(*m_device, buffer_size, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR, mem_props, &allocate_flag_info);

    // Get device address of buffer to write to
    auto pBuffer = buffer1.address();

    auto *data = static_cast<VkDeviceAddress *>(buffer0.memory().map());
    data[0] = pBuffer;
    data[1] = 4;
    buffer0.memory().unmap();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();
    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

// Regression test for semaphore timeout with GPU-AV enabled:
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4968
TEST_F(PositiveGpuAV, GetCounterFromSignaledSemaphoreAfterSubmit) {
    TEST_DESCRIPTION("Get counter value from the semaphore signaled by queue submit");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceSynchronization2Features sync2_features = vku::InitStructHelper();
    VkPhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore_features = vku::InitStructHelper(&sync2_features);
    GetPhysicalDeviceFeatures2(timeline_semaphore_features);
    RETURN_IF_SKIP(InitState(nullptr, &timeline_semaphore_features));

    VkSemaphoreTypeCreateInfo semaphore_type_info = vku::InitStructHelper();
    semaphore_type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    const VkSemaphoreCreateInfo create_info = vku::InitStructHelper(&semaphore_type_info);
    vkt::Semaphore semaphore(*m_device, create_info);

    VkSemaphoreSubmitInfo signal_info = vku::InitStructHelper();
    signal_info.semaphore = semaphore;
    signal_info.value = 1;
    signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submit_info = vku::InitStructHelper();
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &signal_info;
    ASSERT_EQ(VK_SUCCESS, vk::QueueSubmit2(m_default_queue, 1, &submit_info, VK_NULL_HANDLE));

    std::uint64_t counter = 0;
    ASSERT_EQ(VK_SUCCESS, vk::GetSemaphoreCounterValue(*m_device, semaphore, &counter));
}

TEST_F(PositiveGpuAV, MutableBuffer) {
    TEST_DESCRIPTION("Makes sure we can use vkCmdBindDescriptorSets()");
    AddRequiredExtensions(VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutable_descriptor_type_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(mutable_descriptor_type_features);
    RETURN_IF_SKIP(InitState(nullptr, &mutable_descriptor_type_features));
    InitRenderTarget();

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

    VkMutableDescriptorTypeCreateInfoEXT mdtci = vku::InitStructHelper();
    mdtci.mutableDescriptorTypeListCount = 3;
    mdtci.pMutableDescriptorTypeLists = lists;

    VkShaderObj cs(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);

    OneOffDescriptorSet descriptor_set_0(m_device,
                                         {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
                                          {1, VK_DESCRIPTOR_TYPE_MUTABLE_EXT, 2, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}},
                                         0, &mdtci);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, csSource, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set_0.layout_});
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkBufferUsageFlags buffer_usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    uint32_t buffer_size = 262144;
    vkt::Buffer buffer_0(*m_device, buffer_size, buffer_usage);
    vkt::Buffer buffer_1(*m_device, buffer_size, buffer_usage);

    VkWriteDescriptorSet descriptor_write = vku::InitStructHelper();
    descriptor_write.dstSet = descriptor_set_0.set_;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    VkDescriptorBufferInfo buffer_info_0 = {buffer_0.handle(), 0, 1024};
    descriptor_write.pBufferInfo = &buffer_info_0;

    vk::UpdateDescriptorSets(device(), 1, &descriptor_write, 0, nullptr);

    VkCopyDescriptorSet descriptor_copy = vku::InitStructHelper();
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
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &descriptor_set_0.set_, 0, nullptr);

    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_commandBuffer->end();

    VkSubmitInfo submit_info = vku::InitStructHelper();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveGpuAV, MaxDescriptorsClamp) {
    TEST_DESCRIPTION("Make sure maxUpdateAfterBindDescriptorsInAllPools is clamped");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    auto desc_indexing_props = vku::InitStruct<VkPhysicalDeviceDescriptorIndexingProperties>();
    auto props2 = vku::InitStruct<VkPhysicalDeviceProperties2>(&desc_indexing_props);

    vk::GetPhysicalDeviceProperties2(gpu(), &props2);

    ASSERT_GE(gpuav::glsl::kDebugInputBindlessMaxDescriptors, desc_indexing_props.maxUpdateAfterBindDescriptorsInAllPools);
}

TEST_F(PositiveGpuAV, MaxDescriptorsClamp13) {
    TEST_DESCRIPTION("Make sure maxUpdateAfterBindDescriptorsInAllPools is clamped");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    auto vk12_props = vku::InitStruct<VkPhysicalDeviceVulkan12Properties>();
    auto props2 = vku::InitStruct<VkPhysicalDeviceProperties2>(&vk12_props);

    vk::GetPhysicalDeviceProperties2(gpu(), &props2);

    ASSERT_GE(gpuav::glsl::kDebugInputBindlessMaxDescriptors, vk12_props.maxUpdateAfterBindDescriptorsInAllPools);
}

TEST_F(PositiveGpuAV, UnInitImage) {
    TEST_DESCRIPTION("Make sure there's not a crash if the sampler of a combined image sampler is initialized by the image isn't.");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    auto maintenance4_features = vku::InitStruct<VkPhysicalDeviceMaintenance4Features>();
    maintenance4_features.maintenance4 = true;
    auto features2 = vku::InitStruct<VkPhysicalDeviceFeatures2KHR>(&maintenance4_features);
    auto indexing_features = vku::InitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    maintenance4_features.pNext = &indexing_features;
    GetPhysicalDeviceFeatures2(features2);

    if (!indexing_features.runtimeDescriptorArray || !indexing_features.descriptorBindingSampledImageUpdateAfterBind ||
        !indexing_features.descriptorBindingPartiallyBound || !indexing_features.descriptorBindingVariableDescriptorCount ||
        !indexing_features.shaderSampledImageArrayNonUniformIndexing ||
        !indexing_features.shaderStorageBufferArrayNonUniformIndexing) {
        GTEST_SKIP() << "Not all descriptor indexing features supported, skipping descriptor indexing tests";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
    InitRenderTarget();

    const uint32_t buffer_size = 1024;
    VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    // Make a uniform buffer to be passed to the shader that contains the invalid array index.
    vkt::Buffer buffer0(*m_device, buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, mem_props);
    // Make another buffer to populate the buffer array to be indexed
    vkt::Buffer buffer1(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, mem_props);

    void *layout_pnext = nullptr;
    void *allocate_pnext = nullptr;
    auto pool_create_flags = 0;
    auto layout_create_flags = 0;
    VkDescriptorBindingFlagsEXT ds_binding_flags[3] = {};
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT layout_createinfo_binding_flags[1] = {};
    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = 0;
    ds_binding_flags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    layout_createinfo_binding_flags[0] = vku::InitStruct<VkDescriptorSetLayoutBindingFlagsCreateInfo>();
    layout_createinfo_binding_flags[0].bindingCount = 3;
    layout_createinfo_binding_flags[0].pBindingFlags = ds_binding_flags;
    layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    pool_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
    layout_pnext = layout_createinfo_binding_flags;

    layout_create_flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    pool_create_flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;
    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    const VkSampler samplers[2] = {sampler.handle(), sampler.handle()};

    OneOffDescriptorSet descriptor_set_variable(m_device,
                                                {
                                                {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 5, VK_SHADER_STAGE_ALL, nullptr},
                                                {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_ALL, samplers},
                                                },
                                                layout_create_flags, layout_pnext, pool_create_flags, allocate_pnext);

    const vkt::PipelineLayout pipeline_layout_variable(*m_device, {&descriptor_set_variable.layout_});
    VkImageObj image(m_device);
    image.Init(16, 16, 1, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_LINEAR);
    vkt::ImageView image_view = image.CreateView();

    VkDescriptorBufferInfo buffer_info[1] = {};
    buffer_info[0].buffer = buffer0.handle();
    buffer_info[0].offset = 0;
    buffer_info[0].range = sizeof(uint32_t);

    VkDescriptorImageInfo image_info[1] = {};
    image_info[0] = {VK_NULL_HANDLE, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkWriteDescriptorSet descriptor_writes[2] = {};
    descriptor_writes[0] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[0].dstSet = descriptor_set_variable.set_;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].pBufferInfo = buffer_info;
    descriptor_writes[1] = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes[1].dstSet = descriptor_set_variable.set_;
    descriptor_writes[1].dstBinding = 2;
    descriptor_writes[1].dstArrayElement = 1;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].pImageInfo = image_info;
    vk::UpdateDescriptorSets(m_device->device(), 2, descriptor_writes, 0, NULL);

    ds_binding_flags[0] = 0;
    ds_binding_flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT;

    // - The vertex shader fetches the invalid index from the uniform buffer and passes it to the fragment shader.
    // - The fragment shader makes the invalid array access.
    char const *vsSource_frag = R"glsl(
        #version 450

        layout(std140, binding = 0) uniform foo { uint tex_index[1]; } uniform_index_buffer;
        layout(location = 0) out flat uint index;
        vec2 vertices[3];
        void main(){
              vertices[0] = vec2(-1.0, -1.0);
              vertices[1] = vec2( 1.0, -1.0);
              vertices[2] = vec2( 0.0,  1.0);
           gl_Position = vec4(vertices[gl_VertexIndex % 3], 0.0, 1.0);
           index = uniform_index_buffer.tex_index[0];
        }
    )glsl";
    char const *fsSource_frag_runtime = R"glsl(
        #version 450
        #extension GL_EXT_nonuniform_qualifier : enable

        layout(set = 0, binding = 2) uniform sampler2D tex[];
        layout(location = 0) out vec4 uFragColor;
        layout(location = 0) in flat uint index;
        void main(){
           uFragColor = texture(tex[index], vec2(0, 0));
        }
    )glsl";
    struct TestCase {
        char const *vertex_source;
        char const *fragment_source;
        bool debug;
        const vkt::PipelineLayout *pipeline_layout;
        const OneOffDescriptorSet *descriptor_set;
        uint32_t index;
    };

    std::vector<TestCase> tests;

    tests.push_back({vsSource_frag, fsSource_frag_runtime, false, &pipeline_layout_variable,
                    &descriptor_set_variable, 1});

    VkSubmitInfo submit_info = vku::InitStruct<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer->handle();

    for (const auto &iter : tests) {
        VkShaderObj vs(this, iter.vertex_source, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                       iter.debug);
        VkShaderObj fs(this, iter.fragment_source, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr,
                       "main", iter.debug);
        CreatePipelineHelper pipe(*this);
        pipe.InitState();
        pipe.shader_stages_.clear();
        pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
        pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
        pipe.gp_ci_.layout = iter.pipeline_layout->handle();
        pipe.CreateGraphicsPipeline();
        m_commandBuffer->begin();
        m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
        vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
        vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, iter.pipeline_layout->handle(), 0, 1,
                                  &iter.descriptor_set->set_, 0, nullptr);
        vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
        vk::CmdEndRenderPass(m_commandBuffer->handle());
        m_commandBuffer->end();
        uint32_t *data = (uint32_t *)buffer0.memory().map();
        data[0] = iter.index;
        buffer0.memory().unmap();

        vk::QueueSubmit(m_default_queue, 1, &submit_info, VK_NULL_HANDLE);
        vk::QueueWaitIdle(m_default_queue);
    }
    return;
}

TEST_F(PositiveGpuAV, SelectInstrumentedShaders) {
    TEST_DESCRIPTION("Use a bad vertex shader, but don't select it for validation and make sure we don't get a buffer oob warning");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    const VkBool32 value = true;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
                                       &value};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                               &setting};
    RETURN_IF_SKIP(InitGpuAvFramework(&layer_settings_create_info));
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.robustBufferAccess) {
        GTEST_SKIP() << "Not safe to write outside of buffer memory";
    }
    // Robust buffer access will be on by default
    VkCommandPoolCreateFlags pool_flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    InitState(nullptr, nullptr, pool_flags);
    InitRenderTarget();

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    vkt::Buffer write_buffer(*m_device, 4, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, reqs);
    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});

    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});
    descriptor_set.WriteDescriptorBufferInfo(0, write_buffer.handle(), 0, 4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();
    static const char vertshader[] = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer StorageBuffer { uint data[]; } Data;
        void main() {
                Data.data[4] = 0xdeadca71;
        }
    )glsl";
    // Don't instrument buggy vertex shader
    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    // Instrument non-buggy fragment shader
    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    VkShaderObj fs(this, vertshader, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main", false,
                   &features);
    CreatePipelineHelper pipe(*this);
    pipe.InitState();
    pipe.shader_stages_.clear();
    pipe.shader_stages_.push_back(vs.GetStageCreateInfo());
    pipe.shader_stages_.push_back(fs.GetStageCreateInfo());
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_commandBuffer->handle(), 3, 1, 0, 0);
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
    // Should not get a warning since buggy vertex shader wasn't instrumented
    m_errorMonitor->ExpectSuccess(kWarningBit | kErrorBit);
    m_commandBuffer->QueueCommandBuffer();
    vk::QueueWaitIdle(m_default_queue);
}

TEST_F(PositiveGpuAV, BindingPartiallyBound) {
    TEST_DESCRIPTION("Ensure that no validation errors for invalid descriptors if binding is PARTIALLY_BOUND");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    auto indexing_features = vku::InitStruct<VkPhysicalDeviceDescriptorIndexingFeaturesEXT>();
    auto features2 = vku::InitStruct<VkPhysicalDeviceFeatures2KHR>(&indexing_features);
    GetPhysicalDeviceFeatures2(features2);

    if (!indexing_features.descriptorBindingPartiallyBound) {
        GTEST_SKIP() << "Partially bound bindings not supported, skipping test";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2));
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

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint32_t *data = (uint32_t*)buffer.memory().map();
    data[0] = 0;
    buffer.memory().unmap();

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // Only update binding 0
    descriptor_set.WriteDescriptorBufferInfo(0, buffer.handle(), 0, sizeof(uint32_t));
    descriptor_set.UpdateDescriptorSets();

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

    m_commandBuffer->begin();
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

TEST_F(PositiveGpuAV, DrawingWithUnboundUnusedSet) {
    TEST_DESCRIPTION(
        "Test issuing draw command with pipeline layout that has 2 descriptor sets with first descriptor set begin unused and "
        "unbound.");
    AddRequiredExtensions(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());

    RETURN_IF_SKIP(InitState());
    InitRenderTarget();
    if (DeviceValidationVersion() != VK_API_VERSION_1_1) {
        GTEST_SKIP() << "Tests requires Vulkan 1.1 exactly";
    }

    char const *fs_source = R"glsl(
        #version 450
        layout (set = 1, binding = 0) uniform sampler2D samplerColor;
        layout(location = 0) out vec4 color;
        void main() {
           color = texture(samplerColor, gl_FragCoord.xy);
           color += texture(samplerColor, gl_FragCoord.wz);
        }
    )glsl";
    VkShaderObj fs(this, fs_source, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::ImageView imageView = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    OneOffDescriptorSet descriptor_set(m_device,
                                       {
                                           {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                       });
    descriptor_set.WriteDescriptorImageInfo(0, imageView, sampler.handle());
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer indirect_buffer(*m_device, sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer indexed_indirect_buffer(*m_device, sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer count_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::Buffer index_buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    pipe.pipeline_layout_ = vkt::PipelineLayout(*m_device, {&descriptor_set.layout_, &descriptor_set.layout_});
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 1, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindIndexBuffer(m_commandBuffer->handle(), index_buffer.handle(), 0, VK_INDEX_TYPE_UINT32);
    vk::CmdDrawIndexedIndirectCountKHR(m_commandBuffer->handle(), indexed_indirect_buffer.handle(), 0, count_buffer.handle(), 0, 1,
                                       sizeof(VkDrawIndexedIndirectCommand));
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
