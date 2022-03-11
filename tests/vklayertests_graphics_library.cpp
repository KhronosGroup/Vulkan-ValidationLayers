/*
 * Copyright (c) 2015-2022 The Khronos Group Inc.
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
 * Copyright (c) 2015-2022 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Author: Nathaniel Cesario <nathaniel@lunarg.com>
 */

#include "layer_validation_tests.h"
#include "vk_extension_helper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include "cast_utils.h"

class VkGraphicsLibraryLayerTest : public VkLayerTest {};

TEST_F(VkGraphicsLibraryLayerTest, InvalidDSLs) {
    TEST_DESCRIPTION("Create a pipeline layout with invalid descriptor set layouts");
    m_errorMonitor->ExpectSuccess();

    ASSERT_NO_FATAL_FAILURE(Init());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dsl_binding.pImmutableSamplers = nullptr;

    auto dsl_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    dsl_ci.bindingCount = 1;
    dsl_ci.pBindings = &dsl_binding;

    vk_testing::DescriptorSetLayout dsl(*m_device, dsl_ci);
    m_errorMonitor->VerifyNotFound();

    std::vector<const vk_testing::DescriptorSetLayout*> dsls = {&dsl, nullptr};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-pSetLayouts-06561");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, dsls);
    m_errorMonitor->VerifyFound();
}

TEST_F(VkGraphicsLibraryLayerTest, GPLInvalidDSLs) {
    TEST_DESCRIPTION("Create a pipeline layout with invalid descriptor set layouts with VK_EXT_grahpics_pipeline_library enabled");
    m_errorMonitor->ExpectSuccess();

    if (!AddRequiredExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME)) {
        printf("%s %s not supported\n", kSkipPrefix, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
        return;
    }

    SetTargetApiVersion(VK_API_VERSION_1_2);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        printf("%s Vulkan >= 1.2 required", kSkipPrefix);
        return;
    }

    std::vector<const char *> failed_exts;
    if (!AreRequestedExtensionsEnabled(failed_exts)) {
        printf("%s The following device extensions are not supported: ", kSkipPrefix);
        for (const auto &ext : failed_exts) {
            printf("%s ", ext);
        }
        printf("\n");
        return;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());

    VkDescriptorSetLayoutBinding dsl_binding = {};
    dsl_binding.binding = 0;
    dsl_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsl_binding.descriptorCount = 1;
    dsl_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dsl_binding.pImmutableSamplers = nullptr;

    auto dsl_ci = LvlInitStruct<VkDescriptorSetLayoutCreateInfo>();
    dsl_ci.bindingCount = 1;
    dsl_ci.pBindings = &dsl_binding;

    vk_testing::DescriptorSetLayout dsl(*m_device, dsl_ci);
    m_errorMonitor->VerifyNotFound();

    std::vector<const vk_testing::DescriptorSetLayout *> dsls = {&dsl, nullptr};

    VkPipelineLayoutCreateInfo pipeline_layout_ci = LvlInitStruct<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkPipelineLayoutCreateInfo-flags-06562");
    vk_testing::PipelineLayout pipeline_layout(*m_device, pipeline_layout_ci, dsls);
    m_errorMonitor->VerifyFound();
}