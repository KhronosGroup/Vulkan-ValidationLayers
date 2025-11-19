/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

class VkLayerTest;

// Single spot to handle all the things around properties
class CooperativeMatrixHelper {
  public:
    explicit CooperativeMatrixHelper(VkLayerTest &layer_test);

    VkLayerTest &layer_test;
    std::vector<VkCooperativeMatrixPropertiesKHR> coop_matrix_props;
    std::vector<VkCooperativeMatrixFlexibleDimensionsPropertiesNV> coop_matrix_flex_props;

    bool SupportsStage(VkShaderStageFlags required_stage);
    bool Has8BitComponentType(const VkCooperativeMatrixPropertiesKHR &prop) const;
    bool Has64BitComponentType(const VkCooperativeMatrixPropertiesKHR &prop) const;
    bool Has16x16UintProperty() const;
    bool HasValidProperty(VkScopeKHR scope, uint32_t m, uint32_t n, uint32_t k, VkComponentTypeKHR type) const;
    const char *VkComponentTypeToGLSL(VkComponentTypeKHR type) const;
};
