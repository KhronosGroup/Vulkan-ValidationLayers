/*
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/pipeline_helper.h"

class PositiveDeviceQueue : public VkLayerTest {};

TEST_F(PositiveDeviceQueue, NoQueues) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::maintenance9);
    RETURN_IF_SKIP(InitFramework());

    std::vector<const char *> extensions = m_device_extension_names;
    extensions.push_back(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);

    VkPhysicalDeviceMaintenance9FeaturesKHR maintenance9_features = vku::InitStructHelper();
    maintenance9_features.maintenance9 = VK_TRUE;

    VkDeviceCreateInfo device_ci = vku::InitStructHelper();
    device_ci.pNext = &maintenance9_features;
    device_ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    device_ci.ppEnabledExtensionNames = extensions.data();

    vkt::Device device(gpu_, device_ci);
}
