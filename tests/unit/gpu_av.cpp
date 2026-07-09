/*
 * Copyright (c) 2020-2026 The Khronos Group Inc.
 * Copyright (c) 2020-2026 Valve Corporation
 * Copyright (c) 2020-2026 LunarG, Inc.
 * Copyright (c) 2020-2026 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "layer_validation_tests.h"
#include "pipeline_helper.h"
#include "descriptor_helper.h"
#include "gpu_av_helper.h"

class NegativeGpuAV : public GpuAVTest {};

TEST_F(NegativeGpuAV, ValidationAbort) {
    TEST_DESCRIPTION("GPU validation: Verify that aborting GPU-AV is safe.");
    // GPU Shader Instrumentation requires Vulkan 1.1 or later
    SetTargetApiVersion(VK_API_VERSION_1_0);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFramework(&validation_features));
    m_errorMonitor->SetDesiredError("GPU-AV is being disabled");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();

    // Still make sure we can use Vulkan as expected without errors
    InitRenderTarget();

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(NegativeGpuAV, ValidationFeatures) {
    TEST_DESCRIPTION("Validate Validation Features");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enables;

    auto ici = GetInstanceCreateInfo();
    features.pNext = ici.pNext;
    ici.pNext = &features;
    VkInstance instance;
    m_errorMonitor->SetDesiredError("VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-02967");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, UseAllDescriptorSlotsPipelineNotReserved) {
    TEST_DESCRIPTION("Don't reserve a descriptor slot and proceed to use them all so GPU-AV can't");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    // not using VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
    const VkValidationFeatureEnableEXT gpu_av_enables = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
    VkValidationFeaturesEXT validation_features = vku::InitStructHelper();
    validation_features.enabledValidationFeatureCount = 1;
    validation_features.pEnabledValidationFeatures = &gpu_av_enables;
    RETURN_IF_SKIP(InitFramework(&validation_features));
    if (!CanEnableGpuAV(*this)) {
        GTEST_SKIP() << "Requirements for GPU-AV are not met";
    }
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vkt::Buffer block_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress*>(in_buffer.Memory().Map());
    data[0] = block_buffer.Address();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets;

    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout*> empty_layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            empty_layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout bad_pipe_layout(*m_device, empty_layouts);
        m_errorMonitor->VerifyFound();
    }

    // Reduce by one (so there is room now) and do something invalid. (To make sure things still work as expected)
    std::vector<const vkt::DescriptorSetLayout*> layouts(set_limit - 1);
    for (uint32_t i = 0; i < set_limit - 1; i++) {
        layouts[i] = &descriptor_set.layout_;
    }
    vkt::PipelineLayout pipe_layout(*m_device, layouts);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer foo {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipe_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-PhysicalStorageBuffer64-11819");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, UseAllDescriptorSlotsPipelineReserved) {
    TEST_DESCRIPTION("Reserve a descriptor slot and proceed to use them all anyway so GPU-AV can't");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vkt::Buffer index_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer storage_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    auto data = static_cast<VkDeviceAddress*>(storage_buffer.Memory().Map());
    data[0] = index_buffer.Address();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // Add one to use the descriptor slot we tried to reserve
    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets + 1;

    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout*> empty_layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            empty_layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout bad_pipe_layout(*m_device, empty_layouts);
        m_errorMonitor->VerifyFound();
    }

    // Reduce by one (so there is room now) and do something invalid. (To make sure things still work as expected)
    std::vector<const vkt::DescriptorSetLayout*> layouts(set_limit - 1);
    for (uint32_t i = 0; i < set_limit - 1; i++) {
        layouts[i] = &descriptor_set.layout_;
    }
    vkt::PipelineLayout pipe_layout(*m_device, layouts);

    const char* shader_source = R"glsl(
        #version 450
        #extension GL_EXT_buffer_reference : enable
        layout(buffer_reference, std430) readonly buffer IndexBuffer {
            int indices[];
        };
        layout(set = 0, binding = 0) buffer storage_buffer {
            IndexBuffer data;
            int x;
        };
        void main()  {
            x = data.indices[16];
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipe_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_errorMonitor->SetDesiredError("VUID-RuntimeSpirv-PhysicalStorageBuffer64-11819");
    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, ForceUniformAndStorageBuffer8BitAccess) {
    TEST_DESCRIPTION("Make sure that GPU-AV enabled storageBuffer8BitAccess on behalf of app");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::fragmentStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::vertexPipelineStoresAndAtomics);
    AddRequiredFeature(vkt::Feature::shaderInt64);

    std::vector<VkLayerSettingEXT> layer_settings(1);
    layer_settings[0] = {OBJECT_LAYER_NAME, "gpuav_acceleration_structures_builds", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkFalse};
    RETURN_IF_SKIP(InitGpuAvFramework(layer_settings));

    if (!DeviceExtensionSupported(VK_KHR_8BIT_STORAGE_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_8BIT_STORAGE_EXTENSION_NAME << " not supported, skipping test";
    }

    VkPhysicalDevice8BitStorageFeaturesKHR eight_bit_storage_features = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2 features_2 = vku::InitStructHelper(&eight_bit_storage_features);
    vk::GetPhysicalDeviceFeatures2(Gpu(), &features_2);
    if (!eight_bit_storage_features.storageBuffer8BitAccess) {
        GTEST_SKIP() << "Required feature storageBuffer8BitAccess is not supported, skipping test";
    }

    m_errorMonitor->SetDesiredWarning(
        "Adding a VkPhysicalDevice8BitStorageFeatures to pNext with storageBuffer8BitAccess set to VK_TRUE");

    // noise from disabling settings when features are not supported
    m_errorMonitor->SetAllowedFailureMsg("Disabling");
    m_errorMonitor->SetAllowedFailureMsg(
        "vkGetDeviceProcAddr(): pName is trying to grab vkGetPhysicalDeviceCalibrateableTimeDomainsKHR which is an instance level "
        "function");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, UseAllDescriptorSlotsPipelineLayout) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    // Use robustness to make sure we don't crash as we won't catch the invalid shader
    AddRequiredFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vkt::Buffer buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // Add one to use the descriptor slot we tried to reserve
    const uint32_t set_limit = m_device->Physical().limits_.maxBoundDescriptorSets + 1;

    std::vector<const vkt::DescriptorSetLayout*> empty_layouts(set_limit);
    for (uint32_t i = 0; i < set_limit; i++) {
        empty_layouts[i] = &descriptor_set.layout_;
    }

    m_errorMonitor->SetAllowedFailureMsg("This Pipeline Layout has too many descriptor sets");
    vkt::PipelineLayout bad_pipe_layout(*m_device, empty_layouts);

    const char* shader_source = R"glsl(
        #version 450
        layout(set = 0, binding = 0) buffer foo {
            int x;
            int indices[];
        };
        void main()  {
            x = indices[64]; // OOB
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.cp_ci_.layout = bad_pipe_layout;
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, bad_pipe_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    // normally would produce a VUID-vkCmdDispatch-storageBuffers-06936 warning, but we didn't instrument the shader in the end
    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(NegativeGpuAV, RemoveGpuAvInPresenceOfSyncVal) {
    TEST_DESCRIPTION("Disabling GPU-AV when requirements are not met and sync val is on should not cause a crash");

    SetTargetApiVersion(VK_API_VERSION_1_0);  // GPU-AV needs >1.1

    const std::array validation_enables = {
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };
    const std::array validation_disables = {
        VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT, VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
        VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT};

    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    VkValidationFeaturesEXT validation_features = vku::InitStructHelper();
    validation_features.enabledValidationFeatureCount = size32(validation_enables);
    validation_features.pEnabledValidationFeatures = validation_enables.data();
    if (m_gpuav_disable_core) {
        validation_features.disabledValidationFeatureCount = size32(validation_disables);
        validation_features.pDisabledValidationFeatures = validation_disables.data();
    }

    RETURN_IF_SKIP(InitFramework(&validation_features));

    m_errorMonitor->SetDesiredError("UNASSIGNED-GPU-Assisted-Validation");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, BadDestroy) {
    TEST_DESCRIPTION(
        "In PreCallRecordDestroyDevice, make sure CommandBufferSubState is destroyed before destroying device state and validation "
        "does not crash");
    RETURN_IF_SKIP(InitGpuAvFramework());

    // Workaround for overzealous layers checking even the guaranteed 0th queue family
    const auto q_props = vkt::PhysicalDevice(Gpu()).queue_properties_;
    ASSERT_TRUE(q_props.size() > 0);
    ASSERT_TRUE(q_props[0].queueCount > 0);

    const float q_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_ci = vku::InitStructHelper();
    queue_ci.queueFamilyIndex = 0;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = q_priority;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    VkDevice leaky_device;
    ASSERT_EQ(VK_SUCCESS, vk::CreateDevice(Gpu(), &device_ci, nullptr, &leaky_device));

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo pool_create_info = vku::InitStructHelper();
    pool_create_info.queueFamilyIndex = 0;
    vk::CreateCommandPool(leaky_device, &pool_create_info, nullptr, &command_pool);

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo command_buffer_allocate_info = vku::InitStructHelper();
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vk::AllocateCommandBuffers(leaky_device, &command_buffer_allocate_info, &command_buffer);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyDevice-device-05137");
    // Those 2 will come from self validation if it is enabled
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkDestroyDevice-device-05137");
    vk::DestroyDevice(leaky_device, nullptr);
    m_errorMonitor->VerifyFound();

    // There's no way we can destroy the command pool at this point. Even though DestroyDevice failed, the loader has already
    // removed references to the device
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkDestroyDevice-device-05137");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkDestroyInstance-instance-00629");
}

TEST_F(NegativeGpuAV, LeakedResource) {
    TEST_DESCRIPTION("https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/10218");
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    const char* cs_source = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform sampler2D samplerColor[2];
        void main() {
           vec4 color = texture(samplerColor[1], vec2(0.0));
        }
    )glsl";

    vkt::Image image(*m_device, 32, 32, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    image.SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkImageViewCreateInfo view_ci = vku::InitStructHelper();
    view_ci.image = image;
    view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageView image_view;
    vk::CreateImageView(device(), &view_ci, nullptr, &image_view);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vk::CreateSampler(device(), &sampler_ci, nullptr, &sampler);

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
    pipe.CreateComputePipeline();

    pipe.descriptor_set_.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    pipe.descriptor_set_.WriteDescriptorImageInfo(0, image_view, sampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkDestroyDevice-device-05137");  // sampler
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkDestroyDevice-device-05137");  // imageView
}

TEST_F(NegativeGpuAV, ValidationAbortAndLeakedResource) {
    // GPU Shader Instrumentation requires Vulkan 1.1 or later
    SetTargetApiVersion(VK_API_VERSION_1_0);
    VkValidationFeaturesEXT validation_features = GetGpuAvValidationFeatures();
    RETURN_IF_SKIP(InitFramework(&validation_features));
    m_errorMonitor->SetDesiredError("GPU-AV is being disabled");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();
    InitRenderTarget();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    VkSampler sampler;
    vk::CreateSampler(device(), &sampler_ci, nullptr, &sampler);

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkDestroyDevice-device-05137");
}

TEST_F(NegativeGpuAV, LeakDeviceMemory) {
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    VkBufferCreateInfo buff_ci = vku::InitStructHelper();
    buff_ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buff_ci.size = 256u;
    vkt::Buffer buffer(*m_device, buff_ci, vkt::no_mem);

    VkMemoryRequirements mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = mem_reqs.size;
    m_device->Physical().SetMemoryType(mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkDeviceMemory device_memory;
    vk::AllocateMemory(device(), &alloc_info, nullptr, &device_memory);

    m_errorMonitor->SetUnexpectedError("VUID-vkDestroyDevice-device-05137");
}
