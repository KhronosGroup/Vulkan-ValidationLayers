/*
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 * Copyright (C) 2025 Arm Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "cooperative_matrix_helper.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/utility/vk_struct_helper.hpp>
#include "containers/container_utils.h"
#include "layer_validation_tests.h"

CooperativeMatrixHelper::CooperativeMatrixHelper(VkLayerTest &layer_test) : layer_test(layer_test) {
    const VkPhysicalDevice gpu = layer_test.Gpu();
    uint32_t props_count = 0;
    vk::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(gpu, &props_count, nullptr);
    for (uint32_t i = 0; i < props_count; i++) {
        coop_matrix_props.emplace_back(vku::InitStruct<VkCooperativeMatrixPropertiesKHR>());
    }
    vk::GetPhysicalDeviceCooperativeMatrixPropertiesKHR(gpu, &props_count, coop_matrix_props.data());

    if (layer_test.IsExtensionsEnabled(VK_NV_COOPERATIVE_MATRIX_2_EXTENSION_NAME)) {
        props_count = 0;
        vk::GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(gpu, &props_count, nullptr);
        for (uint32_t i = 0; i < props_count; i++) {
            coop_matrix_flex_props.emplace_back(vku::InitStruct<VkCooperativeMatrixFlexibleDimensionsPropertiesNV>());
        }
        vk::GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(gpu, &props_count, coop_matrix_flex_props.data());
    }
}

bool CooperativeMatrixHelper::SupportsStage(VkShaderStageFlags required_stage) {
    if (required_stage == 0) {
        return true;
    }
    VkPhysicalDeviceCooperativeMatrixPropertiesKHR props = vku::InitStructHelper();
    layer_test.GetPhysicalDeviceProperties2(props);
    return ((props.cooperativeMatrixSupportedStages & required_stage) != 0);
}

bool CooperativeMatrixHelper::Has8BitComponentType(const VkCooperativeMatrixPropertiesKHR &prop) const {
    const VkComponentTypeKHR type_8bit[6] = {
        VK_COMPONENT_TYPE_SINT8_KHR,       VK_COMPONENT_TYPE_UINT8_KHR,       VK_COMPONENT_TYPE_SINT8_PACKED_NV,
        VK_COMPONENT_TYPE_UINT8_PACKED_NV, VK_COMPONENT_TYPE_FLOAT8_E4M3_EXT, VK_COMPONENT_TYPE_FLOAT8_E5M2_EXT,
    };
    return IsValueIn(prop.AType, type_8bit) || IsValueIn(prop.BType, type_8bit) || IsValueIn(prop.CType, type_8bit) ||
           IsValueIn(prop.ResultType, type_8bit);
}

bool CooperativeMatrixHelper::Has64BitComponentType(const VkCooperativeMatrixPropertiesKHR &prop) const {
    const VkComponentTypeKHR type_64bit[3] = {VK_COMPONENT_TYPE_FLOAT64_KHR, VK_COMPONENT_TYPE_SINT64_KHR,
                                              VK_COMPONENT_TYPE_UINT64_KHR};
    return IsValueIn(prop.AType, type_64bit) || IsValueIn(prop.BType, type_64bit) || IsValueIn(prop.CType, type_64bit) ||
           IsValueIn(prop.ResultType, type_64bit);
}

bool CooperativeMatrixHelper::Has16x16UintProperty() const {
    for (const auto &prop : coop_matrix_props) {
        if (prop.scope == VK_SCOPE_SUBGROUP_KHR && prop.KSize == 16 && prop.MSize == 16 && prop.NSize == 16 &&
            prop.AType == VK_COMPONENT_TYPE_UINT8_KHR && prop.BType == VK_COMPONENT_TYPE_UINT8_KHR &&
            prop.CType == VK_COMPONENT_TYPE_UINT32_KHR && prop.ResultType == VK_COMPONENT_TYPE_UINT32_KHR) {
            return true;
        }
    }
    return false;
}

bool CooperativeMatrixHelper::HasValidProperty(VkScopeKHR scope, uint32_t m, uint32_t n, uint32_t k,
                                               VkComponentTypeKHR type) const {
    bool found_a = false;
    bool found_b = false;
    bool found_c = false;
    bool found_r = false;
    for (const auto &prop : coop_matrix_props) {
        if (prop.scope == scope && prop.AType == type && prop.MSize == m && prop.KSize == k) {
            found_a = true;
        }
        if (prop.scope == scope && prop.BType == type && prop.KSize == k && prop.NSize == n) {
            found_b = true;
        }
        if (prop.scope == scope && prop.CType == type && prop.MSize == m && prop.NSize == n) {
            found_c = true;
        }
        if (prop.scope == scope && prop.ResultType == type && prop.MSize == m && prop.NSize == n) {
            found_r = true;
        }
    }
    if (found_a && found_b && found_c && found_r) {
        return true;
    }

    found_a = false;
    found_b = false;
    found_c = false;
    found_r = false;
    for (const auto &prop : coop_matrix_flex_props) {
        if (prop.scope == scope && prop.AType == type && (m % prop.MGranularity) == 0 && (k % prop.KGranularity) == 0) {
            found_a = true;
        }
        if (prop.scope == scope && prop.BType == type && (k % prop.KGranularity) == 0 && (n % prop.NGranularity) == 0) {
            found_b = true;
        }
        if (prop.scope == scope && prop.CType == type && (m % prop.MGranularity) == 0 && (n % prop.NGranularity) == 0) {
            found_c = true;
        }
        if (prop.scope == scope && prop.ResultType == type && (m % prop.MGranularity) == 0 && (n % prop.NGranularity) == 0) {
            found_r = true;
        }
    }
    if (found_a && found_b && found_c && found_r) {
        return true;
    }

    return false;
}

const char *CooperativeMatrixHelper::VkComponentTypeToGLSL(VkComponentTypeKHR type) const {
    switch (type) {
        case VK_COMPONENT_TYPE_FLOAT16_KHR:
            return "float16_t";
        case VK_COMPONENT_TYPE_FLOAT32_KHR:
            return "float32_t";
        case VK_COMPONENT_TYPE_FLOAT64_KHR:
            return "float64_t";
        case VK_COMPONENT_TYPE_SINT8_KHR:
            return "int8_t";
        case VK_COMPONENT_TYPE_SINT16_KHR:
            return "int16_t";
        case VK_COMPONENT_TYPE_SINT32_KHR:
            return "int32_t";
        case VK_COMPONENT_TYPE_SINT64_KHR:
            return "int64_t";
        case VK_COMPONENT_TYPE_UINT8_KHR:
            return "uint8_t";
        case VK_COMPONENT_TYPE_UINT16_KHR:
            return "uint16_t";
        case VK_COMPONENT_TYPE_UINT32_KHR:
            return "uint32_t";
        case VK_COMPONENT_TYPE_UINT64_KHR:
            return "uint64_t";
        default:
            return "unknown";
    }
}