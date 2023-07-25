/*
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include "layer_validation_tests.h"

namespace nv {
namespace rt {

// Helper class for to create ray tracing pipeline tests
// Designed with minimal error checking to ensure easy error state creation
// See OneshotTest for typical usage
struct RayTracingPipelineHelper {
  public:
    std::vector<VkDescriptorSetLayoutBinding> dsl_bindings_;
    std::unique_ptr<OneOffDescriptorSet> descriptor_set_;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VkPipelineLayoutCreateInfo pipeline_layout_ci_ = {};
    vk_testing::PipelineLayout pipeline_layout_;
    VkRayTracingPipelineCreateInfoNV rp_ci_ = {};
    VkRayTracingPipelineCreateInfoKHR rp_ci_KHR_ = {};
    VkPipelineCacheCreateInfo pc_ci_ = {};
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::vector<VkRayTracingShaderGroupCreateInfoNV> groups_;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups_KHR_;
    std::unique_ptr<VkShaderObj> rgs_;
    std::unique_ptr<VkShaderObj> chs_;
    std::unique_ptr<VkShaderObj> mis_;
    VkLayerTest& layer_test_;
    RayTracingPipelineHelper(VkLayerTest& test);
    ~RayTracingPipelineHelper();

    void InitShaderGroups();
    void InitShaderGroupsKHR();
    void InitDescriptorSetInfo();
    void InitDescriptorSetInfoKHR();
    void InitPipelineLayoutInfo();
    void InitShaderInfo();
    void InitShaderInfoKHR();
    void InitNVRayTracingPipelineInfo();
    void InitKHRRayTracingPipelineInfo();
    void InitPipelineCacheInfo();
    void InitInfo(bool isKHR = false);
    void InitState();
    void InitPipelineCache();
    void LateBindPipelineInfo(bool isKHR = false);
    VkResult CreateNVRayTracingPipeline(bool implicit_destroy = true, bool do_late_bind = true);
    VkResult CreateKHRRayTracingPipeline(bool implicit_destroy = true, bool do_late_bind = true);
    // Helper function to create a simple test case (positive or negative)
    //
    // info_override can be any callable that takes a CreateNVRayTracingPipelineHelper &
    // flags, error can be any args accepted by "SetDesiredFailure".
    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test& test, const OverrideFunc& info_override, const std::vector<Error>& errors,
                            const VkFlags flags = kErrorBit) {
        RayTracingPipelineHelper helper(test);
        helper.InitInfo();
        info_override(helper);
        helper.InitState();

        for (const auto& error : errors) test.Monitor().SetDesiredFailureMsg(flags, error);
        helper.CreateNVRayTracingPipeline();
        test.Monitor().VerifyFound();
    }

    template <typename Test, typename OverrideFunc, typename Error>
    static void OneshotTest(Test& test, const OverrideFunc& info_override, Error error, const VkFlags flags = kErrorBit) {
        OneshotTest(test, info_override, std::vector<Error>(1, error), flags);
    }

    template <typename Test, typename OverrideFunc>
    static void OneshotPositiveTest(Test& test, const OverrideFunc& info_override, const VkFlags message_flag_mask = kErrorBit) {
        RayTracingPipelineHelper helper(test);
        helper.InitInfo();
        info_override(helper);
        helper.InitState();

        ASSERT_VK_SUCCESS(helper.CreateNVRayTracingPipeline());
    }
};

// DEPRECATED: This is part of the legacy ray tracing framework, now only used in the old nvidia ray tracing extension tests.
void GetSimpleGeometryForAccelerationStructureTests(const vk_testing::Device& device, vk_testing::Buffer* vbo,
                                                    vk_testing::Buffer* ibo, VkGeometryNV* geometry, VkDeviceSize offset = 0,
                                                    bool buffer_device_address = false);
}  // namespace rt
}  // namespace nv
