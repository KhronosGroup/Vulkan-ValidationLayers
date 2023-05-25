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

class NegativeVertexInput : public VkLayerTest {};

TEST_F(NegativeVertexInput, AttributeFormat) {
    TEST_DESCRIPTION("Test that pipeline validation catches invalid vertex attribute formats");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attribs;
    memset(&input_attribs, 0, sizeof(input_attribs));

    // Pick a really bad format for this purpose and make sure it should fail
    input_attribs.format = VK_FORMAT_BC2_UNORM_BLOCK;
    VkFormatProperties format_props = m_device->format_properties(input_attribs.format);
    if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) != 0) {
        GTEST_SKIP() << "Format unsuitable for test";
    }

    input_attribs.location = 0;

    auto set_info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkVertexInputAttributeDescription-format-00623");
}

TEST_F(NegativeVertexInput, DivisorExtension) {
    TEST_DESCRIPTION("Test VUIDs added with VK_EXT_vertex_attribute_divisor extension.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    vadf.vertexAttributeInstanceRateDivisor = VK_TRUE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_TRUE;

    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vadf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPhysicalDeviceLimits &dev_limits = m_device->props.limits;
    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props =
        LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&pdvad_props);
    GetPhysicalDeviceProperties2(pd_props2);

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = LvlInitStruct<VkPipelineVertexInputDivisorStateCreateInfoEXT>();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.stride = 12;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    if (pdvad_props.maxVertexAttribDivisor < pvids_ci.vertexBindingDivisorCount) {
        GTEST_SKIP() << "This device does not support vertexBindingDivisors";
    }

    using std::vector;
    struct TestCase {
        uint32_t div_binding;
        uint32_t div_divisor;
        uint32_t desc_binding;
        VkVertexInputRate desc_rate;
        vector<std::string> vuids;
    };

    // clang-format off
    vector<TestCase> test_cases = {
        {   0,
            1,
            0,
            VK_VERTEX_INPUT_RATE_VERTEX,
            {"VUID-VkVertexInputBindingDivisorDescriptionEXT-inputRate-01871"}
        },
        {   dev_limits.maxVertexInputBindings + 1,
            1,
            0,
            VK_VERTEX_INPUT_RATE_INSTANCE,
            {"VUID-VkVertexInputBindingDivisorDescriptionEXT-binding-01869",
             "VUID-VkVertexInputBindingDivisorDescriptionEXT-inputRate-01871"}
        }
    };

    if (vvl::kU32Max != pdvad_props.maxVertexAttribDivisor) {  // Can't test overflow if maxVAD is UINT32_MAX
        test_cases.push_back(
            {   0,
                pdvad_props.maxVertexAttribDivisor + 1,
                0,
                VK_VERTEX_INPUT_RATE_INSTANCE,
                {"VUID-VkVertexInputBindingDivisorDescriptionEXT-divisor-01870"}
            } );
    }
    // clang-format on

    for (const auto &test_case : test_cases) {
        const auto bad_divisor_state = [&test_case, &vibdd, &pvids_ci, &vibd](CreatePipelineHelper &helper) {
            vibdd.binding = test_case.div_binding;
            vibdd.divisor = test_case.div_divisor;
            vibd.binding = test_case.desc_binding;
            vibd.inputRate = test_case.desc_rate;
            helper.vi_ci_.pNext = &pvids_ci;
            helper.vi_ci_.vertexBindingDescriptionCount = 1;
            helper.vi_ci_.pVertexBindingDescriptions = &vibd;
        };
        CreatePipelineHelper::OneshotTest(*this, bad_divisor_state, kErrorBit, test_case.vuids);
    }
}

TEST_F(NegativeVertexInput, DivisorDisabled) {
    TEST_DESCRIPTION("Test instance divisor feature disabled for VK_EXT_vertex_attribute_divisor extension.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    vadf.vertexAttributeInstanceRateDivisor = VK_FALSE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vadf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT pdvad_props =
        LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorPropertiesEXT>();
    VkPhysicalDeviceProperties2 pd_props2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&pdvad_props);
    GetPhysicalDeviceProperties2(pd_props2);

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    vibdd.binding = 0;
    vibdd.divisor = 2;
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = LvlInitStruct<VkPipelineVertexInputDivisorStateCreateInfoEXT>();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.binding = vibdd.binding;
    vibd.stride = 12;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    if (pdvad_props.maxVertexAttribDivisor < pvids_ci.vertexBindingDivisorCount) {
        GTEST_SKIP() << "This device does not support vertexBindingDivisors";
    }

    const auto instance_rate = [&pvids_ci, &vibd](CreatePipelineHelper &helper) {
        helper.vi_ci_.pNext = &pvids_ci;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexBindingDescriptions = &vibd;
    };
    CreatePipelineHelper::OneshotTest(*this, instance_rate, kErrorBit,
                                      "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateDivisor-02229");
}

TEST_F(NegativeVertexInput, DivisorInstanceRateZero) {
    TEST_DESCRIPTION("Test instanceRateZero feature of VK_EXT_vertex_attribute_divisor extension.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT vadf = LvlInitStruct<VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT>();
    vadf.vertexAttributeInstanceRateDivisor = VK_TRUE;
    vadf.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
    VkPhysicalDeviceFeatures2 pd_features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&vadf);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &pd_features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDivisorDescriptionEXT vibdd = {};
    vibdd.binding = 0;
    vibdd.divisor = 0;
    VkPipelineVertexInputDivisorStateCreateInfoEXT pvids_ci = LvlInitStruct<VkPipelineVertexInputDivisorStateCreateInfoEXT>();
    pvids_ci.vertexBindingDivisorCount = 1;
    pvids_ci.pVertexBindingDivisors = &vibdd;
    VkVertexInputBindingDescription vibd = {};
    vibd.binding = vibdd.binding;
    vibd.stride = 12;
    vibd.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    const auto instance_rate = [&pvids_ci, &vibd](CreatePipelineHelper &helper) {
        helper.vi_ci_.pNext = &pvids_ci;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexBindingDescriptions = &vibd;
    };
    CreatePipelineHelper::OneshotTest(
        *this, instance_rate, kErrorBit,
        "VUID-VkVertexInputBindingDivisorDescriptionEXT-vertexAttributeInstanceRateZeroDivisor-02228");
}

TEST_F(NegativeVertexInput, InputBindingMaxVertexInputBindings) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputBindingDescription-binding-00618: binding must be less than "
        "VkPhysicalDeviceLimits::maxVertexInputBindings");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when binding is greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings.
    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = m_device->props.limits.maxVertexInputBindings;

    const auto set_binding = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_binding, kErrorBit, "VUID-VkVertexInputBindingDescription-binding-00618");
}

TEST_F(NegativeVertexInput, InputBindingMaxVertexInputBindingStride) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputBindingDescription-stride-00619: stride must be less than or equal to "
        "VkPhysicalDeviceLimits::maxVertexInputBindingStride");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when stride is greater than VkPhysicalDeviceLimits::maxVertexInputBindingStride.
    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.stride = m_device->props.limits.maxVertexInputBindingStride + 1;

    const auto set_binding = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_binding, kErrorBit, "VUID-VkVertexInputBindingDescription-stride-00619");
}

TEST_F(NegativeVertexInput, InputAttributeMaxVertexInputAttributes) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputAttributeDescription-location-00620: location must be less than "
        "VkPhysicalDeviceLimits::maxVertexInputAttributes");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when location is greater than or equal to VkPhysicalDeviceLimits::maxVertexInputAttributes.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.location = m_device->props.limits.maxVertexInputAttributes;

    const auto set_attribute = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_attribute, kErrorBit,
                                      vector<string>{"VUID-VkVertexInputAttributeDescription-location-00620",
                                                     "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                                                     "VUID-VkVertexInputAttributeDescription-format-00623"});
}

TEST_F(NegativeVertexInput, InputAttributeMaxVertexInputBindings) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputAttributeDescription-binding-00621: binding must be less than "
        "VkPhysicalDeviceLimits::maxVertexInputBindings");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    // Test when binding is greater than or equal to VkPhysicalDeviceLimits::maxVertexInputBindings.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.binding = m_device->props.limits.maxVertexInputBindings;

    const auto set_attribute = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_attribute, kErrorBit,
                                      vector<string>{"VUID-VkVertexInputAttributeDescription-binding-00621",
                                                     "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                                                     "VUID-VkVertexInputAttributeDescription-format-00623"});
}

TEST_F(NegativeVertexInput, AttributeDescriptionOffset) {
    TEST_DESCRIPTION(
        "Test VUID-VkVertexInputAttributeDescription-offset-00622: offset must be less than or equal to "
        "VkPhysicalDeviceLimits::maxVertexInputAttributeOffset");

    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    VkPhysicalDeviceProperties device_props = {};
    vk::GetPhysicalDeviceProperties(gpu(), &device_props);
    const uint32_t maxVertexInputAttributeOffset = device_props.limits.maxVertexInputAttributeOffset;
    if (maxVertexInputAttributeOffset == 0xFFFFFFFF) {
        GTEST_SKIP() << "maxVertexInputAttributeOffset is max<uint32_t> already";
    }
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription vertex_input_binding_description{};
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = m_device->props.limits.maxVertexInputBindingStride;
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // Test when offset is greater than maximum.
    VkVertexInputAttributeDescription vertex_input_attribute_description{};
    vertex_input_attribute_description.format = VK_FORMAT_R8_UNORM;
    vertex_input_attribute_description.offset = maxVertexInputAttributeOffset + 1;
    const auto set_attribute = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &vertex_input_binding_description;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &vertex_input_attribute_description;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_attribute, kErrorBit, "VUID-VkVertexInputAttributeDescription-offset-00622");
}

TEST_F(NegativeVertexInput, BindingDescriptions) {
    TEST_DESCRIPTION(
        "Attempt to create a graphics pipeline where:"
        "1) count of vertex bindings exceeds device's maxVertexInputBindings limit"
        "2) requested bindings include a duplicate binding value");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const uint32_t binding_count = m_device->props.limits.maxVertexInputBindings + 1;

    std::vector<VkVertexInputBindingDescription> input_bindings(binding_count);
    for (uint32_t i = 0; i < binding_count; ++i) {
        input_bindings[i].binding = i;
        input_bindings[i].stride = 4;
        input_bindings[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }
    // Let the last binding description use same binding as the first one
    input_bindings[binding_count - 1].binding = 0;

    VkVertexInputAttributeDescription input_attrib;
    input_attrib.binding = 0;
    input_attrib.location = 0;
    input_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    input_attrib.offset = 0;

    const auto set_Info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = input_bindings.data();
        helper.vi_ci_.vertexBindingDescriptionCount = binding_count;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    constexpr std::array vuids = {"VUID-VkPipelineVertexInputStateCreateInfo-vertexBindingDescriptionCount-00613",
                                  "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616"};
    CreatePipelineHelper::OneshotTest(*this, set_Info, kErrorBit, vuids);
}

TEST_F(NegativeVertexInput, AttributeDescriptions) {
    TEST_DESCRIPTION(
        "Attempt to create a graphics pipeline where:"
        "1) count of vertex attributes exceeds device's maxVertexInputAttributes limit"
        "2) requested location include a duplicate location value"
        "3) binding used by one attribute is not defined by a binding description");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding;
    input_binding.binding = 0;
    input_binding.stride = 4;
    input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    const uint32_t attribute_count = m_device->props.limits.maxVertexInputAttributes + 1;
    std::vector<VkVertexInputAttributeDescription> input_attribs(attribute_count);
    for (uint32_t i = 0; i < attribute_count; ++i) {
        input_attribs[i].binding = 0;
        input_attribs[i].location = i;
        input_attribs[i].format = VK_FORMAT_R32G32B32_SFLOAT;
        input_attribs[i].offset = 0;
    }
    // Let the last input_attribs description use same location as the first one
    input_attribs[attribute_count - 1].location = 0;
    // Let the last input_attribs description use binding which is not defined
    input_attribs[attribute_count - 1].binding = 1;

    const auto set_Info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = input_attribs.data();
        helper.vi_ci_.vertexAttributeDescriptionCount = attribute_count;
    };
    constexpr std::array vuids = {"VUID-VkPipelineVertexInputStateCreateInfo-vertexAttributeDescriptionCount-00614",
                                  "VUID-VkPipelineVertexInputStateCreateInfo-binding-00615",
                                  "VUID-VkPipelineVertexInputStateCreateInfo-pVertexAttributeDescriptions-00617"};
    CreatePipelineHelper::OneshotTest(*this, set_Info, kErrorBit, vuids);
}

TEST_F(NegativeVertexInput, UsingProvokingVertexModeLastVertexExtDisabled) {
    TEST_DESCRIPTION("Test using VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but it doesn't enable provokingVertexLast.");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    auto provoking_vertex_state_ci = LvlInitStruct<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>();
    provoking_vertex_state_ci.provokingVertexMode = VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT;
    pipe.rs_state_ci_.pNext = &provoking_vertex_state_ci;
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT,
                                         "VUID-VkPipelineRasterizationProvokingVertexStateCreateInfoEXT-provokingVertexMode-04883");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVertexInput, ProvokingVertexModePerPipeline) {
    TEST_DESCRIPTION(
        "Test using different VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT but it doesn't support provokingVertexModePerPipeline.");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_EXT_PROVOKING_VERTEX_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework(m_errorMonitor));

    if (DeviceValidationVersion() < VK_API_VERSION_1_1) {
        GTEST_SKIP() << "At least Vulkan version 1.1 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    auto provoking_vertex_properties = LvlInitStruct<VkPhysicalDeviceProvokingVertexPropertiesEXT>();
    auto properties2 = LvlInitStruct<VkPhysicalDeviceProperties2>(&provoking_vertex_properties);
    GetPhysicalDeviceProperties2(properties2);
    if (provoking_vertex_properties.provokingVertexModePerPipeline == VK_TRUE) {
        GTEST_SKIP() << "provokingVertexModePerPipeline is VK_TRUE";
    }

    auto provoking_vertex_features = LvlInitStruct<VkPhysicalDeviceProvokingVertexFeaturesEXT>();
    provoking_vertex_features.provokingVertexLast = VK_TRUE;
    auto features2 = LvlInitStruct<VkPhysicalDeviceFeatures2>(&provoking_vertex_features);

    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    CreatePipelineHelper pipe1(*this);
    pipe1.InitInfo();
    auto provoking_vertex_state_ci = LvlInitStruct<VkPipelineRasterizationProvokingVertexStateCreateInfoEXT>();
    provoking_vertex_state_ci.provokingVertexMode = VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT;
    pipe1.rs_state_ci_.pNext = &provoking_vertex_state_ci;
    pipe1.InitState();
    pipe1.CreateGraphicsPipeline();

    CreatePipelineHelper pipe2(*this);
    pipe2.InitInfo();
    provoking_vertex_state_ci.provokingVertexMode = VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT;
    pipe2.rs_state_ci_.pNext = &provoking_vertex_state_ci;
    pipe2.InitState();
    pipe2.CreateGraphicsPipeline();

    CreatePipelineHelper pipe3(*this);
    pipe3.InitInfo();
    pipe3.InitState();
    pipe3.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.pipeline_);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdBindPipeline-pipelineBindPoint-04881");
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe3.pipeline_);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeVertexInput, VertextBinding) {
    TEST_DESCRIPTION("Verify if VkPipelineVertexInputStateCreateInfo matches vkCmdBindVertexBuffers");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkBufferObj vtx_buf;
    auto info = vtx_buf.create_info(32, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vtx_buf.init(*m_device, info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    // CmdBindVertexBuffers only has binding:1. It causes 04007 & 04008 desired fail.
    VkVertexInputBindingDescription vtx_binding_des[3] = {
        {0, 64, VK_VERTEX_INPUT_RATE_VERTEX}, {1, 64, VK_VERTEX_INPUT_RATE_VERTEX}, {2, 64, VK_VERTEX_INPUT_RATE_VERTEX}};

    // CmdBindVertexBuffers only has binding:1. It causes twice 02721 desired fail.
    // Plus, binding:1's offset is wrong. It causes 02721 desired fail, again.
    VkVertexInputAttributeDescription vtx_attri_des[3] = {{0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 10},
                                                          {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 10},
                                                          {2, 2, VK_FORMAT_R32G32B32A32_SFLOAT, 10}};
    pipe.vi_ci_.vertexBindingDescriptionCount = 3;
    pipe.vi_ci_.pVertexBindingDescriptions = vtx_binding_des;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 3;
    pipe.vi_ci_.pVertexAttributeDescriptions = vtx_attri_des;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);
    VkDeviceSize offset = 0;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 1, 1, &vtx_buf.handle(), &offset);

    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-04008");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-04007");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02721");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02721");
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02721");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeVertexInput, AttributeAlignment) {
    TEST_DESCRIPTION("Check for proper aligment of attribAddress which depends on a bound pipeline and on a bound vertex buffer");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    const VkPipelineLayoutObj pipeline_layout(m_device);

    struct VboEntry {
        uint16_t input0[2];
        uint32_t input1;
        float input2[4];
    };

    const unsigned vbo_entry_count = 3;
    const VboEntry vbo_data[vbo_entry_count] = {};

    VkConstantBufferObj vbo(m_device, static_cast<int>(sizeof(VboEntry) * vbo_entry_count),
                            reinterpret_cast<const void *>(vbo_data), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkVertexInputBindingDescription input_binding;
    input_binding.binding = 0;
    input_binding.stride = sizeof(VboEntry);
    input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription input_attribs[3];

    input_attribs[0].binding = 0;
    // Location switch between attrib[0] and attrib[1] is intentional
    input_attribs[0].location = 1;
    input_attribs[0].format = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    input_attribs[0].offset = offsetof(VboEntry, input1);

    input_attribs[1].binding = 0;
    input_attribs[1].location = 0;
    input_attribs[1].format = VK_FORMAT_R16G16_UNORM;
    input_attribs[1].offset = offsetof(VboEntry, input0);

    input_attribs[2].binding = 0;
    input_attribs[2].location = 2;
    input_attribs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    input_attribs[2].offset = offsetof(VboEntry, input2);

    char const *vsSource = R"glsl(
        #version 450
        layout(location = 0) in vec2 input0;
        layout(location = 1) in vec4 input1;
        layout(location = 2) in vec4 input2;
        void main(){
           gl_Position = input1 + input2;
           gl_Position.xy += input0;
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkPipelineObj pipe1(m_device);
    pipe1.AddDefaultColorAttachment();
    pipe1.AddShader(&vs);
    pipe1.AddShader(&fs);
    pipe1.AddVertexInputBindings(&input_binding, 1);
    pipe1.AddVertexInputAttribs(&input_attribs[0], 3);
    pipe1.SetViewport(m_viewports);
    pipe1.SetScissor(m_scissors);
    pipe1.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    input_binding.stride = 6;

    VkPipelineObj pipe2(m_device);
    pipe2.AddDefaultColorAttachment();
    pipe2.AddShader(&vs);
    pipe2.AddShader(&fs);
    pipe2.AddVertexInputBindings(&input_binding, 1);
    pipe2.AddVertexInputAttribs(&input_attribs[0], 3);
    pipe2.SetViewport(m_viewports);
    pipe2.SetScissor(m_scissors);
    pipe2.CreateVKPipeline(pipeline_layout.handle(), renderPass());

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);

    // Test with invalid buffer offset
    VkDeviceSize offset = 1;
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe1.handle());
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 0
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 1
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 2
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    // Test with invalid buffer stride
    offset = 0;
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe2.handle());
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vbo.handle(), &offset);
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 0
    // Attribute[1] is aligned properly even with a wrong stride
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-vkCmdDraw-None-02721");  // attribute 2
    m_commandBuffer->Draw(1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeVertexInput, BindVertexOffset) {
    TEST_DESCRIPTION("set the pOffset in vkCmdBindVertexBuffers to 3 and use R16");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkBufferObj vtx_buf;
    auto info = vtx_buf.create_info(1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vtx_buf.init(*m_device, info);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkVertexInputBindingDescription input_binding = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, VK_FORMAT_R16_UNORM, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.InitState();
    pipe.CreateGraphicsPipeline();

    m_commandBuffer->begin();
    m_commandBuffer->BeginRenderPass(m_renderPassBeginInfo);
    vk::CmdBindPipeline(m_commandBuffer->handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.pipeline_);

    VkDeviceSize offset = 3;
    vk::CmdBindVertexBuffers(m_commandBuffer->handle(), 0, 1, &vtx_buf.handle(), &offset);
    m_errorMonitor->SetDesiredFailureMsg(VK_DEBUG_REPORT_ERROR_BIT_EXT, "VUID-vkCmdDraw-None-02721");
    vk::CmdDraw(m_commandBuffer->handle(), 1, 0, 0, 0);
    m_errorMonitor->VerifyFound();

    m_commandBuffer->EndRenderPass();
    m_commandBuffer->end();
}

TEST_F(NegativeVertexInput, AttributeNotConsumed) {
    TEST_DESCRIPTION("Test that a warning is produced for a vertex attribute which is not consumed by the vertex shader");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit,
                                      "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
}

TEST_F(NegativeVertexInput, AttributeLocationMismatch) {
    TEST_DESCRIPTION(
        "Test that a warning is produced for a location mismatch on vertex attributes. This flushes out bad behavior in the "
        "interface walker");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };

    CreatePipelineHelper::OneshotTest(*this, set_info, kPerformanceWarningBit,
                                      "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed");
}

TEST_F(NegativeVertexInput, AttributeNotProvided) {
    TEST_DESCRIPTION("Test that an error is produced for a vertex shader input which is not provided by a vertex attribute");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in vec4 x; /* not provided */
        void main(){
           gl_Position = x;
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-Input-07904");
}

TEST_F(NegativeVertexInput, AttributeTypeMismatch) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a mismatch between the fundamental type (float/int/uint) of an attribute and the "
        "vertex shader input that consumes it");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in int x; /* attrib provided float */
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        helper.vi_ci_.pVertexBindingDescriptions = &input_binding;
        helper.vi_ci_.vertexBindingDescriptionCount = 1;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-Input-08733");
}

// Currently are not checking structs inside vertex input correctly
// https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5906
TEST_F(NegativeVertexInput, DISABLED_AttributeStructType) {
    TEST_DESCRIPTION("Input is OpTypeStruct but doesn't match");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkVertexInputBindingDescription input_binding = {0, 24, VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
    input_attrib.location = 4;

    // This is not valid GLSL (but is valid SPIR-V) - would look like:
    //     in VertexIn {
    //         layout(location = 4) vec4 x;
    //     } x_struct;
    char const *vsSource = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Vertex %1 "main" %2
               OpMemberDecorate %_struct_3 0 Location 4
               OpDecorate %_struct_3 Block
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
  %_struct_3 = OpTypeStruct %v4float
%_ptr_Input__struct_3 = OpTypePointer Input %_struct_3
          %2 = OpVariable %_ptr_Input__struct_3 Input
          %1 = OpFunction %void None %5
         %13 = OpLabel
               OpReturn
               OpFunctionEnd
    )";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT, SPV_ENV_VULKAN_1_0, SPV_SOURCE_ASM);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), fs.GetStageCreateInfo()};
    pipe.InitState();
    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "VUID-VkGraphicsPipelineCreateInfo-Input-08733");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVertexInput, AttributeBindingConflict) {
    TEST_DESCRIPTION(
        "Test that an error is produced for a vertex attribute setup where multiple bindings provide the same location");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    /* Two binding descriptions for binding 0 */
    VkVertexInputBindingDescription input_bindings[2] = {{0, 4, VK_VERTEX_INPUT_RATE_VERTEX}, {0, 4, VK_VERTEX_INPUT_RATE_VERTEX}};

    VkVertexInputAttributeDescription input_attrib;
    memset(&input_attrib, 0, sizeof(input_attrib));
    input_attrib.format = VK_FORMAT_R32_SFLOAT;

    char const *vsSource = R"glsl(
        #version 450
        layout(location=0) in float x; /* attrib provided float */
        void main(){
           gl_Position = vec4(x);
        }
    )glsl";

    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    const auto set_info = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {vs.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
        helper.vi_ci_.pVertexBindingDescriptions = input_bindings;
        helper.vi_ci_.vertexBindingDescriptionCount = 2;
        helper.vi_ci_.pVertexAttributeDescriptions = &input_attrib;
        helper.vi_ci_.vertexAttributeDescriptionCount = 1;
    };
    CreatePipelineHelper::OneshotTest(*this, set_info, kErrorBit,
                                      "VUID-VkPipelineVertexInputStateCreateInfo-pVertexBindingDescriptions-00616");
}

TEST_F(NegativeVertexInput, Attribute64bitInputAttribute) {
    TEST_DESCRIPTION("InputAttribute has 64-bit, but shader reads 32-bit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().shaderFloat64) {
        GTEST_SKIP() << "Device does not support 64bit vertex attributes";
    }

    const VkFormat format = VK_FORMAT_R64_SFLOAT;
    VkFormatProperties format_props = m_device->format_properties(format);
    if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    VkBufferObj vtx_buf;
    auto info = vtx_buf.create_info(1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vtx_buf.init(*m_device, info);

    char const *vsSource = R"glsl(
        #version 450 core
        layout(location = 0) in float pos; // 32-bit
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkVertexInputBindingDescription input_binding = {0, 8, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-VkGraphicsPipelineCreateInfo-Attribute-64bit");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVertexInput, Attribute64bitShaderInput) {
    TEST_DESCRIPTION("InputAttribute has 32-bit, but shader reads 64-bit");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().shaderFloat64) {
        GTEST_SKIP() << "Device does not support 64bit vertex attributes";
    }

    const VkFormat format = VK_FORMAT_R32_SFLOAT;
    VkFormatProperties format_props = m_device->format_properties(format);
    if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    VkBufferObj vtx_buf;
    auto info = vtx_buf.create_info(1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vtx_buf.init(*m_device, info);

    char const *vsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location = 0) in float64_t pos;
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkVertexInputBindingDescription input_binding = {0, 4, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-VkGraphicsPipelineCreateInfo-Input-64bit");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVertexInput, Attribute64bitUnusedComponent) {
    TEST_DESCRIPTION("Shader uses f64vec2, but only provides first component with R64");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().shaderFloat64) {
        GTEST_SKIP() << "Device does not support 64bit vertex attributes";
    }

    const VkFormat format = VK_FORMAT_R64_SFLOAT;
    VkFormatProperties format_props = m_device->format_properties(format);
    if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    VkBufferObj vtx_buf;
    auto info = vtx_buf.create_info(1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vtx_buf.init(*m_device, info);

    char const *vsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location = 0) in f64vec2 pos;
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkVertexInputBindingDescription input_binding = {0, 8, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-VkGraphicsPipelineCreateInfo-Component-64bit");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVertexInput, Attribute64bitMissingComponent) {
    TEST_DESCRIPTION("Shader uses f64vec2, but provides too many component with R64G64B64A64");

    ASSERT_NO_FATAL_FAILURE(Init());
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    if (!m_device->phy().features().shaderFloat64) {
        GTEST_SKIP() << "Device does not support 64bit vertex attributes";
    }

    const VkFormat format = VK_FORMAT_R64G64B64A64_SFLOAT;
    VkFormatProperties format_props = m_device->format_properties(format);
    if ((format_props.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT) == 0) {
        GTEST_SKIP() << "Format not supported for Vertex Buffer";
    }

    VkBufferObj vtx_buf;
    auto info = vtx_buf.create_info(1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vtx_buf.init(*m_device, info);

    char const *vsSource = R"glsl(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types_float64 : enable
        layout(location = 0) in f64vec2 pos;
        void main() {}
    )glsl";
    VkShaderObj vs(this, vsSource, VK_SHADER_STAGE_VERTEX_BIT);

    CreatePipelineHelper pipe(*this);
    pipe.InitInfo();
    VkVertexInputBindingDescription input_binding = {0, 32, VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription input_attribs = {0, 0, format, 0};

    pipe.vi_ci_.vertexBindingDescriptionCount = 1;
    pipe.vi_ci_.pVertexBindingDescriptions = &input_binding;
    pipe.vi_ci_.vertexAttributeDescriptionCount = 1;
    pipe.vi_ci_.pVertexAttributeDescriptions = &input_attribs;
    pipe.shader_stages_ = {vs.GetStageCreateInfo(), pipe.fs_->GetStageCreateInfo()};
    pipe.InitState();

    m_errorMonitor->SetDesiredFailureMsg(kErrorBit, "UNASSIGNED-VkGraphicsPipelineCreateInfo-Component-64bit");
    pipe.CreateGraphicsPipeline();
    m_errorMonitor->VerifyFound();
}
