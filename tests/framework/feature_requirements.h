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

#include "test_common.h"

#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <utility>

template <typename VkFeatureA, typename VkFeatureB>
void pnext_connect(VkFeatureA &feature_a, VkFeatureB &feature_b) {
    feature_a.feature_struct.pNext = &feature_b.feature_struct;
}

// Adapted from https://stackoverflow.com/questions/4832949/c-iterating-over-a-tuple-resolution-of-type-vs-constant-parameters

template <std::size_t I = 0, typename... Tp>
inline typename std::enable_if<(I + 1) >= sizeof...(Tp), void>::type pnext_connect_vk_tuple(std::tuple<Tp...> &) {}

template <std::size_t I = 0, typename... Tp>
inline typename std::enable_if<(I + 1) < sizeof...(Tp), void>::type pnext_connect_vk_tuple(std::tuple<Tp...> &t) {
    pnext_connect(*std::get<I>(t), *std::get<I + 1>(t));
    pnext_connect_vk_tuple<I + 1, Tp...>(t);
}

template <std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I >= sizeof...(Tp), void>::type enforce_disabled_features(std::tuple<Tp...> &) {}

template <std::size_t I = 0, typename... Tp>
    inline typename std::enable_if < I<sizeof...(Tp), void>::type enforce_disabled_features(std::tuple<Tp...> &t) {
    for (auto &disabled_feature : std::get<I>(t)->disabled_features) {
        *disabled_feature.feature = VK_FALSE;
    }
    enforce_disabled_features<I + 1, Tp...>(t);
}

template <std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I >= sizeof...(Tp), void>::type skip_if_mandatory_feature_disabled(const std::tuple<Tp...> &) {}

template <std::size_t I = 0, typename... Tp>
    inline typename std::enable_if < I<sizeof...(Tp), void>::type skip_if_mandatory_feature_disabled(const std::tuple<Tp...> &t) {
    for (auto &mandatory_feature : std::get<I>(t)->mandatory_features) {
        if (*mandatory_feature.feature == VK_FALSE) {
            GTEST_SKIP() << "Requested feature " << std::get<I>(t)->feature_struct_name << "::" << mandatory_feature.feature_name
                         << " is not available on test device, skipping test";
        }
    }
    skip_if_mandatory_feature_disabled<I + 1, Tp...>(t);
}

struct SingleFeature {
    VkBool32 *feature;
    const char *feature_name;
    SingleFeature(VkBool32 *feature, const char *feature_name) : feature(feature), feature_name(feature_name) {}
};

template <typename T>
struct FeatureRequirements {
    T feature_struct{};
    const char *feature_struct_name = nullptr;
    std::vector<SingleFeature> mandatory_features;
    std::vector<SingleFeature> optional_features;
    std::vector<SingleFeature> disabled_features;

    FeatureRequirements(const char *feature_struct_name)
        : feature_struct(vku::InitStructHelper()), feature_struct_name(feature_struct_name) {}
    void AddMandatoryFeature(VkBool32 *p, const char *feature_name) {
        mandatory_features.emplace_back(SingleFeature(p, feature_name));
    }
    void AddDisabledFeature(VkBool32 *p, const char *feature_name) {
        disabled_features.emplace_back(SingleFeature(p, feature_name));
    }
};

template <typename... T>
class FeatureRequirementsCollection {
  public:
    FeatureRequirementsCollection(T &&...args) : feature_structs(std::forward<T>(args)...) {
        if constexpr (sizeof...(T) > 1) {
            pnext_connect_vk_tuple(feature_structs);
        }
    }

    auto &GetFirstFeatureStruct() { return std::get<0>(feature_structs)->feature_struct; }
    void EnforceDisabledFeatures() { enforce_disabled_features(feature_structs); }
    void SkipIfMandatoryFeatureDisabled() const { skip_if_mandatory_feature_disabled(feature_structs); }

  private:
    std::tuple<T...> feature_structs;
};

// Use this to specify that a feature needs to be enabled
// eg: FEATURE_REQUIREMENTS(VkPhysicalDeviceFeatures2, FEATURE_MANDATORY(features.fillModeNonSolid))
#define FEATURE_MANDATORY(field) feature_req->AddMandatoryFeature(&feature_req->feature_struct.field, #field);
// Use this to specify that a feature needs to be disabled
// eg: FEATURE_REQUIREMENTS(VkPhysicalDeviceFeatures2, FEATURE_DISABLED(features.fillModeNonSolid))
#define FEATURE_DISABLED(field) feature_req->AddDisabledFeature(&feature_req->feature_struct.field, #field);

// Cannot have recursive macros, so use this instead. In practice, 10 level deep recursion is enough
#define POOR_MAN_RECURSION_11(...)
#define POOR_MAN_RECURSION_10(x, ...) x POOR_MAN_RECURSION_11(__VA_ARGS__)
#define POOR_MAN_RECURSION_9(x, ...) x POOR_MAN_RECURSION_10(__VA_ARGS__)
#define POOR_MAN_RECURSION_8(x, ...) x POOR_MAN_RECURSION_9(__VA_ARGS__)
#define POOR_MAN_RECURSION_7(x, ...) x POOR_MAN_RECURSION_8(__VA_ARGS__)
#define POOR_MAN_RECURSION_6(x, ...) x POOR_MAN_RECURSION_7(__VA_ARGS__)
#define POOR_MAN_RECURSION_5(x, ...) x POOR_MAN_RECURSION_6(__VA_ARGS__)
#define POOR_MAN_RECURSION_4(x, ...) x POOR_MAN_RECURSION_5(__VA_ARGS__)
#define POOR_MAN_RECURSION_3(x, ...) x POOR_MAN_RECURSION_4(__VA_ARGS__)
#define POOR_MAN_RECURSION_2(x, ...) x POOR_MAN_RECURSION_3(__VA_ARGS__)
#define POOR_MAN_RECURSION_1(x, ...) x POOR_MAN_RECURSION_2(__VA_ARGS__)

#define FEATURE_REQUIREMENTS(VkPhysicalFeature, ...)                                                     \
    []() {                                                                                               \
        auto feature_req = std::make_unique<FeatureRequirements<VkPhysicalFeature>>(#VkPhysicalFeature); \
        POOR_MAN_RECURSION_1(__VA_ARGS__)                                                                \
        return feature_req;                                                                              \
    }()

/// Typical feature requirements flow:
/// ---
//
// AddRequiredExtensions(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
// RETURN_IF_SKIP(InitFramework())
//
// auto feature_requirements = MakeFeatureRequirementsCollection(
//     FEATURE_REQUIREMENTS(VkPhysicalDeviceFeatures2,
//        FEATURE_MANDATORY(features.logicOp), FEATURE_DISABLED(features.fillModeNonSolid)),
//     FEATURE_REQUIREMENTS(VkPhysicalDeviceExtendedDynamicState3FeaturesEXT, FEATURE_MANDATORY(extendedDynamicState3PolygonMode)));
//
// RETURN_IF_SKIP(InitStateWithRequirements(feature_requirements))

template <typename... T>
auto MakeFeatureRequirementsCollection(T &&...args) {
    return FeatureRequirementsCollection<T...>(std::forward<T>(args)...);
}
