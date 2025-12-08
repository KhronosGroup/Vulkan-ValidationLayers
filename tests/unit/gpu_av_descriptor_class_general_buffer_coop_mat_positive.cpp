/*
 * Copyright (c) 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2020-2025 Valve Corporation
 * Copyright (c) 2020-2025 LunarG, Inc.
 * Copyright (c) 2020-2025 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include <vulkan/vulkan_core.h>
#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"
#include "../framework/descriptor_helper.h"
#include "cooperative_matrix_helper.h"

class PositiveGpuAVDescriptorClassGeneralBufferCoopMat : public GpuAVDescriptorClassGeneralBufferCoopMat {};

void GpuAVDescriptorClassGeneralBufferCoopMat::InitCooperativeMatrixKHR(bool safe_mode) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    // glslang will generate OpCapability VulkanMemoryModel and need entension enabled
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    RETURN_IF_SKIP(InitGpuAvFramework({}, safe_mode));
    RETURN_IF_SKIP(InitState());
}

void GpuAVDescriptorClassGeneralBufferCoopMat::BasicComputeTest(const char *shader, int source_type, VkDeviceSize buffer_size,
                                                                const char *expected_error, uint32_t error_count) {
    const bool safe_mode = expected_error != nullptr;
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(safe_mode));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2, (SpvSourceType)source_type);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    if (expected_error) m_errorMonitor->SetDesiredError(expected_error, error_count);
    m_default_queue->SubmitAndWait(m_command_buffer);
    if (expected_error) m_errorMonitor->VerifyFound();
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBufferCoopMat, Basic) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(false));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in; // work as a multiple of most subgroup sizes
         layout(set=0, binding=0) coherent buffer InputA { uint8_t x[]; } inputA;
         layout(set=0, binding=1) coherent buffer InputB { uint8_t x[]; } inputB;
         layout(set=0, binding=2) coherent buffer InputC { uint32_t x[]; } inoutC;
         coopmat<int8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseB> matB;
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
         void main() {
            coopMatLoad(matA, inputA.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatLoad(matB, inputB.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatLoad(matC, inoutC.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            matC = coopMatMulAdd(matA, matB, matC);
            coopMatStore(matC, inoutC.x, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                                  {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    // Exact size
    vkt::Buffer buffer_a(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer buffer_b(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    vkt::Buffer buffer_c(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);

    descriptor_set.WriteDescriptorBufferInfo(0, buffer_a, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(1, buffer_b, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.WriteDescriptorBufferInfo(2, buffer_c, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBufferCoopMat, BDA) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::bufferDeviceAddress);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(false));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in; // work as a multiple of most subgroup sizes

         struct Payload {
             uint8_t payload[512];
         };
         layout(set=0, binding=0) coherent buffer SSBO {
            Payload ptr;
            uint8_t results[];
         };
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
            coopMatLoad(matA, ptr.payload, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, results, 0, 16, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    OneOffDescriptorSet descriptor_set(m_device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}});
    vkt::PipelineLayout pipeline_layout(*m_device, {&descriptor_set.layout_});

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
    pipe.cp_ci_.layout = pipeline_layout;
    pipe.CreateComputePipeline();
    m_errorMonitor->VerifyFound();

    vkt::Buffer ssbo(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    descriptor_set.WriteDescriptorBufferInfo(0, ssbo, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    descriptor_set.UpdateDescriptorSets();

    vkt::Buffer payload(*m_device, 512, 0, vkt::device_address);
    auto *ssbo_ptr = static_cast<VkDeviceAddress *>(ssbo.Memory().Map());
    ssbo_ptr[0] = payload.Address();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_set.set_, 0,
                              nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBufferCoopMat, UnsafeOptimization) {
    TEST_DESCRIPTION("Ensure our optimization for unsafe mode are working");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_KHR_shader_subgroup_basic : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) buffer SSBO0 { uint8_t payload[4096]; };
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;

         void a() {
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
            coopMatStore(matC, payload, 6, 16, gl_CooperativeMatrixLayoutColumnMajor); // highest
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
         }
         void b() {
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
            coopMatStore(matC, payload, 0, 32, gl_CooperativeMatrixLayoutColumnMajor); // highest
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
         }
         void c() {
            coopMatLoad(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor); // highest
            payload[10] = uint8_t(0);
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor); // highest
         }
         void main() {
            coopMatLoad(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
            payload[3000] = uint8_t(0); // highest
            coopMatStore(matC, payload, 0, 16, gl_CooperativeMatrixLayoutColumnMajor);
            a();
            b();
            c();
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 4096);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBufferCoopMat, Robustness) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    AddRequiredFeature(vkt::Feature::cooperativeMatrixRobustBufferAccess);

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO { uint8_t payload[]; };
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
            coopMatLoad(matA, payload, 64, 64, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, payload, 128, 32, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    BasicComputeTest(cs_source, SPV_SOURCE_GLSL, 256);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBufferCoopMat, RobustnessForced) {
    std::vector<VkLayerSettingEXT> layer_settings = {
        {OBJECT_LAYER_NAME, "gpuav_force_on_robustness", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &kVkTrue},
    };
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::cooperativeMatrix);
    AddRequiredFeature(vkt::Feature::vulkanMemoryModel);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitGpuAvFramework({layer_settings}, false));
    RETURN_IF_SKIP(InitState());

    VkPhysicalDeviceCooperativeMatrixFeaturesKHR robustness2_features = vku::InitStructHelper();
    GetPhysicalDeviceFeatures2(robustness2_features);
    if (!robustness2_features.cooperativeMatrixRobustBufferAccess) {
        GTEST_SKIP() << "cooperativeMatrixRobustBufferAccess is not supported";
    }

    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) coherent buffer SSBO { uint8_t payload[]; };
         coopmat<uint8_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matA;
         void main() {
            coopMatLoad(matA, payload, 64, 64, gl_CooperativeMatrixLayoutRowMajor);
            coopMatStore(matA, payload, 128, 32, gl_CooperativeMatrixLayoutRowMajor);
         }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}

TEST_F(PositiveGpuAVDescriptorClassGeneralBufferCoopMat, DynamicStrideAndElement) {
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredFeature(vkt::Feature::shaderInt8);
    AddRequiredFeature(vkt::Feature::storageBuffer8BitAccess);
    RETURN_IF_SKIP(InitCooperativeMatrixKHR(false));
    CooperativeMatrixHelper helper(*this);
    if (!helper.Has16x16UintProperty()) {
        GTEST_SKIP() << "16x16 Uint Property not found";
    }

    const char *cs_source = R"glsl(
         #version 450 core
         #pragma use_vulkan_memory_model
         #extension GL_KHR_memory_scope_semantics : enable
         #extension GL_KHR_cooperative_matrix : enable
         #extension GL_EXT_shader_explicit_arithmetic_types : enable
         #extension GL_EXT_shader_explicit_arithmetic_types_int8 : enable
         layout(local_size_x = 64) in;
         layout(set=0, binding=0) buffer SSBO {
            uint32_t stride;
            int32_t element;
            uint8_t payload[];
         };
         coopmat<uint32_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> matC;
         void main() {
            coopMatLoad(matC, payload, element, stride + 4, gl_CooperativeMatrixLayoutColumnMajor);
            coopMatStore(matC, payload, element + 4, stride, gl_CooperativeMatrixLayoutColumnMajor);
         }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.dsl_bindings_[0] = {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr};
    pipe.cs_ = VkShaderObj(*m_device, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_2);
    pipe.CreateComputePipeline();

    vkt::Buffer in_buffer(*m_device, 4096, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, kHostVisibleMemProps);
    pipe.descriptor_set_.WriteDescriptorBufferInfo(0, in_buffer, 0, VK_WHOLE_SIZE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_.UpdateDescriptorSets();

    uint32_t *buffer_ptr = (uint32_t *)in_buffer.Memory().Map();
    buffer_ptr[0] = 16;  // stride
    buffer_ptr[1] = 4;   // element

    m_command_buffer.Begin();
    vk::CmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe);
    vk::CmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_, 0, 1,
                              &pipe.descriptor_set_.set_, 0, nullptr);
    vk::CmdDispatch(m_command_buffer, 1, 1, 1);
    m_command_buffer.End();

    m_default_queue->SubmitAndWait(m_command_buffer);
}