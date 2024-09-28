/*
 * Copyright (c) 2020-2024 The Khronos Group Inc.
 * Copyright (c) 2020-2024 Valve Corporation
 * Copyright (c) 2020-2024 LunarG, Inc.
 * Copyright (c) 2020-2024 Google, Inc.
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
#include "../framework/gpu_av_helper.h"

class NegativeGpuAV : public GpuAVTest {};

TEST_F(NegativeGpuAV, ValidationAbort) {
    TEST_DESCRIPTION("GPU validation: Verify that aborting GPU-AV is safe.");
    RETURN_IF_SKIP(InitGpuAvFramework());

    PFN_vkSetPhysicalDeviceFeaturesEXT fpvkSetPhysicalDeviceFeaturesEXT = nullptr;
    PFN_vkGetOriginalPhysicalDeviceFeaturesEXT fpvkGetOriginalPhysicalDeviceFeaturesEXT = nullptr;
    if (!LoadDeviceProfileLayer(fpvkSetPhysicalDeviceFeaturesEXT, fpvkGetOriginalPhysicalDeviceFeaturesEXT)) {
        GTEST_SKIP() << "Failed to load device profile layer.";
    }

    VkPhysicalDeviceFeatures features = {};
    fpvkGetOriginalPhysicalDeviceFeaturesEXT(gpu(), &features);

    // Disable features necessary for GPU-AV so initialization aborts
    features.vertexPipelineStoresAndAtomics = false;
    features.fragmentStoresAndAtomics = false;
    fpvkSetPhysicalDeviceFeaturesEXT(gpu(), features);
    m_errorMonitor->SetDesiredError("GPU-AV is being disabled");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();

    // Still make sure we can use Vulkan as expected without errors
    InitRenderTarget();

    CreateComputePipelineHelper pipe(*this);
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
}

TEST_F(NegativeGpuAV, ValidationFeatures) {
    TEST_DESCRIPTION("Validate Validation Features");
    SetTargetApiVersion(VK_API_VERSION_1_1);
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

    VkValidationFeatureEnableEXT printf_enables[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                                     VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    features.pEnabledValidationFeatures = printf_enables;
    features.enabledValidationFeatureCount = 2;
    m_errorMonitor->SetDesiredError("VUID-VkValidationFeaturesEXT-pEnabledValidationFeatures-02968");
    vk::CreateInstance(&ici, nullptr, &instance);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, SelectInstrumentedShaders) {
    TEST_DESCRIPTION("GPU validation: Validate selection of which shaders get instrumented for GPU-AV");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    const VkBool32 value = true;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "gpuav_select_instrumented_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1,
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

    VkShaderObj vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT);
    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_[0] = vs.GetStageCreateInfo();
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    VkCommandBufferBeginInfo begin_info = vku::InitStructHelper();
    m_command_buffer.begin(&begin_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();
    // Should not get a warning since shader wasn't instrumented
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    VkValidationFeatureEnableEXT enabled[] = {VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT};
    VkValidationFeaturesEXT features = vku::InitStructHelper();
    features.enabledValidationFeatureCount = 1;
    features.pEnabledValidationFeatures = enabled;
    VkShaderObj instrumented_vs(this, vertshader, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL, nullptr, "main",
                                &features);
    CreatePipelineHelper pipe2(*this);
    pipe2.shader_stages_[0] = instrumented_vs.GetStageCreateInfo();
    pipe2.gp_ci_.layout = pipeline_layout.handle();
    pipe2.CreateGraphicsPipeline();

    m_command_buffer.begin(&begin_info);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.Handle());
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();
    // Should get a warning since shader was instrumented
    m_errorMonitor->ExpectSuccess(kWarningBit | kErrorBit);
    m_errorMonitor->SetDesiredWarning("VUID-vkCmdDraw-storageBuffers-06936", 3);
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, UseAllDescriptorSlotsPipelineNotReserved) {
    TEST_DESCRIPTION("Don't reserve a descriptor slot and proceed to use them all so GPU-AV can't");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);

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
    vkt::Buffer in_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(in_buffer.memory().map());
    data[0] = block_buffer.address();
    in_buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, in_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    const uint32_t set_limit = m_device->phy().limits_.maxBoundDescriptorSets;

    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> empty_layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            empty_layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout bad_pipe_layout(*m_device, empty_layouts);
        m_errorMonitor->VerifyFound();
    }

    // Reduce by one (so there is room now) and do something invalid. (To make sure things still work as expected)
    std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
    for (uint32_t i = 0; i < set_limit - 1; i++) {
        layouts[i] = &descriptor_set.layout_;
    }
    vkt::PipelineLayout pipe_layout(*m_device, layouts);

    char const *shader_source = R"glsl(
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
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipe_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, UseAllDescriptorSlotsPipelineReserved) {
    TEST_DESCRIPTION("Reserve a descriptor slot and proceed to use them all anyway so GPU-AV can't");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    AddDisabledFeature(vkt::Feature::robustBufferAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->ExpectSuccess(kErrorBit | kWarningBit);

    vkt::Buffer index_buffer(*m_device, 16, 0, vkt::device_address);
    vkt::Buffer storage_buffer(*m_device, 16, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto data = static_cast<VkDeviceAddress *>(storage_buffer.memory().map());
    data[0] = index_buffer.address();
    storage_buffer.memory().unmap();

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    descriptor_set.WriteDescriptorBufferInfo(0, storage_buffer.handle(), 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    // Add one to use the descriptor slot we tried to reserve
    const uint32_t set_limit = m_device->phy().limits_.maxBoundDescriptorSets + 1;

    // First try to use too many sets in the pipeline layout
    {
        m_errorMonitor->SetDesiredWarning(
            "This Pipeline Layout has too many descriptor sets that will not allow GPU shader instrumentation to be setup for "
            "pipelines created with it");
        std::vector<const vkt::DescriptorSetLayout *> empty_layouts(set_limit);
        for (uint32_t i = 0; i < set_limit; i++) {
            empty_layouts[i] = &descriptor_set.layout_;
        }
        vkt::PipelineLayout bad_pipe_layout(*m_device, empty_layouts);
        m_errorMonitor->VerifyFound();
    }

    // Reduce by one (so there is room now) and do something invalid. (To make sure things still work as expected)
    std::vector<const vkt::DescriptorSetLayout *> layouts(set_limit - 1);
    for (uint32_t i = 0; i < set_limit - 1; i++) {
        layouts[i] = &descriptor_set.layout_;
    }
    vkt::PipelineLayout pipe_layout(*m_device, layouts);

    char const *shader_source = R"glsl(
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
    pipe.cs_ = std::make_unique<VkShaderObj>(this, shader_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.cp_ci_.layout = pipe_layout.handle();
    pipe.CreateComputePipeline();

    m_command_buffer.begin();
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.Handle());
    vk::CmdDispatch(m_command_buffer.handle(), 1, 1, 1);
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("UNASSIGNED-Device address out of bounds");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO the SPIRV-Tools instrumentation doesn't work for this shader
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6944
TEST_F(NegativeGpuAV, DISABLED_InvalidAtomicStorageOperation) {
    TEST_DESCRIPTION(
        "If storage view use atomic operation, the view's format MUST support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT or "
        "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT ");

    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

    RETURN_IF_SKIP(InitGpuAvFramework());

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(atomic_float_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2));

    if (atomic_float_features.shaderImageFloat32Atomics == VK_FALSE) {
        GTEST_SKIP() << "shaderImageFloat32Atomics not supported.";
    }

    VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT;
    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT to
                                                       // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    auto image_ci = vkt::Image::ImageCreateInfo2D(64, 64, 1, 1, image_format, usage);

    if (ImageFormatIsSupported(instance(), gpu(), image_ci, VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)) {
        GTEST_SKIP() << "Cannot make VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT not supported.";
    }

    VkFormat buffer_view_format =
        VK_FORMAT_R8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT to
                             // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    if (BufferFormatAndFeaturesSupported(gpu(), buffer_view_format, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT)) {
        GTEST_SKIP() << "Cannot make VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT not supported.";
    }
    m_errorMonitor->SetUnexpectedError("VUID-VkBufferViewCreateInfo-format-08779");
    InitRenderTarget();

    VkPhysicalDeviceFeatures device_features = {};
    GetPhysicalDeviceFeatures(&device_features);

    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    vkt::Sampler sampler(*m_device, SafeSaneSamplerCreateInfo());

    vkt::Buffer buffer(*m_device, 64, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);

    VkBufferViewCreateInfo bvci = vku::InitStructHelper();
    bvci.buffer = buffer.handle();
    bvci.format = buffer_view_format;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);

    char const *fsSource = R"glsl(
        #version 450
        layout(set=0, binding=3, r32f) uniform image2D si0;
        layout(set=0, binding=2, r32f) uniform image2D si1[2];
        layout(set = 0, binding = 1, r32f) uniform imageBuffer stb2;
        layout(set = 0, binding = 0, r32f) uniform imageBuffer stb3[2];
        layout(location=0) out vec4 color;
        void main() {
              color += imageAtomicExchange(si1[0], ivec2(0), 1);
              color += imageAtomicExchange(si1[1], ivec2(0), 1);
              color += imageAtomicExchange(stb3[0], 0, 1);
              color += imageAtomicExchange(stb3[1], 0, 1);
        }
    )glsl";

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.CreateGraphicsPipeline();

    g_pipe.descriptor_set_->WriteDescriptorImageInfo(3, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(2, image_view, sampler.handle(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                     VK_IMAGE_LAYOUT_GENERAL, 1);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(1, buffer_view.handle());
    g_pipe.descriptor_set_->WriteDescriptorBufferView(0, buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
    g_pipe.descriptor_set_->WriteDescriptorBufferView(0, buffer_view.handle(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-02691", 2);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-07888", 2);
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO: The SPIRV-Tools instrumentation doesn't work correctly for this shader
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6944
TEST_F(NegativeGpuAV, DISABLED_UnnormalizedCoordinatesInBoundsAccess) {
    TEST_DESCRIPTION("If a samper is unnormalizedCoordinates, but using OpInBoundsAccessChain");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    //#version 450
    //layout (set = 0, binding = 0) uniform sampler2D tex[2];
    //layout(location=0) out vec4 color;
    //
    //void main() {
    //    color = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
    //}
    // but with OpInBoundsAccessChain instead of normal generated OpAccessChain
    const char *fsSource = R"(
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpExtension "SPV_KHR_physical_storage_buffer"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %color "color"
               OpName %tex "tex"
               OpDecorate %color Location 0
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %6 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_11_uint_2 = OpTypeArray %11 %uint_2
%_ptr_UniformConstant__arr_11_uint_2 = OpTypePointer UniformConstant %_arr_11_uint_2
        %tex = OpVariable %_ptr_UniformConstant__arr_11_uint_2 UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %24 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %6
         %25 = OpLabel
         %26 = OpInBoundsAccessChain %_ptr_UniformConstant_11 %tex %int_1
         %27 = OpLoad %11 %26
         %28 = OpImageSampleExplicitLod %v4float %27 %21 Lod|ConstOffset %float_0 %24
               OpStore %color %28
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.CreateGraphicsPipeline();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view_pass = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08611");
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO: The SPIRV-Tools instrumentation doesn't work correctly for this shader
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6944
TEST_F(NegativeGpuAV, DISABLED_UnnormalizedCoordinatesCopyObject) {
    TEST_DESCRIPTION("If a samper is unnormalizedCoordinates, but using OpCopyObject");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);

    // layout (set = 0, binding = 0) uniform sampler2D tex[2];
    // void main() {
    //     vec4 x = textureLodOffset(tex[1], vec2(0), 0, ivec2(0));
    // }
    const char *fsSource = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%ptr_v4float = OpTypePointer Function %v4float
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %11 = OpTypeSampledImage %10
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
       %array = OpTypeArray %11 %uint_2
%ptr_uc_array = OpTypePointer UniformConstant %array
        %tex = OpVariable %ptr_uc_array UniformConstant
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %ptr_uc = OpTypePointer UniformConstant %11
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %24 = OpConstantComposite %v2float %float_0 %float_0
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %27 = OpConstantComposite %v2int %int_0 %int_0
       %main = OpFunction %void None %3
          %5 = OpLabel
          %x = OpVariable %ptr_v4float Function
   %var_copy = OpCopyObject %ptr_v4float %x
         %20 = OpAccessChain %ptr_uc %tex %int_1
    %ac_copy = OpCopyObject %ptr_uc %20
         %21 = OpLoad %11 %ac_copy
  %load_copy = OpCopyObject %11 %21
         %28 = OpImageSampleExplicitLod %v4float %load_copy %24 Lod|ConstOffset %float_0 %27
 %image_copy = OpCopyObject %v4float %28
               OpStore %var_copy %image_copy
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.CreateGraphicsPipeline();

    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView view_pass = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
    g_pipe.descriptor_set_->WriteDescriptorImageInfo(0, view_pass, sampler.handle(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    g_pipe.descriptor_set_->UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08611");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();
}

TEST_F(NegativeGpuAV, UnnormalizedCoordinatesSeparateSamplerSharedSampler) {
    TEST_DESCRIPTION("Doesn't use COMBINED_IMAGE_SAMPLER, but multiple OpLoad share Sampler OpVariable");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // There are 2 OpLoad/OpAccessChain that point the same OpVariable
    const char fsSource[] = R"glsl(
        #version 450
        // VK_DESCRIPTOR_TYPE_SAMPLER
        layout(set = 0, binding = 0) uniform sampler s1;
        // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
        layout(set = 0, binding = 1) uniform texture2D si_good;
        layout(set = 0, binding = 2) uniform texture3D si_bad[2]; // 3D image view

        layout(location=0) out vec4 color;
        void main() {
            vec4 x = texture(sampler2D(si_good, s1), vec2(0));
            vec4 y = texture(sampler3D(si_bad[1], s1), vec3(0));
            color = vec4(x + y);
        }
    )glsl";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.gp_ci_.layout = pipeline_layout.handle();
    g_pipe.CreateGraphicsPipeline();

    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    const VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
    auto image_ci = vkt::Image::ImageCreateInfo2D(128, 128, 1, 1, format, usage);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    image_ci.imageType = VK_IMAGE_TYPE_3D;
    vkt::Image image_3d(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view_3d = image_3d.CreateView(VK_IMAGE_VIEW_TYPE_3D);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.WriteDescriptorImageInfo(2, image_view_3d, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);

    // Core validation triggers these errors, causing the following draw to be skipped.
    // GPU-AV will thus not be able to validate this draw call.
    // Descriptor arrays are not validated in core validation
    // (See call to `descriptor_set.SkipBinding()` in `CoreChecks::ValidateDrawState()`)
    if (!m_gpuav_disable_core) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-08609");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-08610");
    }
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);

    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08609");
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08610");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, ShareOpSampledImage) {
    TEST_DESCRIPTION(
        "Have two OpImageSampleImplicitLod share the same OpSampledImage. This needs to be in the same block post-shader "
        "instrumentation.");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());
    InitRenderTarget();

    // #version 450
    // layout(set = 0, binding = 0) uniform sampler s1;
    // layout(set = 0, binding = 1) uniform texture2D si_good;
    // layout(location=0) out vec4 color;
    // void main() {
    //     color = texture(sampler2D(si_good, s1), vec2(0));
    //     color += texture(sampler2D(si_good, s1), vec2(color.x));
    // }
    const char *fsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %color
               OpExecutionMode %main OriginUpperLeft
               OpDecorate %color Location 0
               OpDecorate %si_good DescriptorSet 0
               OpDecorate %si_good Binding 1
               OpDecorate %s1 DescriptorSet 0
               OpDecorate %s1 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
    %si_good = OpVariable %_ptr_UniformConstant_10 UniformConstant
         %14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
         %s1 = OpVariable %_ptr_UniformConstant_14 UniformConstant
         %18 = OpTypeSampledImage %10
    %v2float = OpTypeVector %float 2
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v2float %float_0 %float_0
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpLoad %10 %si_good
         %17 = OpLoad %14 %s1
              ; the results (%19) needs to be in same block as what consumes it
         %19 = OpSampledImage %18 %13 %17
         %23 = OpImageSampleImplicitLod %v4float %19 %22
               OpStore %color %23
         %30 = OpAccessChain %_ptr_Output_float %color %uint_0
         %31 = OpLoad %float %30
         %32 = OpCompositeConstruct %v2float %31 %31
         %33 = OpImageSampleImplicitLod %v4float %19 %32
         %34 = OpLoad %v4float %color
         %35 = OpFAdd %v4float %34 %33
               OpStore %color %35
               OpReturn
               OpFunctionEnd
    )";
    VkShaderObj vs(this, kVertexDrawPassthroughGlsl, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}});
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.gp_ci_.layout = pipeline_layout.handle();
    g_pipe.CreateGraphicsPipeline();

    auto image_ci = vkt::Image::ImageCreateInfo2D(
        128, 128, 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    vkt::Image image(*m_device, image_ci, vkt::set_layout);
    vkt::ImageView image_view = image.CreateView();

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.unnormalizedCoordinates = VK_TRUE;
    sampler_ci.maxLod = 0;
    vkt::Sampler sampler(*m_device, sampler_ci);

    descriptor_set.WriteDescriptorImageInfo(0, VK_NULL_HANDLE, sampler.handle(), VK_DESCRIPTOR_TYPE_SAMPLER);
    descriptor_set.WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.Handle());

    if (!m_gpuav_disable_core) {
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdDraw-None-08610");
    }
    vk::CmdDraw(m_command_buffer.handle(), 3, 1, 0, 0);
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();

    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-08610");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

// TODO - Indexing is not being recognized by GPU-AV
TEST_F(NegativeGpuAV, DISABLED_YcbcrDrawFetchIndexed) {
    TEST_DESCRIPTION("Do OpImageFetch on a Ycbcr COMBINED_IMAGE_SAMPLER.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    RETURN_IF_SKIP(InitGpuAvFramework());
    VkPhysicalDeviceVulkan11Features features11 = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(features11);
    if (!features11.samplerYcbcrConversion) {
        GTEST_SKIP() << "samplerYcbcrConversion not supported, skipping test";
    }
    RETURN_IF_SKIP(InitState(nullptr, &features11));
    InitRenderTarget();
    const VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR;

    auto ci = vku::InitStruct<VkImageCreateInfo>();
    ci.flags = 0;
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = format;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ci.extent = {256, 256, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    if (!ImageFormatIsSupported(instance(), gpu(), ci, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
        // Assume there's low ROI on searching for different mp formats
        GTEST_SKIP() << "Multiplane image format not supported";
    }

    vkt::Image image(*m_device, ci, vkt::set_layout);
    vkt::SamplerYcbcrConversion conversion(*m_device, format);
    auto conversion_info = conversion.ConversionInfo();
    vkt::ImageView view = image.CreateView(VK_IMAGE_ASPECT_COLOR_BIT, &conversion_info);

    VkSamplerCreateInfo sampler_ci = SafeSaneSamplerCreateInfo();
    sampler_ci.pNext = &conversion_info;
    vkt::Sampler sampler(*m_device, sampler_ci);
    VkSampler immutable_samplers[2] = {sampler.handle(), sampler.handle()};

    OneOffDescriptorSet descriptor_set(
        m_device, {
                      {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, immutable_samplers},
                  });
    const vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    VkDescriptorImageInfo image_infos[2] = {};
    image_infos[0] = {sampler.handle(), view.handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    image_infos[1] = {sampler.handle(), view.handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkWriteDescriptorSet descriptor_writes = vku::InitStruct<VkWriteDescriptorSet>();
    descriptor_writes.dstSet = descriptor_set.set_;
    descriptor_writes.dstBinding = 0;
    descriptor_writes.descriptorCount = 2;
    descriptor_writes.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes.pImageInfo = image_infos;
    vk::UpdateDescriptorSets(device(), 1, &descriptor_writes, 0, nullptr);

    const char fsSource[] = R"glsl(
        #version 450
        layout (set = 0, binding = 0) uniform sampler2D ycbcr[2];
        layout(location=0) out vec4 out_color;
        void main() {
            int index = 0;
            if (gl_FragCoord.x > 0.5) {
                index = 1;
            }
            out_color = texelFetch(ycbcr[index], ivec2(0), 0);
        }
    )glsl";
    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.gp_ci_.layout = pipeline_layout.handle();
    pipe.CreateGraphicsPipeline();

    m_command_buffer.begin();
    m_command_buffer.BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.Handle());
    vk::CmdBindDescriptorSets(m_command_buffer.handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout.handle(), 0, 1,
                              &descriptor_set.set_, 0, nullptr);
    m_errorMonitor->SetDesiredError("VUID-vkCmdDraw-None-06550");
    vk::CmdDraw(m_command_buffer.handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();
    m_command_buffer.EndRenderPass();
    m_command_buffer.end();
}

TEST_F(NegativeGpuAV, ForceUniformAndStorageBuffer8BitAccess) {
    TEST_DESCRIPTION("Make sure that GPU-AV enabled uniformAndStorageBuffer8BitAccess on behalf of app");

    RETURN_IF_SKIP(InitGpuAvFramework());

    if (!DeviceExtensionSupported(VK_KHR_8BIT_STORAGE_EXTENSION_NAME)) {
        GTEST_SKIP() << VK_KHR_8BIT_STORAGE_EXTENSION_NAME << " not supported, skipping test";
    }

    VkPhysicalDevice8BitStorageFeaturesKHR eight_bit_storage_features = vku::InitStructHelper();
    VkPhysicalDeviceFeatures2 features_2 = vku::InitStructHelper(&eight_bit_storage_features);
    vk::GetPhysicalDeviceFeatures2(gpu(), &features_2);
    if (!eight_bit_storage_features.uniformAndStorageBuffer8BitAccess) {
        GTEST_SKIP() << "Required feature uniformAndStorageBuffer8BitAccess is not supported, skipping test";
    }

    m_errorMonitor->SetDesiredWarning(
        "Adding a VkPhysicalDevice8BitStorageFeatures to pNext with uniformAndStorageBuffer8BitAccess set to VK_TRUE");

    // noise
    m_errorMonitor->SetAllowedFailureMsg("Adding a VkPhysicalDevice8BitStorageFeatures to pNext with shaderInt64 set to VK_TRUE");
    m_errorMonitor->SetAllowedFailureMsg(
        "Adding a VkPhysicalDeviceTimelineSemaphoreFeatures to pNext with timelineSemaphore set to VK_TRUE");
    m_errorMonitor->SetAllowedFailureMsg(
        "Adding a VkPhysicalDeviceBufferDeviceAddressFeatures to pNext with bufferDeviceAddress set to VK_TRUE");
    m_errorMonitor->SetAllowedFailureMsg(
        "Buffer device address validation option was enabled, but required buffer device address extension and/or features are not "
        "enabled");
    m_errorMonitor->SetAllowedFailureMsg("Ray Query validation option was enabled, but the rayQuery feature is not enabled");
    m_errorMonitor->SetAllowedFailureMsg(
        "vkGetDeviceProcAddr(): pName is trying to grab vkGetPhysicalDeviceCalibrateableTimeDomainsKHR which is an instance level "
        "function");
    RETURN_IF_SKIP(InitState());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, CopyBufferToImageD32) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT.");

    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);

    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, sizeof(float) * 64 * 64,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    float *ptr = static_cast<float *>(copy_src_buffer.memory().map());
    for (size_t i = 0; i < 64 * 64; ++i) {
        ptr[i] = 0.1f;
    }
    ptr[4094] = 42.0f;
    copy_src_buffer.memory().unmap();

    vkt::Image copy_dst_image(*m_device, 64, 64, 1, VK_FORMAT_D32_SFLOAT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.begin();

    VkBufferImageCopy buffer_image_copy_1;
    buffer_image_copy_1.bufferOffset = 0;
    buffer_image_copy_1.bufferRowLength = 0;
    buffer_image_copy_1.bufferImageHeight = 0;
    buffer_image_copy_1.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    buffer_image_copy_1.imageOffset = {0, 0, 0};
    buffer_image_copy_1.imageExtent = {64, 64, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy_1);

    VkBufferImageCopy buffer_image_copy_2 = buffer_image_copy_1;
    buffer_image_copy_2.imageOffset = {32, 32, 0};
    buffer_image_copy_2.imageExtent = {32, 32, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy_2);

    m_command_buffer.end();
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, CopyBufferToImageD32Vk13) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, sizeof(float) * 64 * 64,
                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    float *ptr = static_cast<float *>(copy_src_buffer.memory().map());
    for (size_t i = 0; i < 64 * 64; ++i) {
        ptr[i] = 0.1f;
    }
    ptr[4094] = 42.0f;
    copy_src_buffer.memory().unmap();

    vkt::Image copy_dst_image(*m_device, 64, 64, 1, VK_FORMAT_D32_SFLOAT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.begin();

    VkBufferImageCopy2 region_1 = vku::InitStructHelper();
    region_1.bufferOffset = 0;
    region_1.bufferRowLength = 0;
    region_1.bufferImageHeight = 0;
    region_1.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    region_1.imageOffset = {0, 0, 0};
    region_1.imageExtent = {64, 64, 1};

    VkCopyBufferToImageInfo2 buffer_image_copy = vku::InitStructHelper();
    buffer_image_copy.srcBuffer = copy_src_buffer;
    buffer_image_copy.dstImage = copy_dst_image;
    buffer_image_copy.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    buffer_image_copy.regionCount = 1;
    buffer_image_copy.pRegions = &region_1;

    vk::CmdCopyBufferToImage2(m_command_buffer, &buffer_image_copy);

    region_1.imageOffset = {32, 32, 0};
    region_1.imageExtent = {32, 32, 1};

    vk::CmdCopyBufferToImage2(m_command_buffer, &buffer_image_copy);

    m_command_buffer.end();
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_errorMonitor->SetDesiredError("has a float value at offset 16376 that is not in the range [0, 1]");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, CopyBufferToImageD32U8) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT_S8_UINT.");
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, 5 * 64 * 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto ptr = static_cast<uint8_t *>(copy_src_buffer.memory().map());
    std::memset(ptr, 0, static_cast<size_t>(copy_src_buffer.CreateInfo().size));
    for (size_t i = 0; i < 64 * 64; ++i) {
        auto ptr_float = reinterpret_cast<float *>(ptr + 5 * i);
        if (i == 64 * 64 - 1) {
            *ptr_float = 42.0f;
        } else {
            *ptr_float = 0.1f;
        }
    }

    copy_src_buffer.memory().unmap();

    vkt::Image copy_dst_image(*m_device, 64, 64, 1, VK_FORMAT_D32_SFLOAT_S8_UINT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.begin();

    VkBufferImageCopy buffer_image_copy;
    buffer_image_copy.bufferOffset = 0;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    buffer_image_copy.imageOffset = {33, 33, 0};
    buffer_image_copy.imageExtent = {31, 31, 1};

    vk::CmdCopyBufferToImage(m_command_buffer, copy_src_buffer, copy_dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                             &buffer_image_copy);

    m_command_buffer.end();
    m_errorMonitor->SetDesiredError("has a float value at offset 20475 that is not in the range [0, 1]");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeGpuAV, CopyBufferToImageD32U8Vk13) {
    TEST_DESCRIPTION(
        "Copy depth buffer to image with some of its depth value being outside of the [0, 1] legal range. Depth image has format "
        "VK_FORMAT_D32_SFLOAT_S8_UINT.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::uniformAndStorageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework());
    RETURN_IF_SKIP(InitState());

    vkt::Buffer copy_src_buffer(*m_device, 5 * 64 * 64, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto ptr = static_cast<uint8_t *>(copy_src_buffer.memory().map());
    std::memset(ptr, 0, static_cast<size_t>(copy_src_buffer.CreateInfo().size));
    for (size_t i = 0; i < 64 * 64; ++i) {
        auto ptr_float = reinterpret_cast<float *>(ptr + 5 * i);
        if (i == 64 * 64 - 1) {
            *ptr_float = 42.0f;
        } else {
            *ptr_float = 0.1f;
        }
    }

    copy_src_buffer.memory().unmap();

    vkt::Image copy_dst_image(*m_device, 64, 64, 1, VK_FORMAT_D32_SFLOAT_S8_UINT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    copy_dst_image.SetLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m_command_buffer.begin();

    VkBufferImageCopy2 region_1 = vku::InitStructHelper();
    region_1.bufferOffset = 0;
    region_1.bufferRowLength = 0;
    region_1.bufferImageHeight = 0;
    region_1.imageSubresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0, 1};
    region_1.imageOffset = {33, 33, 0};
    region_1.imageExtent = {31, 31, 1};

    VkCopyBufferToImageInfo2 buffer_image_copy = vku::InitStructHelper();
    buffer_image_copy.srcBuffer = copy_src_buffer;
    buffer_image_copy.dstImage = copy_dst_image;
    buffer_image_copy.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    buffer_image_copy.regionCount = 1;
    buffer_image_copy.pRegions = &region_1;

    vk::CmdCopyBufferToImage2(m_command_buffer, &buffer_image_copy);

    m_command_buffer.end();
    m_errorMonitor->SetDesiredError("has a float value at offset 20475 that is not in the range [0, 1]");
    m_default_queue->Submit(m_command_buffer);
    m_default_queue->Wait();
    m_errorMonitor->VerifyFound();
}
