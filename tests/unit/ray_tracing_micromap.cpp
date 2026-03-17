/*
 * Copyright (c) 2015-2026 The Khronos Group Inc.
 * Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2025 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/descriptor_helper.h"

class NegativeRayTracingMicromap : public RayTracingTest {};

TEST_F(NegativeRayTracingMicromap, FeatureDisableEXT) {
    TEST_DESCRIPTION("Micromap feature is disabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkCreateMicromapEXT-micromap-07430");

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);

    VkMicromapEXT micromap;
    VkMicromapCreateInfoEXT maCreateInfo = vku::InitStructHelper();

    maCreateInfo.createFlags = 0;
    maCreateInfo.buffer = buffer;
    maCreateInfo.offset = 0;
    maCreateInfo.size = 0;
    maCreateInfo.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    maCreateInfo.deviceAddress = 0ull;

    vk::CreateMicromapEXT(device(), &maCreateInfo, nullptr, &micromap);

    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeRayTracingMicromap, CaptureReplayFeatureDisableEXT) {
    TEST_DESCRIPTION("Micromap capture replay feature is disabled");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::accelerationStructure);
    AddRequiredFeature(vkt::Feature::micromap);
    RETURN_IF_SKIP(Init());

    m_errorMonitor->SetDesiredError("VUID-vkCreateMicromapEXT-deviceAddress-07431");

    vkt::Buffer buffer(*m_device, 32, VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT);

    VkMicromapEXT micromap;
    VkMicromapCreateInfoEXT maCreateInfo = vku::InitStructHelper();

    maCreateInfo.createFlags = 0;
    maCreateInfo.buffer = buffer;
    maCreateInfo.offset = 0;
    maCreateInfo.size = 0;
    maCreateInfo.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    maCreateInfo.deviceAddress = 0x100000ull;

    vk::CreateMicromapEXT(device(), &maCreateInfo, nullptr, &micromap);

    m_errorMonitor->VerifyFound();
}
