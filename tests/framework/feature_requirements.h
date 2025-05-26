/*
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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
    FeatureRequirements();
    ~FeatureRequirements();
    // Add a feature required for the test to be executed
    void AddRequiredFeature(APIVersion api_version, vkt::Feature feature);
    // Add an optional feature that will be enabled when available for the test to be executed
    void AddOptionalFeature(APIVersion api_version, vkt::Feature feature);

    bool HasFeatures2() const { return queried_phys_dev_features_.pNext != nullptr; }
    VkPhysicalDeviceFeatures* GetQueriedFeatures() { return &queried_phys_dev_features_.features; }
    VkPhysicalDeviceFeatures2* GetQueriedFeatures2() { return &queried_phys_dev_features_; }
    VkPhysicalDeviceFeatures* GetEnabledFeatures() {
        UpdateEnabledFeatures();
        return &enabled_phys_dev_features_.features;
    }
    VkPhysicalDeviceFeatures2* GetEnabledFeatures2() {
        UpdateEnabledFeatures();
        return &enabled_phys_dev_features_;
    }
    // Return nullptr if all required feature are enabled, else return the name of the first disabled feature
    const char* AnyRequiredFeatureDisabled() const;

  private:
    FeatureAndName AddFeature(VkPhysicalDeviceFeatures2& phys_dev_features, APIVersion api_version, vkt::Feature feature);
    void UpdateEnabledFeatures();

    struct FeatureInfo {
        FeatureAndName queried_feature;
        FeatureAndName enabled_feature;
    };

    VkPhysicalDeviceFeatures2 queried_phys_dev_features_{};
    VkPhysicalDeviceFeatures2 enabled_phys_dev_features_{};

    std::vector<FeatureInfo> required_features_{};
    std::vector<FeatureInfo> optional_features_{};
};

}  // namespace vkt