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

#include "generated/feature_requirements_helper.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace vkt {

class FeatureRequirements {
  public:
    ~FeatureRequirements();
    // Add a feature required for the test to be executed
    void AddRequiredFeature(APIVersion api_version, vkt::Feature feature);
    // Add a feature that will be disabled when creating the device
    void AddDisabledFeature(APIVersion api_version, vkt::Feature feature);

    bool HasFeatures2() const { return feature_chain_ != nullptr; }
    VkPhysicalDeviceFeatures* GetFeatures() { return &phys_dev_features_.features; };
    VkPhysicalDeviceFeatures2* GetFeatures2();
    // Return nullptr if all required feature are enabled, else return the name of the first disabled feature
    const char* AnyRequiredFeatureDisabled() const;
    void EnforceDisableFeatures();

  private:
    FeatureAndName SetFeature(APIVersion api_version, vkt::Feature feature, VkBool32 value);

    void* feature_chain_ = nullptr;
    VkPhysicalDeviceFeatures2 phys_dev_features_{};
    std::vector<FeatureAndName> required_features_{};
    std::vector<VkBool32*> disabled_features_{};
};

}  // namespace vkt