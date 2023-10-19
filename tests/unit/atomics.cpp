/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
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

TEST_F(NegativeAtomic, VertexStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if vertexPipelineStoresAndAtomics disable.");

    VkPhysicalDeviceFeatures features{};
    features.vertexPipelineStoresAndAtomics = VK_FALSE;
    RETURN_IF_SKIP(Init(&features))
    InitRenderTarget();

    // Test StoreOp
    {
        char const *vsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageStore(si0, ivec2(0), vec4(0));
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-RuntimeSpirv-NonWritable-06341");
    }

    // Test AtomicOp
    {
        char const *vsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, r32f) uniform image2D si0;
            void main() {
                  imageAtomicExchange(si0, ivec2(0), 1);
            }
        )glsl";

        VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
        if (VK_SUCCESS == vs.InitFromGLSLTry()) {
            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {vs.GetStageCreateInfo(), info.fs_->GetStageCreateInfo()};
                info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}};
            };

            constexpr std::array vuids = {"VUID-RuntimeSpirv-None-06286", "VUID-RuntimeSpirv-NonWritable-06341"};
            // extra VU for not enabling atomic float support
            CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT, vuids);
        }
    }
}

TEST_F(NegativeAtomic, FragmentStoresAndAtomicsFeatureDisable) {
    TEST_DESCRIPTION("Run shader with StoreOp or AtomicOp to verify if fragmentStoresAndAtomics disable.");

    VkPhysicalDeviceFeatures features{};
    features.fragmentStoresAndAtomics = VK_FALSE;
    RETURN_IF_SKIP(Init(&features))
    InitRenderTarget();

    // Test StoreOp
    {
        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, rgba8) uniform image2D si0;
            void main() {
                  imageStore(si0, ivec2(0), vec4(0));
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

        auto info_override = [&](CreatePipelineHelper &info) {
            info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
            info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
        };

        CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                          "VUID-RuntimeSpirv-NonWritable-06340");
    }

    // Test AtomicOp
    {
        char const *fsSource = R"glsl(
            #version 450
            layout(set=0, binding=0, r32f) uniform image2D si0;
            void main() {
                  imageAtomicExchange(si0, ivec2(0), 1);
            }
        )glsl";

        VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_GLSL_TRY);
        if (VK_SUCCESS == fs.InitFromGLSLTry()) {
            auto info_override = [&](CreatePipelineHelper &info) {
                info.shader_stages_ = {info.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
                info.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
            };

            // extra VU for not enabling atomic float support
            constexpr std::array vuids = {"VUID-RuntimeSpirv-None-06286", "VUID-RuntimeSpirv-NonWritable-06340"};
            CreatePipelineHelper::OneshotTest(*this, info_override, VK_DEBUG_REPORT_ERROR_BIT_EXT, vuids);
        }
    }
}

TEST_F(NegativeAtomic, Int64) {
    TEST_DESCRIPTION("Test VK_KHR_shader_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    // Create device without VK_KHR_shader_atomic_int64 extension or features enabled
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures available_features = {};
    GetPhysicalDeviceFeatures(&available_features);
    if (!available_features.shaderInt64) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::shaderInt64 is not supported";
    }
    RETURN_IF_SKIP(InitState())

    // For sanity check without GL_EXT_shader_atomic_int64
    std::string cs_positive = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
           y = x + 1;
        }
    )glsl";

    std::string cs_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_atomic_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared uint64_t x;
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        void main() {
    )glsl";

    // clang-format off
    // StorageBuffer storage class
    std::string cs_storage_buffer = cs_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    // StorageBuffer storage class using AtomicStore
    // atomicStore is slightly different than other atomics, so good edge case
    std::string cs_store = cs_base + R"glsl(
           atomicStore(y, 1ul, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    // Workgroup storage class
    std::string cs_workgroup = cs_base + R"glsl(
           atomicAdd(x, 1);
           barrier();
           y = x + 1;
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    current_shader = cs_positive.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // shaderBufferInt64Atomics
    current_shader = cs_storage_buffer.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-RuntimeSpirv-None-06278"});
    current_shader = cs_store.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-RuntimeSpirv-None-06278"});

    // shaderSharedInt64Atomics
    current_shader = cs_workgroup.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-RuntimeSpirv-None-06279"});
}

TEST_F(NegativeAtomic, ImageInt64) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    // Create device without VK_EXT_shader_image_atomic_int64 extension or features enabled
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceFeatures available_features = {};
    GetPhysicalDeviceFeatures(&available_features);
    if (!available_features.shaderInt64) {
        GTEST_SKIP() << "VkPhysicalDeviceFeatures::shaderInt64 is not supported, skipping tests.";
    }

    RETURN_IF_SKIP(InitState())

    // clang-format off
    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        layout(set = 0, binding = 1, r64ui) uniform u64image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    std::unique_ptr<VkShaderObj> current_shader;
    const auto set_info = [&current_shader](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        helper.cs_ = std::move(current_shader);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                                {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    // shaderImageInt64Atomics
    // Need 01091 VUID check for both Int64ImageEXT and Int64Atomics.. test could be rewritten to be more complex in order to set
    // capability requirements with other features, but this is simpler
    current_shader = std::make_unique<VkShaderObj>(this, cs_image_load.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                   SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08740",
                                "VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06288"});
    }

    // glslang doesn't omit Int64Atomics for store currently
    current_shader = std::make_unique<VkShaderObj>(this, cs_image_store.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                   SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08740",
                                "VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06288"});
    }

    current_shader = std::make_unique<VkShaderObj>(this, cs_image_exchange.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                   SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08740",
                                "VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06288"});
    }

    current_shader = std::make_unique<VkShaderObj>(this, cs_image_add.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                   SPV_SOURCE_GLSL_TRY);
    m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08737");
    if (VK_SUCCESS == current_shader->InitFromGLSLTry()) {
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08740",
                                "VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06288"});
    }
}

TEST_F(NegativeAtomic, ImageInt64Drawtime64) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 draw time with 64 bit image view.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT image_atomic_int64_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(image_atomic_int64_features);
    if (features2.features.shaderInt64 == VK_FALSE) {
        GTEST_SKIP() << "shaderInt64 feature not supported";
    } else if (image_atomic_int64_features.shaderImageInt64Atomics == VK_FALSE) {
        GTEST_SKIP() << "shaderImageInt64Atomics feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    std::string cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, r64ui) uniform u64image2D z;
        void main() {
            uint64_t y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView view = image.targetView(VK_FORMAT_R32_UINT);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-SampledType-04471");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeAtomic, ImageInt64Drawtime32) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 draw time with 32 bit image view.");
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT image_atomic_int64_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(image_atomic_int64_features);
    if (features2.features.shaderInt64 == VK_FALSE) {
        GTEST_SKIP() << "shaderInt64 feature not supported";
    } else if (image_atomic_int64_features.shaderImageInt64Atomics == VK_FALSE) {
        GTEST_SKIP() << "shaderImageInt64Atomics feature not supported";
    }

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    std::string cs_source = R"glsl(
        #version 450
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0, r32ui) uniform uimage2D z;
        void main() {
            uint y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    // "64-bit integer atomic support is guaranteed for optimally tiled images with the VK_FORMAT_R64_UINT"
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R64_UINT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView view = image.targetView(VK_FORMAT_R64_UINT);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-SampledType-04470");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeAtomic, ImageInt64DrawtimeSparse) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 at draw time with Sparse image.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT image_atomic_int64_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(image_atomic_int64_features);
    if (!features2.features.shaderInt64) {
        GTEST_SKIP() << "shaderInt64 feature not supported";
    } else if (!features2.features.sparseBinding) {
        GTEST_SKIP() << "sparseBinding feature not supported";
    } else if (!image_atomic_int64_features.shaderImageInt64Atomics) {
        GTEST_SKIP() << "shaderImageInt64Atomics feature not supported";
    }
    image_atomic_int64_features.sparseImageInt64Atomics = VK_FALSE;  // turn off
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    const char *cs_source = R"glsl(
        #version 450
        #extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
        #extension GL_EXT_shader_image_int64 : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { uint64_t y; };
        layout(set = 0, binding = 1, r64ui) uniform u64image2D z;
        void main() {
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    CreateComputePipelineHelper pipe(*this);
    // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
    pipe.cs_ = std::make_unique<VkShaderObj>(this, cs_source, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr},
                          {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    pipe.InitState();
    pipe.CreateComputePipeline();

    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_ci.size = 1024;
    vkt::Buffer buffer(*m_device, buffer_ci);
    pipe.descriptor_set_->WriteDescriptorBufferInfo(0, buffer.handle(), 0, 1024, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    pipe.descriptor_set_->UpdateDescriptorSets();

    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = VK_FORMAT_R64_UINT;
    image_ci.extent = {32, 32, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_STORAGE_BIT;
    VkImageObj image(m_device);
    image.init_no_mem(*m_device, image_ci);
    VkImageView image_view = image.targetView(VK_FORMAT_R64_UINT);
    pipe.descriptor_set_->WriteDescriptorImageInfo(1, image_view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_COMPUTE, pipe.pipeline_);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDispatch-sparseImageInt64Atomics-04474");
    vk::CmdDispatch(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->end();
}

TEST_F(NegativeAtomic, ImageInt64Mesh32) {
    TEST_DESCRIPTION("Test VK_EXT_shader_image_atomic_int64 draw time with 32 bit image view in Mesh shaders.");
    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework());

    VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features = vku::InitStructHelper();
    VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT image_atomic_int64_features = vku::InitStructHelper(&mesh_shader_features);
    auto features2 = GetPhysicalDeviceFeatures2(image_atomic_int64_features);
    if (features2.features.shaderInt64 == VK_FALSE) {
        GTEST_SKIP() << "shaderInt64 feature not supported";
    } else if (image_atomic_int64_features.shaderImageInt64Atomics == VK_FALSE) {
        GTEST_SKIP() << "shaderImageInt64Atomics feature not supported";
    } else if (mesh_shader_features.meshShader == VK_FALSE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }
    mesh_shader_features.multiviewMeshShader = VK_FALSE;
    mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &features2))
    InitRenderTarget();

    const char *mesh_source = R"glsl(
        #version 460
        #extension GL_EXT_mesh_shader : enable
        #extension GL_KHR_memory_scope_semantics : enable

        layout(max_vertices = 3, max_primitives=1) out;
        layout(triangles) out;
        layout(set = 0, binding = 0, r32ui) uniform uimage2D z;

        void main() {
            SetMeshOutputsEXT(3,1);
            uint y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    VkShaderObj ms(this, mesh_source, VK_SHADER_STAGE_MESH_BIT_EXT, SPV_ENV_VULKAN_1_2);
    VkShaderObj fs(this, kFragmentMinimalGlsl, VK_SHADER_STAGE_FRAGMENT_BIT, SPV_ENV_VULKAN_1_2);

    CreatePipelineHelper pipe(*this);
    pipe.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr}};
    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    pipe.gp_ci_.pVertexInputState = nullptr;
    pipe.gp_ci_.pInputAssemblyState = nullptr;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    // "64-bit integer atomic support is guaranteed for optimally tiled images with the VK_FORMAT_R64_UINT"
    VkImageObj image(m_device);
    image.Init(32, 32, 1, VK_FORMAT_R64_UINT, VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_TILING_OPTIMAL, 0);
    VkImageView view = image.targetView(VK_FORMAT_R64_UINT);

    pipe.descriptor_set_->WriteDescriptorImageInfo(0, view, VK_NULL_HANDLE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                                   VK_IMAGE_LAYOUT_GENERAL);
    pipe.descriptor_set_->UpdateDescriptorSets();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_layout_.handle(), 0, 1,
                              &pipe.descriptor_set_->set_, 0, nullptr);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDrawMeshTasksEXT-SampledType-04470");
    vk::CmdDrawMeshTasksEXT(m_commandBuffer->handle(), 1, 1, 1);
    m_errorMonitor->VerifyFound();
    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeAtomic, Float) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float.");
    SetTargetApiVersion(VK_API_VERSION_1_1);

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    // Create device without VK_EXT_shader_atomic_float extension or features enabled
    RETURN_IF_SKIP(InitFramework())
    VkPhysicalDeviceFeatures available_features = {};
    GetPhysicalDeviceFeatures(&available_features);
    RETURN_IF_SKIP(InitState())

    // clang-format off
    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_add = cs_32_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_store = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_32_add = cs_32_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_load = cs_32_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_store = cs_32_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_32_exchange = cs_32_base + R"glsl(
           float32_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_add = cs_64_base + R"glsl(
           atomicAdd(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_store = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_shared_float_64_add = cs_64_base + R"glsl(
           y = atomicAdd(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_load = cs_64_base + R"glsl(
           y = 1 + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_store = cs_64_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_64_exchange = cs_64_base + R"glsl(
           float64_t a = 1;
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_image_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float : enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_load = cs_image_base + R"glsl(
           y = imageAtomicLoad(z, ivec2(1, 1), gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_store = cs_image_base + R"glsl(
           imageAtomicStore(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_exchange = cs_image_base + R"glsl(
           imageAtomicExchange(z, ivec2(1, 1), y, gl_ScopeDevice, gl_StorageSemanticsImage, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_image_add = cs_image_base + R"glsl(
           y = imageAtomicAdd(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [&](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1,
                                                   SPV_SOURCE_GLSL_TRY);
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        if (VK_SUCCESS != helper.cs_.get()->InitFromGLSLTry()) {
            helper.override_skip_ = true;
        }
        helper.dsl_bindings_ = current_bindings;
    };

    // shaderBufferFloat32Atomics
    current_shader = cs_buffer_float_32_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

    current_shader = cs_buffer_float_32_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

    current_shader = cs_buffer_float_32_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

    // shaderBufferFloat32AtomicAdd
    current_shader = cs_buffer_float_32_add.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742",
                            "VUID-RuntimeSpirv-None-06284"});

    // shaderSharedFloat32Atomics
    current_shader = cs_shared_float_32_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

    current_shader = cs_shared_float_32_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

    current_shader = cs_shared_float_32_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

    // shaderSharedFloat32AtomicAdd
    current_shader = cs_shared_float_32_add.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742",
                            "VUID-RuntimeSpirv-None-06285"});

    // shaderBufferFloat64Atomics
    if (available_features.shaderFloat64) {
        current_shader = cs_buffer_float_64_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

        current_shader = cs_buffer_float_64_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

        current_shader = cs_buffer_float_64_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

        // shaderBufferFloat64AtomicAdd
        current_shader = cs_buffer_float_64_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742",
                                "VUID-RuntimeSpirv-None-06284"});

        // shaderSharedFloat64Atomics
        current_shader = cs_shared_float_64_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

        current_shader = cs_shared_float_64_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

        current_shader = cs_shared_float_64_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

        // shaderSharedFloat64AtomicAdd
        current_shader = cs_shared_float_64_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742",
                                "VUID-RuntimeSpirv-None-06285"});
    } else {
        printf("Skipping 64-bit float tests\n");
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    // shaderImageFloat32Atomics
    current_shader = cs_image_load.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06286");

    current_shader = cs_image_store.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06286");

    current_shader = cs_image_exchange.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06286");

    // shaderImageFloat32AtomicAdd
    current_shader = cs_image_add.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08740", "VUID-VkShaderModuleCreateInfo-pCode-08742",
                            "VUID-RuntimeSpirv-None-06286"});
}

TEST_F(NegativeAtomic, Float2) {
    TEST_DESCRIPTION("Test VK_EXT_shader_atomic_float2.");
    SetTargetApiVersion(VK_API_VERSION_1_2);

    // Create device without VK_EXT_shader_atomic_float2 extension or features enabled
    RETURN_IF_SKIP(InitFramework())

    // Still check for proper 16-bit storage/float support for most tests
    VkPhysicalDeviceShaderFloat16Int8Features float16int8_features = vku::InitStructHelper();
    VkPhysicalDevice16BitStorageFeatures storage_16_bit_features = vku::InitStructHelper(&float16int8_features);
    auto features2 = GetPhysicalDeviceFeatures2(storage_16_bit_features);

    const bool support_16_bit =
        (float16int8_features.shaderFloat16 == VK_TRUE) && (storage_16_bit_features.storageBuffer16BitAccess == VK_TRUE);

    RETURN_IF_SKIP(InitState(nullptr, &features2))

    // clang-format off
    std::string cs_16_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_16bit_storage: enable
        #extension GL_KHR_memory_scope_semantics : enable
        shared float16_t x;
        layout(set = 0, binding = 0) buffer ssbo { float16_t y; };
        void main() {
    )glsl";

     std::string cs_buffer_float_16_add = cs_16_base + R"glsl(
           atomicAdd(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_store = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicStore(y, a, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_buffer_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(y, a);
        }
    )glsl";

    std::string cs_buffer_float_16_min = cs_16_base + R"glsl(
           atomicMin(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_16_max = cs_16_base + R"glsl(
           atomicMax(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_add = cs_16_base + R"glsl(
           y = atomicAdd(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_load = cs_16_base + R"glsl(
           y = float16_t(1.0) + atomicLoad(x, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_store = cs_16_base + R"glsl(
           atomicStore(x, y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
        }
    )glsl";

    std::string cs_shared_float_16_exchange = cs_16_base + R"glsl(
           float16_t a = float16_t(1.0);
           atomicExchange(x, y);
        }
    )glsl";

    std::string cs_shared_float_16_min = cs_16_base + R"glsl(
           y = atomicMin(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_shared_float_16_max = cs_16_base + R"glsl(
           y = atomicMax(x, float16_t(1.0));
        }
    )glsl";

    std::string cs_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_32_min = cs_32_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_32_max = cs_32_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_32_min = cs_32_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_32_max = cs_32_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    std::string cs_64_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        shared float64_t x;
        layout(set = 0, binding = 0) buffer ssbo { float64_t y; };
        void main() {
    )glsl";

    std::string cs_buffer_float_64_min = cs_64_base + R"glsl(
           atomicMin(y, 1);
        }
    )glsl";

    std::string cs_buffer_float_64_max = cs_64_base + R"glsl(
           atomicMax(y, 1);
        }
    )glsl";

    std::string cs_shared_float_64_min = cs_64_base + R"glsl(
           y = atomicMin(x, 1);
        }
    )glsl";

    std::string cs_shared_float_64_max = cs_64_base + R"glsl(
           y = atomicMax(x, 1);
        }
    )glsl";

    std::string cs_image_32_base = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        layout(set = 0, binding = 0) buffer ssbo { float y; };
        layout(set = 0, binding = 1, r32f) uniform image2D z;
        void main() {
    )glsl";

    std::string cs_image_32_min = cs_image_32_base + R"glsl(
           y = imageAtomicMin(z, ivec2(1, 1), y);
        }
    )glsl";

    std::string cs_image_32_max = cs_image_32_base + R"glsl(
           y = imageAtomicMax(z, ivec2(1, 1), y);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    // set binding for buffer tests
    std::vector<VkDescriptorSetLayoutBinding> current_bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};

    const auto set_info = [this, &current_shader, &current_bindings](CreateComputePipelineHelper &helper) {
        // Requires SPIR-V 1.3 for SPV_KHR_storage_buffer_storage_class
        m_errorMonitor->SetUnexpectedError("VUID-VkShaderModuleCreateInfo-pCode-08740");
        helper.cs_ = VkShaderObj::CreateFromGLSL(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1);
        // Skip the test if shader failed to compile
        helper.override_skip_ = !static_cast<bool>(helper.cs_);
        helper.dsl_bindings_ = current_bindings;
    };

    if (support_16_bit) {
        // shaderBufferFloat16Atomics
        current_shader = cs_buffer_float_16_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

        current_shader = cs_buffer_float_16_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

        current_shader = cs_buffer_float_16_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06284");

        // shaderBufferFloat16AtomicAdd
        current_shader = cs_buffer_float_16_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-VkShaderModuleCreateInfo-pCode-08742",
                                "VUID-RuntimeSpirv-None-06284"});

        // shaderBufferFloat16AtomicMinMax
        current_shader = cs_buffer_float_16_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06284"});

        current_shader = cs_buffer_float_16_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06284"});

        // shaderSharedFloat16Atomics
        current_shader = cs_shared_float_16_load.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

        current_shader = cs_shared_float_16_store.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

        current_shader = cs_shared_float_16_exchange.c_str();
        CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-RuntimeSpirv-None-06285");

        // shaderSharedFloat16AtomicAdd
        current_shader = cs_shared_float_16_add.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-VkShaderModuleCreateInfo-pCode-08742",
                                "VUID-RuntimeSpirv-None-06285"});

        // shaderSharedFloat16AtomicMinMax
        current_shader = cs_shared_float_16_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06285"});

        current_shader = cs_shared_float_16_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06285"});
    } else {
        printf("Skipping 16-bit tests\n");
    }

    // shaderBufferFloat32AtomicMinMax
    current_shader = cs_buffer_float_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06284"});

    current_shader = cs_buffer_float_32_max.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06284"});

    // shaderSharedFloat32AtomicMinMax
    current_shader = cs_shared_float_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06285"});

    current_shader = cs_shared_float_32_max.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06285"});

    if (features2.features.shaderFloat64 == VK_TRUE) {
        // shaderBufferFloat64AtomicMinMax
        current_shader = cs_buffer_float_64_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06284"});

        current_shader = cs_buffer_float_64_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06284"});

        // shaderSharedFloat64AtomicMinMax
        current_shader = cs_shared_float_64_min.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06285"});

        current_shader = cs_shared_float_64_max.c_str();
        CreateComputePipelineHelper::OneshotTest(
            *this, set_info, kErrorBit,
            std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06285"});
    } else {
        printf("Skipping 64-bit float tests\n");
    }

    // Add binding for images
    current_bindings.push_back({1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_ALL, nullptr});

    // shaderSharedFloat32AtomicMinMax
    current_shader = cs_image_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06286"});

    current_shader = cs_image_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(
        *this, set_info, kErrorBit,
        std::vector<string>{"VUID-VkShaderModuleCreateInfo-pCode-08742", "VUID-RuntimeSpirv-None-06286"});
}

TEST_F(NegativeAtomic, Float2WidthMismatch) {
    TEST_DESCRIPTION("VK_EXT_shader_atomic_float2 but enable wrong bitwidth.");
    SetTargetApiVersion(VK_API_VERSION_1_3);
    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceVulkan13Features features13 = vku::InitStructHelper();  // need maintenance4
    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT atomic_float2_features = vku::InitStructHelper(&features13);
    VkPhysicalDeviceShaderFloat16Int8Features float16int8_features = vku::InitStructHelper(&atomic_float2_features);
    VkPhysicalDevice16BitStorageFeatures storage_16_bit_features = vku::InitStructHelper(&float16int8_features);
    GetPhysicalDeviceFeatures2(storage_16_bit_features);
    if (!float16int8_features.shaderFloat16 || !storage_16_bit_features.storageBuffer16BitAccess ||
        !atomic_float2_features.shaderBufferFloat16AtomicMinMax) {
        GTEST_SKIP() << "Required float 16 atomic features not supported";
    }
    // turn off 32-bit support
    atomic_float2_features.shaderBufferFloat32AtomicMinMax = VK_FALSE;
    RETURN_IF_SKIP(InitState(nullptr, &storage_16_bit_features));

    // clang-format off
    std::string cs_buffer_float_16_min = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable
        #extension GL_EXT_shader_16bit_storage: enable
        #extension GL_KHR_memory_scope_semantics : enable
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        shared float16_t x;
        layout(set = 0, binding = 0) buffer ssbo { float16_t y; };
        void main() {
           atomicMin(y, float16_t(1.0));
        }
    )glsl";

    std::string cs_buffer_float_32_min = R"glsl(
        #version 450
        #extension GL_EXT_shader_atomic_float2 : enable
        #extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        shared float32_t x;
        layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
        void main() {
           atomicMin(y, 1);
        }
    )glsl";
    // clang-format on

    const char *current_shader = nullptr;
    const auto set_info = [this, &current_shader](CreateComputePipelineHelper &helper) {
        helper.cs_ = std::make_unique<VkShaderObj>(this, current_shader, VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_3);
        helper.dsl_bindings_ = {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_ALL, nullptr}};
    };

    // shaderBufferFloat16AtomicMinMax - valid - everything enabled
    current_shader = cs_buffer_float_16_min.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit);

    // shaderBufferFloat32AtomicMinMax - not enabled
    current_shader = cs_buffer_float_32_min.c_str();
    CreateComputePipelineHelper::OneshotTest(*this, set_info, kErrorBit, std::vector<string>{"VUID-RuntimeSpirv-None-06338"});
}

TEST_F(NegativeAtomic, InvalidStorageOperation) {
    TEST_DESCRIPTION(
        "If storage view use atomic operation, the view's format MUST support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT or "
        "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT ");

    AddRequiredExtensions(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

    RETURN_IF_SKIP(InitFramework())

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features = vku::InitStructHelper();
    auto features2 = GetPhysicalDeviceFeatures2(atomic_float_features);
    RETURN_IF_SKIP(InitState(nullptr, &features2))

    if (atomic_float_features.shaderImageFloat32Atomics == VK_FALSE) {
        GTEST_SKIP() << "shaderImageFloat32Atomics not supported.";
    }

    VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT;
    VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;  // The format doesn't support VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT to
                                                       // cause DesiredFailure. VK_FORMAT_R32_UINT is right format.
    auto image_ci = VkImageObj::ImageCreateInfo2D(64, 64, 1, 1, image_format, usage, VK_IMAGE_TILING_OPTIMAL);

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
    if (!device_features.vertexPipelineStoresAndAtomics || !device_features.fragmentStoresAndAtomics) {
        GTEST_SKIP() << "vertexPipelineStoresAndAtomics & fragmentStoresAndAtomics NOT supported";
    }

    VkImageObj image(m_device);
    image.Init(image_ci);
    VkImageView image_view = image.targetView(image_format);

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
        void main() {
              imageAtomicExchange(si0, ivec2(0), 1);
              imageAtomicExchange(si1[0], ivec2(0), 1);
              imageAtomicExchange(si1[1], ivec2(0), 1);
              imageAtomicExchange(stb2, 0, 1);
              imageAtomicExchange(stb3[0], 0, 1);
              imageAtomicExchange(stb3[1], 0, 1);
        }
    )glsl";

    VkShaderObj fs(this, fsSource, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper g_pipe(*this);
    g_pipe.shader_stages_ = {g_pipe.vs_->GetStageCreateInfo(), fs.GetStageCreateInfo()};
    g_pipe.dsl_bindings_ = {{3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {1, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
                            {0, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
    g_pipe.InitState();
    ASSERT_EQ(VK_SUCCESS, g_pipe.CreateGraphicsPipeline());

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

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_);
    vk::CmdBindDescriptorSets(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipe.pipeline_layout_.handle(), 0, 1,
                              &g_pipe.descriptor_set_->set_, 0, nullptr);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02691");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02691");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-07888");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-07888");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}
