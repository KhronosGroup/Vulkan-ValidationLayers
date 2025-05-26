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

#include "../framework/feature_requirements.h"

#include "generated/pnext_chain_extraction.h"

#include <vulkan/utility/vk_struct_helper.hpp>

namespace vkt {

FeatureRequirements::FeatureRequirements() {
    queried_phys_dev_features_ = vku::InitStructHelper();
    enabled_phys_dev_features_ = vku::InitStructHelper();
}

FeatureRequirements::~FeatureRequirements() {
    vvl::PnextChainFree(queried_phys_dev_features_.pNext);
    queried_phys_dev_features_.pNext = nullptr;

    vvl::PnextChainFree(enabled_phys_dev_features_.pNext);
    enabled_phys_dev_features_.pNext = nullptr;
}

void FeatureRequirements::AddRequiredFeature(APIVersion api_version, vkt::Feature feature) {
    required_features_.emplace_back(FeatureInfo{AddFeature(queried_phys_dev_features_, api_version, feature),
                                                AddFeature(enabled_phys_dev_features_, api_version, feature)});
}

void FeatureRequirements::AddOptionalFeature(APIVersion api_version, vkt::Feature feature) {
    optional_features_.emplace_back(FeatureInfo{AddFeature(queried_phys_dev_features_, api_version, feature),
                                                AddFeature(enabled_phys_dev_features_, api_version, feature)});
}

vkt::FeatureAndName FeatureRequirements::AddFeature(VkPhysicalDeviceFeatures2& phys_dev_features, APIVersion api_version,
                                                    vkt::Feature feature) {
    switch (feature) {
        case vkt::Feature::robustBufferAccess: {
            FeatureAndName f{&phys_dev_features.features.robustBufferAccess, "VkPhysicalDeviceFeatures::robustBufferAccess"};
            return f;
        }
        case vkt::Feature::fullDrawIndexUint32: {
            FeatureAndName f{&phys_dev_features.features.fullDrawIndexUint32, "VkPhysicalDeviceFeatures::fullDrawIndexUint32"};
            return f;
        }
        case vkt::Feature::imageCubeArray: {
            FeatureAndName f{&phys_dev_features.features.imageCubeArray, "VkPhysicalDeviceFeatures::imageCubeArray"};
            return f;
        }
        case vkt::Feature::independentBlend: {
            FeatureAndName f{&phys_dev_features.features.independentBlend, "VkPhysicalDeviceFeatures::independentBlend"};
            return f;
        }
        case vkt::Feature::geometryShader: {
            FeatureAndName f{&phys_dev_features.features.geometryShader, "VkPhysicalDeviceFeatures::geometryShader"};
            return f;
        }
        case vkt::Feature::tessellationShader: {
            FeatureAndName f{&phys_dev_features.features.tessellationShader, "VkPhysicalDeviceFeatures::tessellationShader"};
            return f;
        }
        case vkt::Feature::sampleRateShading: {
            FeatureAndName f{&phys_dev_features.features.sampleRateShading, "VkPhysicalDeviceFeatures::sampleRateShading"};
            return f;
        }
        case vkt::Feature::dualSrcBlend: {
            FeatureAndName f{&phys_dev_features.features.dualSrcBlend, "VkPhysicalDeviceFeatures::dualSrcBlend"};
            return f;
        }
        case vkt::Feature::logicOp: {
            FeatureAndName f{&phys_dev_features.features.logicOp, "VkPhysicalDeviceFeatures::logicOp"};
            return f;
        }
        case vkt::Feature::multiDrawIndirect: {
            FeatureAndName f{&phys_dev_features.features.multiDrawIndirect, "VkPhysicalDeviceFeatures::multiDrawIndirect"};
            return f;
        }
        case vkt::Feature::drawIndirectFirstInstance: {
            FeatureAndName f{&phys_dev_features.features.drawIndirectFirstInstance,
                             "VkPhysicalDeviceFeatures::drawIndirectFirstInstance"};
            return f;
        }
        case vkt::Feature::depthClamp: {
            FeatureAndName f{&phys_dev_features.features.depthClamp, "VkPhysicalDeviceFeatures::depthClamp"};
            return f;
        }
        case vkt::Feature::depthBiasClamp: {
            FeatureAndName f{&phys_dev_features.features.depthBiasClamp, "VkPhysicalDeviceFeatures::depthBiasClamp"};
            return f;
        }
        case vkt::Feature::fillModeNonSolid: {
            FeatureAndName f{&phys_dev_features.features.fillModeNonSolid, "VkPhysicalDeviceFeatures::fillModeNonSolid"};
            return f;
        }
        case vkt::Feature::depthBounds: {
            FeatureAndName f{&phys_dev_features.features.depthBounds, "VkPhysicalDeviceFeatures::depthBounds"};
            return f;
        }
        case vkt::Feature::wideLines: {
            FeatureAndName f{&phys_dev_features.features.wideLines, "VkPhysicalDeviceFeatures::wideLines"};
            return f;
        }
        case vkt::Feature::largePoints: {
            FeatureAndName f{&phys_dev_features.features.largePoints, "VkPhysicalDeviceFeatures::largePoints"};
            return f;
        }
        case vkt::Feature::alphaToOne: {
            FeatureAndName f{&phys_dev_features.features.alphaToOne, "VkPhysicalDeviceFeatures::alphaToOne"};
            return f;
        }
        case vkt::Feature::multiViewport: {
            FeatureAndName f{&phys_dev_features.features.multiViewport, "VkPhysicalDeviceFeatures::multiViewport"};
            return f;
        }
        case vkt::Feature::samplerAnisotropy: {
            FeatureAndName f{&phys_dev_features.features.samplerAnisotropy, "VkPhysicalDeviceFeatures::samplerAnisotropy"};
            return f;
        }
        case vkt::Feature::textureCompressionETC2: {
            FeatureAndName f{&phys_dev_features.features.textureCompressionETC2,
                             "VkPhysicalDeviceFeatures::textureCompressionETC2"};
            return f;
        }
        case vkt::Feature::textureCompressionASTC_LDR: {
            FeatureAndName f{&phys_dev_features.features.textureCompressionASTC_LDR,
                             "VkPhysicalDeviceFeatures::textureCompressionASTC_LDR"};
            return f;
        }
        case vkt::Feature::textureCompressionBC: {
            FeatureAndName f{&phys_dev_features.features.textureCompressionBC, "VkPhysicalDeviceFeatures::textureCompressionBC"};
            return f;
        }
        case vkt::Feature::occlusionQueryPrecise: {
            FeatureAndName f{&phys_dev_features.features.occlusionQueryPrecise, "VkPhysicalDeviceFeatures::occlusionQueryPrecise"};
            return f;
        }
        case vkt::Feature::pipelineStatisticsQuery: {
            FeatureAndName f{&phys_dev_features.features.pipelineStatisticsQuery,
                             "VkPhysicalDeviceFeatures::pipelineStatisticsQuery"};
            return f;
        }
        case vkt::Feature::vertexPipelineStoresAndAtomics: {
            FeatureAndName f{&phys_dev_features.features.vertexPipelineStoresAndAtomics,
                             "VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics"};
            return f;
        }
        case vkt::Feature::fragmentStoresAndAtomics: {
            FeatureAndName f{&phys_dev_features.features.fragmentStoresAndAtomics,
                             "VkPhysicalDeviceFeatures::fragmentStoresAndAtomics"};
            return f;
        }
        case vkt::Feature::shaderTessellationAndGeometryPointSize: {
            FeatureAndName f{&phys_dev_features.features.shaderTessellationAndGeometryPointSize,
                             "VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize"};
            return f;
        }
        case vkt::Feature::shaderImageGatherExtended: {
            FeatureAndName f{&phys_dev_features.features.shaderImageGatherExtended,
                             "VkPhysicalDeviceFeatures::shaderImageGatherExtended"};
            return f;
        }
        case vkt::Feature::shaderStorageImageExtendedFormats: {
            FeatureAndName f{&phys_dev_features.features.shaderStorageImageExtendedFormats,
                             "VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats"};
            return f;
        }
        case vkt::Feature::shaderStorageImageMultisample: {
            FeatureAndName f{&phys_dev_features.features.shaderStorageImageMultisample,
                             "VkPhysicalDeviceFeatures::shaderStorageImageMultisample"};
            return f;
        }
        case vkt::Feature::shaderStorageImageReadWithoutFormat: {
            FeatureAndName f{&phys_dev_features.features.shaderStorageImageReadWithoutFormat,
                             "VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat"};
            return f;
        }
        case vkt::Feature::shaderStorageImageWriteWithoutFormat: {
            FeatureAndName f{&phys_dev_features.features.shaderStorageImageWriteWithoutFormat,
                             "VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat"};
            return f;
        }
        case vkt::Feature::shaderUniformBufferArrayDynamicIndexing: {
            FeatureAndName f{&phys_dev_features.features.shaderUniformBufferArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderSampledImageArrayDynamicIndexing: {
            FeatureAndName f{&phys_dev_features.features.shaderSampledImageArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderStorageBufferArrayDynamicIndexing: {
            FeatureAndName f{&phys_dev_features.features.shaderStorageBufferArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderStorageImageArrayDynamicIndexing: {
            FeatureAndName f{&phys_dev_features.features.shaderStorageImageArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderClipDistance: {
            FeatureAndName f{&phys_dev_features.features.shaderClipDistance, "VkPhysicalDeviceFeatures::shaderClipDistance"};
            return f;
        }
        case vkt::Feature::shaderCullDistance: {
            FeatureAndName f{&phys_dev_features.features.shaderCullDistance, "VkPhysicalDeviceFeatures::shaderCullDistance"};
            return f;
        }
        case vkt::Feature::shaderFloat64: {
            FeatureAndName f{&phys_dev_features.features.shaderFloat64, "VkPhysicalDeviceFeatures::shaderFloat64"};
            return f;
        }
        case vkt::Feature::shaderInt64: {
            FeatureAndName f{&phys_dev_features.features.shaderInt64, "VkPhysicalDeviceFeatures::shaderInt64"};
            return f;
        }
        case vkt::Feature::shaderInt16: {
            FeatureAndName f{&phys_dev_features.features.shaderInt16, "VkPhysicalDeviceFeatures::shaderInt16"};
            return f;
        }
        case vkt::Feature::shaderResourceResidency: {
            FeatureAndName f{&phys_dev_features.features.shaderResourceResidency,
                             "VkPhysicalDeviceFeatures::shaderResourceResidency"};
            return f;
        }
        case vkt::Feature::shaderResourceMinLod: {
            FeatureAndName f{&phys_dev_features.features.shaderResourceMinLod, "VkPhysicalDeviceFeatures::shaderResourceMinLod"};
            return f;
        }
        case vkt::Feature::sparseBinding: {
            FeatureAndName f{&phys_dev_features.features.sparseBinding, "VkPhysicalDeviceFeatures::sparseBinding"};
            return f;
        }
        case vkt::Feature::sparseResidencyBuffer: {
            FeatureAndName f{&phys_dev_features.features.sparseResidencyBuffer, "VkPhysicalDeviceFeatures::sparseResidencyBuffer"};
            return f;
        }
        case vkt::Feature::sparseResidencyImage2D: {
            FeatureAndName f{&phys_dev_features.features.sparseResidencyImage2D,
                             "VkPhysicalDeviceFeatures::sparseResidencyImage2D"};
            return f;
        }
        case vkt::Feature::sparseResidencyImage3D: {
            FeatureAndName f{&phys_dev_features.features.sparseResidencyImage3D,
                             "VkPhysicalDeviceFeatures::sparseResidencyImage3D"};
            return f;
        }
        case vkt::Feature::sparseResidency2Samples: {
            FeatureAndName f{&phys_dev_features.features.sparseResidency2Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency2Samples"};
            return f;
        }
        case vkt::Feature::sparseResidency4Samples: {
            FeatureAndName f{&phys_dev_features.features.sparseResidency4Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency4Samples"};
            return f;
        }
        case vkt::Feature::sparseResidency8Samples: {
            FeatureAndName f{&phys_dev_features.features.sparseResidency8Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency8Samples"};
            return f;
        }
        case vkt::Feature::sparseResidency16Samples: {
            FeatureAndName f{&phys_dev_features.features.sparseResidency16Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency16Samples"};
            return f;
        }
        case vkt::Feature::sparseResidencyAliased: {
            FeatureAndName f{&phys_dev_features.features.sparseResidencyAliased,
                             "VkPhysicalDeviceFeatures::sparseResidencyAliased"};
            return f;
        }
        case vkt::Feature::variableMultisampleRate: {
            FeatureAndName f{&phys_dev_features.features.variableMultisampleRate,
                             "VkPhysicalDeviceFeatures::variableMultisampleRate"};
            return f;
        }
        case vkt::Feature::inheritedQueries: {
            FeatureAndName f{&phys_dev_features.features.inheritedQueries, "VkPhysicalDeviceFeatures::inheritedQueries"};
            return f;
        }
        default:
            FeatureAndName f = vkt::AddFeature(api_version, feature, &phys_dev_features.pNext);
            return f;
    }
}

const char* FeatureRequirements::AnyRequiredFeatureDisabled() const {
    for (const auto& info : required_features_) {
        if (*info.queried_feature.feature == VK_FALSE) {
            return info.queried_feature.name;
        }
    }
    return nullptr;
}

void FeatureRequirements::UpdateEnabledFeatures() {
    for (const auto& info : required_features_) {
        *info.enabled_feature.feature = VK_TRUE;
    }

    for (const auto& info : optional_features_) {
        *info.enabled_feature.feature = *info.queried_feature.feature;
    }
}

}  // namespace vkt